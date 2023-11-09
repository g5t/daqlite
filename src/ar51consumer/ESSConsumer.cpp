// Copyright (C) 2020 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.cpp
///
//===----------------------------------------------------------------------===//

#include <ESSConsumer.h>
#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <unistd.h>
#include <vector>

ESSConsumer::ESSConsumer(std::string Broker, std::string Topic) :
  Broker(Broker), Topic(Topic) {

  mConsumer = subscribeTopic();
  assert(mConsumer != nullptr);
}

RdKafka::KafkaConsumer *ESSConsumer::subscribeTopic() const {
  auto mConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

  if (!mConf) {
    fmt::print("Unable to create global Conf object\n");
    return nullptr;
  }

  std::string ErrStr;
  /// \todo figure out good values for these
  /// \todo some may be obsolete
  mConf->set("metadata.broker.list", Broker, ErrStr);
  mConf->set("message.max.bytes", MessageMaxBytes, ErrStr);
  mConf->set("fetch.message.max.bytes", FetchMessagMaxBytes, ErrStr);
  mConf->set("replica.fetch.max.bytes", ReplicaFetchMaxBytes, ErrStr);
  std::string GroupId = "Anyrandomstring0";
  mConf->set("group.id", GroupId, ErrStr);
  mConf->set("enable.auto.commit", EnableAutoCommit, ErrStr);
  mConf->set("enable.auto.offset.store", EnableAutoOffsetStore, ErrStr);

  auto ret = RdKafka::KafkaConsumer::create(mConf, ErrStr);
  if (!ret) {
    fmt::print("Failed to create consumer: {}\n", ErrStr);
    return nullptr;
  }
  //
  // // Start consumer for topic+partition at start offset
  RdKafka::ErrorCode resp = ret->subscribe({Topic});
  if (resp != RdKafka::ERR_NO_ERROR) {
    fmt::print("Failed to subscribe consumer to '{}': {}\n", Topic, err2str(resp));
  }

  return ret;
}



/// proof of concept only, poor error checking
uint32_t ESSConsumer::processAR51Data(RdKafka::Message *Msg) {

  const auto & RawReadoutMsg = GetRawReadoutMessage(Msg->payload());
  int MsgSize = RawReadoutMsg->raw_data()->size();

  struct PacketHeaderV0 * Header = (struct PacketHeaderV0 *)RawReadoutMsg->raw_data()->Data();

  if ((Header->CookieAndType & 0xffffff) != 0x535345) {
    printf("Non-ESS readout (cookie 0x%08x)\n", Header->CookieAndType);
    return 0;
  }

  if (Header->TotalLength !=  MsgSize) {
    printf("Readout size mismatch\n");
    return 0;
  }

  printf("OQ %u, SEQ %u, length %u (%u)\n", Header->OutputQueue, Header->SeqNum,
           Header->TotalLength, RawReadoutMsg->raw_data()->size());

  if (MsgSize == 30) {
    printf("Heartbeat\n");
    return 0;
  }

  int Type = Header->CookieAndType >> 28;

  if (Type == 4) {
    printf("VMM3 based readout\n");
  } else if (Type == 3) {
    printf("CAEN based readout\n");
  } else if (Type == 6) {
    printf("CDT based readout\n");
  } else {
    printf("Unregistered readout\n");
    return 0;
  }



  return 0;
}

bool ESSConsumer::handleMessage(RdKafka::Message *Message) {
  switch (Message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    return false;
    break;

  case RdKafka::ERR_NO_ERROR:
    if (RawReadoutMessageBufferHasIdentifier(Message->payload())) {
      processAR51Data(Message);
    } else {
      printf("Not a ar51 Kafka message!\n");
    }
    return true;
    break;

  default:
    fmt::print("Consume failed: {}", Message->errstr());
    return false;
    break;
  }
}

/// \todo is timeout reasonable?
RdKafka::Message *ESSConsumer::consume() { return mConsumer->consume(1000); }

// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
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
uint32_t ESSConsumer::processAR52Data(RdKafka::Message *Msg) {

  const auto & RawReadoutMsg = GetRawReadoutMessage(Msg->payload());

  struct PacketHeaderV0 * Header = (struct PacketHeaderV0 *)RawReadoutMsg->raw_data()->Data();
  printf("OQ %u, SEQ %u, length %u\n", Header->OutputQueue, Header->SeqNum,
           Header->TotalLength);

  return 0;
}

bool ESSConsumer::handleMessage(RdKafka::Message *Message) {
  switch (Message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    return false;
    break;

  case RdKafka::ERR_NO_ERROR:
    if (RawReadoutMessageBufferHasIdentifier(Message->payload())) {
      processAR52Data(Message);
    } else {
      printf("Not a ar52 Kafka message!\n");
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

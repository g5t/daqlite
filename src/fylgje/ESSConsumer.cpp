// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.cpp
///
//===----------------------------------------------------------------------===//

#include "ESSConsumer.h"
#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <unistd.h>
#include <utility>
#include <vector>

static double frame_time(uint32_t pulse_hi, uint32_t pulse_lo, uint32_t prev_hi, uint32_t prev_lo, uint32_t high, uint32_t low){
    if (high > pulse_hi || (high == pulse_hi && low > pulse_lo)){
        return static_cast<double>(high - pulse_hi) + (static_cast<int>(low) - static_cast<int>(pulse_lo)) / 1e9;
    }
    if (high > prev_hi || (high == prev_hi && low > prev_lo)){
        return static_cast<double>(high - prev_hi) + (static_cast<int>(low) - static_cast<int>(prev_lo)) / 1e9;
    }
    return 0.;
}

ESSConsumer::ESSConsumer(data_t * data, std::string Broker, std::string Topic) :
  Broker(std::move(Broker)), Topic(std::move(Topic)), histograms(data) {

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
  mConf->set("fetch.message.max.bytes", FetchMessageMaxBytes, ErrStr);
  mConf->set("replica.fetch.max.bytes", ReplicaFetchMaxBytes, ErrStr);
  std::string GroupId = fmt::format("Groupid (pid) {}", getpid());
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

/// \brief Example parser for CAEN Data
uint32_t ESSConsumer::parseCAENData(uint8_t * Readout, int Size, uint32_t hi, uint32_t lo, uint32_t p_hi, uint32_t p_lo) {
  uint32_t processed{0};
  int BytesLeft = Size;
  while (BytesLeft >= static_cast<int>(sizeof(caen_readout))) {
    auto * crd = (caen_readout *)Readout;
    if (crd->FEN != 0){
      printf("FEN %u, Length %u, HighTime %u, LowTime %u, Flags %u, Group %u\n",
             crd->FEN, crd->Length, crd->HighTime, crd->LowTime, crd->Flags_OM, crd->Group);
    } else {
      auto time = frame_time(hi, lo, p_hi, p_lo, crd->HighTime, crd->LowTime);
      histograms->add(crd->Fiber, crd->Group, crd->A, crd->B, time);
    }
    BytesLeft -= sizeof(caen_readout);
    Readout += sizeof(caen_readout);
    ++processed;
  }
  return processed;
}

/// Main processing function for AR51 data
uint32_t ESSConsumer::processAR51Data(RdKafka::Message *Msg) {

  // First check header
  const auto & RawReadoutMsg = GetRawReadoutMessage(Msg->payload());
  auto MsgSize = static_cast<int>(RawReadoutMsg->raw_data()->size());

  auto * Header = (struct PacketHeaderV0 *)RawReadoutMsg->raw_data()->Data();

  if ((Header->CookieAndType & 0xffffff) != 0x535345) {
    printf("Non-ESS readout (cookie 0x%08x)\n", Header->CookieAndType);
    return 0;
  }

  if (Header->TotalLength !=  MsgSize) {
    printf("Readout size mismatch\n");
    return 0;
  }


  // Then print details of the data
  // printf("OQ %u, SEQ %u, length %u ", Header->OutputQueue, Header->SeqNum,
  //          Header->TotalLength);

  if (MsgSize == sizeof(struct PacketHeaderV0)) {
    //printf("Heartbeat\n");
    return 0;
  }

  auto Type = Header->CookieAndType >> 28;
  auto pulse_hi = Header->PulseHigh;
  auto pulse_lo = Header->PulseLow;
  auto prev_hi = Header->PrevPulseHigh;
  auto prev_lo = Header->PrevPulseLow;

  uint8_t * DataPtr = (uint8_t * )Header + 30;
  if (Header->Version == 1) {
    DataPtr += 2;
  }
  auto DataLength = Header->TotalLength - sizeof(struct PacketHeaderV0);

  // Dispatch technology specific
  if (3 == Type){
      printf("CAEN based readout\n");
      return parseCAENData(DataPtr, static_cast<int>(DataLength), pulse_hi, pulse_lo, prev_hi, prev_lo);
  } else {
      fmt::print("Unregistered readout Type {}\n", Type);
  }
  return 0;
}


///\brief Main entry for kafka message processing
ESSConsumer::Status ESSConsumer::handleMessage(RdKafka::Message *Message) {
  switch (Message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    return Continue;

  case RdKafka::ERR_NO_ERROR: {
      uint32_t count{0};
      if (RawReadoutMessageBufferHasIdentifier(Message->payload())) {
          count = processAR51Data(Message);
      } else {
          printf("Not a ar51 Kafka message!\n");
      }
      return count ? Update : Continue;
  }
  default:
    fmt::print("Consume failed: {}", Message->errstr());
    return Halt;
  }
}

/// \todo is timeout reasonable?
RdKafka::Message *ESSConsumer::consume() { return mConsumer->consume(1000); }

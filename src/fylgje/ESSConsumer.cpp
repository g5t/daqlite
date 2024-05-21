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

static int min_ticks{100'000'000};
static int max_ticks{-100'000'000};
/**
 * @brief Convert packet header and data times to seconds since reference time
 * @param pulse_hi Latest header reference time integer seconds since epoch
 * @param pulse_lo Latest header reference time 88.053 MHz ticks since pulse_hi
 * @param prev_hi Previous header reference time integer seconds since epoch
 * @param prev_lo Previous header reference time 88.053 MHz ticks since prev_hi
 * @param high Event reference time integer seconds since epoch
 * @param low Event reference time 88.053 MHz ticks since high
 * @return A positive double representing the time in seconds since _a_ reference time
 */
static double frame_time(uint32_t pulse_hi, uint32_t pulse_lo, uint32_t prev_hi, uint32_t prev_lo, uint32_t high, uint32_t low){
  auto converter = [high,low](uint32_t h, uint32_t l) {
    // low is allowed to be less than l, in which case direct subtraction would yield a large positive integer
    // if the cast to int is not done before subtraction.
    const int ticks = 88'052'500;
    auto diff = static_cast<int>(low)-static_cast<int>(l);
//    bool changed{false};
//    if (diff < min_ticks) { min_ticks = diff; changed=true; }
//    if (diff > max_ticks) { max_ticks = diff; changed=true; }
//    if (changed) std::cout << "\rticks (" << min_ticks << ", " << max_ticks << ")" << std::flush;
    return static_cast<double>(high-h) + static_cast<double>(diff) / ticks;
  };
  double time{0.};
  if (high > pulse_hi || (high == pulse_hi && low > pulse_lo)){
    time =  converter(pulse_hi, pulse_lo);
  } else if (high > prev_hi || (high == prev_hi && low > prev_lo)){
    time = converter(prev_hi, prev_lo);
  }
  return time;
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

static double min_time{1};
static double max_time{0};

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
//      bool changed{false};
//      if (time < min_time) { min_time = time; changed=true; }
//      if (time > max_time) { max_time = time; changed=true; }
//      if (changed) std::cout << "\rtime (" << min_time << ", " << max_time << ")" << std::flush;
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

  if (MsgSize == sizeof(struct PacketHeaderV0)) {
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
  //TODO Is this correct for Version 1 headers too?
  auto DataLength = Header->TotalLength - sizeof(struct PacketHeaderV0);

  // Dispatch technology specific
  if (3 == Type){
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

// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.h
///
/// \brief Wrapper class for librdkafka
///
/// Sets up the kafka consumer and handles binning of event pixel ids
//===----------------------------------------------------------------------===//

#pragma once

#include "ar51_readout_data_generated.h"
#include <librdkafka/rdkafkacpp.h>
#include "bifrost.h"

class ESSConsumer {
public:

  // Data format for the common ESS readout header
  struct PacketHeaderV0 {
    uint8_t Padding0;
    uint8_t Version;
    uint32_t CookieAndType;
    uint16_t TotalLength;
    uint8_t OutputQueue;
    uint8_t TimeSource;
    uint32_t PulseHigh;
    uint32_t PulseLow;
    uint32_t PrevPulseHigh;
    uint32_t PrevPulseLow;
    uint32_t SeqNum;
  } __attribute__((packed));

  struct caen_readout {
    uint8_t Fiber;
    uint8_t FEN;
    uint16_t Length;
    uint32_t HighTime;
    uint32_t LowTime;
    uint8_t Flags_OM;
    uint8_t Group;
    uint16_t Unused;
    int16_t A;
    int16_t B;
    int16_t C;
    int16_t D;
  } __attribute__((packed));


  /// \brief Constructor needs the configured Broker and Topic
  ESSConsumer(std::string Broker, std::string Topic);

  /// \brief wrapper function for librdkafka consumer
  RdKafka::Message *consume();

  /// \brief setup librdkafka parameters for Broker and Topic
  RdKafka::KafkaConsumer *subscribeTopic() const;

  /// \brief initial checks for kafka error messages
  /// \return true if message contains data, false otherwise
  bool handleMessage(RdKafka::Message *Msg);

  /// \brief print out some information
  uint32_t processAR51Data(RdKafka::Message *Msg);

  ///
  void parseCAENData(uint8_t * Readout, int Size);

  bifrostHistograms histograms{};

private:
  std::string Broker{""};
  std::string Topic{""};
  std::string MessageMaxBytes{"10000000"};
  std::string FetchMessagMaxBytes{"10000000"};
  std::string ReplicaFetchMaxBytes{"10000000"};
  std::string EnableAutoCommit{"false"};
  std::string EnableAutoOffsetStore{"false"};

  RdKafka::Conf *mConf;
  RdKafka::Conf *mTConf;
  RdKafka::KafkaConsumer *mConsumer;
  RdKafka::Topic *mTopic;
};

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

class ESSConsumer {
public:

  // Data fromat for VMM3a based readout
  struct vmm3a_readout {
    uint8_t Ring;
    uint8_t FEN;
    uint16_t Length;
    uint32_t TimeHi;
    uint32_t TimeLo;
    uint16_t BC;
    uint16_t OTADC;
    uint8_t GEO;
    uint8_t TDC;
    uint8_t VMM;
    uint8_t Channel;
  } __attribute__((packed));

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
  void parseVMM3aData(uint8_t * Readout, int Size);

  ///
  void parseCAENData(uint8_t * Readout, int Size);

  ///
  void parseCDTData(uint8_t * Readout, int Size);

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

  /// \brief Some stat counters
  /// \todo use or delete?
  struct Stat {
    uint64_t MessagesRx{0};
    uint64_t MessagesTMO{0};
    uint64_t MessagesData{0};
    uint64_t MessagesEOF{0};
    uint64_t MessagesUnknown{0};
    uint64_t MessagesOther{0};
  } mKafkaStats;

  // Rings, Hybrids, Asics, Channels
  int Histogram[12][5][2][64];
};

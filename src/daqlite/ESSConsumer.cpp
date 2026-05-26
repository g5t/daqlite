// Copyright (C) 2020 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.cpp
///
//===----------------------------------------------------------------------===//

#include <ESSConsumer.h>

#include <Configuration.h>
#include <ThreadSafeVector.h>
#include <types/PlotType.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wnull-dereference"
#include <da00_dataarray_generated.h>
#include <ev42_events_generated.h>
#include <ev44_events_generated.h>
#include <flatbuffers/flatbuffers.h>
#pragma GCC diagnostic pop

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <random>
#include <string_view>
#include <vector>

using std::optional;
using std::string;
using std::vector;

// Internal key used when no source filtering is specified in JSON config file
static constexpr std::string_view UnfilteredKey{""};

// clang-format off
ESSConsumer::ESSConsumer(Configuration &Config,
                         const vector<std::pair<string, string>> &KafkaConfig)
    : mConfig(Config) {
  auto &geom = mConfig.mGeometry;
  mNumPixels = geom.XDim * geom.YDim * geom.ZDim;
  mMinPixel = geom.Offset + 1;
  mMaxPixel = geom.Offset + mNumPixels;
  assert(mMaxPixel != 0);
  assert(mMinPixel < mMaxPixel);

  mConsumer = subscribeTopic(KafkaConfig);
  assert(mConsumer != nullptr);

  const auto types = {
    DataType::NONE,
    DataType::ANY,
    DataType::TOF,
    DataType::HISTOGRAM,
    DataType::HISTOGRAM_TOF,
    DataType::PIXEL_ID
  };
  for (DataType t : types) {
    mSubscriptionCount[t] = 0;
    mDeliveryCount[t] = 0;
  }
}
// clang-format on

RdKafka::KafkaConsumer *ESSConsumer::subscribeTopic(
    const vector<std::pair<string, string>> &KafkaConfig) const {
  auto Conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

  if (!Conf) {
    fmt::print("Unable to create global Conf object\n");
    return nullptr;
  }

  string ErrStr;
  Conf->set("metadata.broker.list", mConfig.mKafka.Broker, ErrStr);
  Conf->set("message.max.bytes", mConfig.mKafka.MessageMaxBytes, ErrStr);
  Conf->set("fetch.message.max.bytes", mConfig.mKafka.FetchMessageMaxBytes,
             ErrStr);
  Conf->set("replica.fetch.max.bytes", mConfig.mKafka.ReplicaFetchMaxBytes,
             ErrStr);
  string GroupId = randomGroupString(16);
  Conf->set("group.id", GroupId, ErrStr);
  Conf->set("enable.auto.commit", mConfig.mKafka.EnableAutoCommit, ErrStr);
  Conf->set("enable.auto.offset.store", mConfig.mKafka.EnableAutoOffsetStore,
             ErrStr);

  for (const auto &Config : KafkaConfig) {
    Conf->set(Config.first, Config.second, ErrStr);
  }

  auto ret = RdKafka::KafkaConsumer::create(Conf, ErrStr);
  if (!ret) {
    fmt::print("Failed to create consumer: {}\n", ErrStr);
    return nullptr;
  }
  RdKafka::ErrorCode resp = ret->subscribe({mConfig.mKafka.Topic});
  if (resp != RdKafka::ERR_NO_ERROR) {
    fmt::print("Failed to subscribe consumer to '{}': {}\n",
               mConfig.mKafka.Topic, err2str(resp));
  }

  return ret;
}

// Suppress false-positive null-dereference warnings from flatbuffers
// inlined code at higher optimization levels (Release builds)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"

uint32_t ESSConsumer::processEV44Data(RdKafka::Message *Msg) {
  auto EvMsg = GetEvent44Message(Msg->payload());
  auto PixelIds = EvMsg->pixel_id();
  auto TOFs = EvMsg->time_of_flight();

  if (PixelIds->size() != TOFs->size()) {
    mEventDiscard++;
    return 0;
  }

  auto source = resolveSource(EvMsg->source_name()->str());
  if (!source) {
    mEventDiscard++;
    return 0;
  }

  // Local temporary histograms to avoid locking during processing
  vector<uint32_t> PixelVector(mNumPixels, 0);
  vector<uint32_t> TofBinVector(mConfig.mTOF.BinSize, 0);

  if (mHasPixelIds) {
    mPixelIDs[*source].reserve(mPixelIDs[*source].size() + PixelIds->size());
  }
  if (mHasTOFs) {
    mTOFs[*source].reserve(mTOFs[*source].size() + PixelIds->size());
  }
  for (size_t i = 0; i < PixelIds->size(); i++) {
    auto Pixel = static_cast<uint32_t>((*PixelIds)[i]);
    auto Tof   = static_cast<uint32_t>((*TOFs)[i]) / mConfig.mTOF.Scale; // ns to us

    // Accumulate events for 2D TOF
    uint32_t TofBin = std::min(Tof, mConfig.mTOF.MaxValue) *
                      (mConfig.mTOF.BinSize - 1) / mConfig.mTOF.MaxValue;
    if (mHasPixelIds) mPixelIDs[*source].push_back(Pixel);
    if (mHasTOFs)     mTOFs[*source].push_back(TofBin);

    if ((Pixel > mMaxPixel) || (Pixel < mMinPixel)) {
      mEventDiscard++;
    } else {
      mEventAccept++;

      Pixel = Pixel - mConfig.mGeometry.Offset;
      PixelVector[Pixel]++;

      Tof = std::min(Tof, mConfig.mTOF.MaxValue);
      TofBinVector[Tof * (mConfig.mTOF.BinSize - 1) / mConfig.mTOF.MaxValue]++;
    }
  }

  // Update thread safe histograms storage with new data
  mHistograms[*source].add_values(PixelVector);
  mHistogramTOFs[*source].add_values(TofBinVector);

  mEventCount += PixelIds->size();

  return PixelIds->size();
}

uint32_t ESSConsumer::processDA00Data(RdKafka::Message *Msg) {
  auto EvMsg = Getda00_DataArray(Msg->payload());
  if (EvMsg->data()->size() == 0) {
    mEventDiscard++;
    return 0;
  }

  auto source = resolveSource(EvMsg->source_name()->str());
  if (!source) {
    mEventDiscard++;
    return 0;
  }

  const auto TimeBinsVariable = EvMsg->data()->Get(0);
  const auto DataBinsVariable = EvMsg->data()->Get(1);

  auto BinEdges = getDataVector(*TimeBinsVariable);
  auto DataBins = getDataVector(*DataBinsVariable);

  // Bin edges has one plus element to describe last edge compared to the data
  // which has as many elements as bins
  if (BinEdges.size() != DataBins.size() + 1) {
    mEventDiscard++;
    return 0;
  }

  int64_t MaxTime = *std::max_element(BinEdges.begin(), BinEdges.end());

  if (MaxTime / mConfig.mTOF.Scale > mConfig.mTOF.MaxValue) {
    return 0;
  }

  mHistograms[*source].add_values(DataBins);
  if (mHasTOFs) {
    mTOFs[*source] = std::move(BinEdges);
  }

  mEventCount++;
  mEventAccept++;

  return mHistograms[*source].size();
}

uint32_t ESSConsumer::processEV42Data(RdKafka::Message *Msg) {
  auto EvMsg = GetEventMessage(Msg->payload());
  auto PixelIds = EvMsg->detector_id();
  auto TOFs = EvMsg->time_of_flight();

  if (PixelIds->size() != TOFs->size()) {
    mEventDiscard++;
    return 0;
  }

  auto source = resolveSource(EvMsg->source_name()->str());
  if (!source) {
    mEventDiscard++;
    return 0;
  }

  vector<uint32_t> PixelVector(mNumPixels, 0);
  vector<uint32_t> TofBinVector(mConfig.mTOF.BinSize, 0);

  if (mHasPixelIds) {
    mPixelIDs[*source].reserve(mPixelIDs[*source].size() + PixelIds->size());
  }
  if (mHasTOFs) {
    mTOFs[*source].reserve(mTOFs[*source].size() + PixelIds->size());
  }
  for (size_t i = 0; i < PixelIds->size(); i++) {
    uint32_t Pixel = static_cast<uint32_t>((*PixelIds)[i]);
    uint32_t Tof   = static_cast<uint32_t>((*TOFs)[i]) / mConfig.mTOF.Scale; // ns to us

    // Accumulate events for 2D TOF
    uint32_t TofBin = std::min(Tof, mConfig.mTOF.MaxValue) *
                      (mConfig.mTOF.BinSize - 1) / mConfig.mTOF.MaxValue;
    if (mHasPixelIds) mPixelIDs[*source].push_back(Pixel);
    if (mHasTOFs)     mTOFs[*source].push_back(TofBin);

    if ((Pixel > mMaxPixel) || (Pixel < mMinPixel)) {
      mEventDiscard++;
    } else {
      mEventAccept++;
      Pixel = Pixel - mConfig.mGeometry.Offset;
      PixelVector[Pixel]++;
      Tof = std::min(Tof, mConfig.mTOF.MaxValue);
      TofBinVector[Tof * (mConfig.mTOF.BinSize - 1) / mConfig.mTOF.MaxValue]++;
    }
  }

  mHistograms[*source].add_values(PixelVector);
  mHistogramTOFs[*source].add_values(TofBinVector);

  mEventCount += PixelIds->size();
  return PixelIds->size();
}

bool ESSConsumer::handleMessage(RdKafka::Message *Message) {
  const uint8_t *FlatBuffer = static_cast<const uint8_t *>(Message->payload());

  flatbuffers::Verifier Verifier(FlatBuffer, Message->len());

  switch (Message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    return false;

  case RdKafka::ERR_NO_ERROR:
    if (VerifyEvent44MessageBuffer(Verifier)) {
      processEV44Data(Message);
    } else if (VerifyEventMessageBuffer(Verifier)) {
      processEV42Data(Message);
    } else if (Verifyda00_DataArrayBuffer(Verifier)) {
      processDA00Data(Message);
    } else {
      fmt::print("Unknown message type\n");
      return false;
    }

    return true;

  case RdKafka::ERR__PARTITION_EOF:
    return false;

  case RdKafka::ERR__UNKNOWN_TOPIC:
  case RdKafka::ERR__UNKNOWN_PARTITION:
    fmt::print("Consume failed: {}\n", Message->errstr());
    return false;

  default: // Other errors
    fmt::print("Consume failed: {}\n", Message->errstr());
    return false;
  }
}

string ESSConsumer::randomGroupString(size_t length) {
  static constexpr std::string_view charset = "0123456789"
                                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                              "abcdefghijklmnopqrstuvwxyz";
  std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<size_t> dist(0, charset.size() - 1);

  string str(length, 0);
  std::generate_n(str.begin(), length, [&]() -> char {
    return charset[dist(gen)];
  });

  return str;
}

vector<int64_t>
ESSConsumer::getDataVector(const da00_Variable &Variable) const {
  vector<int64_t> Data;

  auto shape = Variable.shape()->Get(0);

  switch (Variable.data_type()) {
  case da00_dtype::int32: {
    auto dataPtr = reinterpret_cast<const int32_t *>(Variable.data());

    // Skip the first element which is the length of the data
    dataPtr++;
    Data.assign(dataPtr, dataPtr + shape);
    break;
  }
  case da00_dtype::int64: {
    auto dataPtr = reinterpret_cast<const int64_t *>(Variable.data());

    // Skip the first element which is the length of the data
    dataPtr++;
    Data.assign(dataPtr, dataPtr + shape);
    break;
  }
  case da00_dtype::uint32: {
    auto dataPtr = reinterpret_cast<const uint32_t *>(Variable.data());

    // Skip the first element which is the length of the data
    dataPtr++;
    Data.assign(dataPtr, dataPtr + shape);
    break;
  }
  case da00_dtype::uint64: {
    auto dataPtr = reinterpret_cast<const uint64_t *>(Variable.data());

    // Skip the first element which is the length of the data
    dataPtr++;
    Data.assign(dataPtr, dataPtr + shape);
    break;
  }
  default:
    fmt::print("getDataVector(): unsupported data type\n");
    break;
  }
  return Data;
}

#pragma GCC diagnostic pop

std::unique_ptr<RdKafka::Message> ESSConsumer::consume() {
  std::unique_ptr<RdKafka::Message> msg(mConsumer->consume(1000));
  return msg;
}

ESSConsumer::TSVectorMap *ESSConsumer::getData(DataType dataType) {
  return const_cast<TSVectorMap *>(std::as_const(*this).getData(dataType));
}

const ESSConsumer::TSVectorMap *ESSConsumer::getData(DataType dataType) const {
  switch (dataType) {
  case DataType::HISTOGRAM:
    return &mHistograms;

  case DataType::HISTOGRAM_TOF:
    return &mHistogramTOFs;

  case DataType::PIXEL_ID:
    return &mPixelIDs;

  case DataType::TOF:
    return &mTOFs;

  default:
    fmt::print("getData(): invalid DataType\n");
    return nullptr;
  }
}

vector<uint32_t> ESSConsumer::readData(DataType dataType,
                                       optional<string> source,
                                       bool reset) {
  // Get non-const pointer to data container for the specified data type
  TSVectorMap *dataMap = getData(dataType);

  // Check that data exists
  if (dataMap == nullptr) {
    return {};
  }

  vector<uint32_t> result;

  // If a source is specified, get data for that source only
  if (source.has_value()) {
    // Check that data exists for the requested source
    auto iter = dataMap->find(*source);
    if (iter == dataMap->cend()) {
      return {};
    }

    // Swap (O(1) under lock) when resetting; copy otherwise
    const bool doReset = reset && checkDelivery(dataType);
    result = iter->second.get(doReset);
  } else {
    // If no source is specified, combine data from all sources element-wise
    const bool doReset = reset && checkDelivery(dataType);
    for (auto &[key, data] : *dataMap) {
      auto chunk = data.get(doReset);
      if (result.empty()) {
        result = std::move(chunk);
      } else {
        // Resize result if necessary to accommodate larger data
        if (chunk.size() > result.size()) {
          result.resize(chunk.size(), 0);
        }
        // Add values element-wise
        for (size_t i = 0; i < chunk.size(); i++) {
          result[i] += chunk[i];
        }
      }
    }
  }

  return result;
}

size_t ESSConsumer::getDataSize(DataType dataType,
                                optional<string> source) const {
  const TSVectorMap *dataMap = getData(dataType);
  if (dataMap == nullptr) {
    return 0;
  }

  if (source.has_value()) {
    const auto iter = dataMap->find(*source);
    return (iter != dataMap->cend()) ? iter->second.size() : 0;
  }

  // No source specified — return total size across all sources
  size_t total = 0;
  for (const auto &[key, data] : *dataMap) {
    total += data.size();
  }
  return total;
}

size_t ESSConsumer::getBinSize(optional<string> source) const {
  const size_t size = getDataSize(DataType::TOF, source);

  return size > 0 ? size - 1 : size;
};

void ESSConsumer::addSource(const optional<string> &source) {
  // std::nullopt means "no filtering" - ignore
  if (!source.has_value() || source->empty()) {
    return;
  }

  mSources.insert(*source);
}

optional<string>
ESSConsumer::resolveSource(const string &msgSourceName) const {
  if (mSources.empty()) {
    return string{UnfilteredKey};
  }
  if (!hasSource(msgSourceName)) {
    return std::nullopt;
  }
  return msgSourceName;
}

bool ESSConsumer::hasSource(const string &source) const {
  const auto it = mSources.find(source);

  return it != mSources.cend();
}

bool ESSConsumer::checkDelivery(DataType Type) {
  mDeliveryCount[Type] += 1;
  if (mDeliveryCount[Type] == mSubscriptionCount[Type]) {
    mDeliveryCount[Type] = 0;

    return true;
  }

  return false;
}

void ESSConsumer::addSubscriber(PlotType Type, bool add) {
  // Increment the total number of plots
  if (add) mSubscribers++; else mSubscribers--;
  if (add) mSubscriptionCount[DataType::ANY]++; else mSubscriptionCount[DataType::ANY]--;

  // Register or de-register data types for the plot type
  switch (Type) {
  case PlotType::TOF:
    if (add) mSubscriptionCount[DataType::HISTOGRAM_TOF]++;
    else     mSubscriptionCount[DataType::HISTOGRAM_TOF]--;
    break;

  case PlotType::TOF2D:
    if (add) mSubscriptionCount[DataType::PIXEL_ID]++;
    else     mSubscriptionCount[DataType::PIXEL_ID]--;
    if (add) mSubscriptionCount[DataType::TOF]++;
    else     mSubscriptionCount[DataType::TOF]--;
    break;

  case PlotType::PIXELS:
    if (add) mSubscriptionCount[DataType::HISTOGRAM]++;
    else     mSubscriptionCount[DataType::HISTOGRAM]--;
    break;

  case PlotType::HISTOGRAM:
    if (add) mSubscriptionCount[DataType::HISTOGRAM]++;
    else     mSubscriptionCount[DataType::HISTOGRAM]--;
    if (add) mSubscriptionCount[DataType::TOF]++;
    else     mSubscriptionCount[DataType::TOF]--;
    break;

  default:
    break;
  }

  // Containers for unsubscribed data types are never cleared, so we refresh
  // subscription flags to skip appends to mPixelIDs / mTOFs when no plot
  // subscribes to those data types.
  mHasPixelIds = mSubscriptionCount[DataType::PIXEL_ID] > 0;
  mHasTOFs     = mSubscriptionCount[DataType::TOF] > 0;

  // Uncomment to print the subscription state
  // for (const auto& dt: DataType::types()) {
  //   fmt::print("ESSConsumer::addSubscriber {} {}\n", Type,
  //   mSubscriptionCount[dt]);
  // }
}

size_t ESSConsumer::subscriptionCount() const {
  size_t count = 0;
  for (const auto &[key, c] : mSubscriptionCount) {
    count += c;
  }

  return count;
}

void ESSConsumer::gotEventRequest() {
  mEventRequests += 1;

  // Reset if all event requests have been delivered
  if (mEventRequests == mSubscriptionCount[DataType::ANY]) {
    mEventCount = 0;
    mEventAccept = 0;
    mEventDiscard = 0;

    mEventRequests = 0;
  }
}

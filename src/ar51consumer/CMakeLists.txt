# Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)

#

set(ar51consumer_src
  ar51consumer.cpp
  ESSConsumer.cpp
  WorkerThread.cpp
  )

set(ar51consumer_inc
  ESSConsumer.h
  WorkerThread.h
  )


add_executable(
  ar51consumer
  ${ar51consumer_src}
  ${ar51consumer_inc}
)

target_link_libraries(
  ar51consumer
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE Qt5::Widgets
)

target_link_libraries(ar51consumer
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(ar51consumer
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)

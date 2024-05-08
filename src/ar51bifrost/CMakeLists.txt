# Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)

#

set(ar51bifrost_src
  ar51bifrost.cpp
  ESSConsumer.cpp
  GraphBase.cpp
  CAENGraph.cpp
  WorkerThread.cpp
  MainWindow.cpp
  )

set(ar51bifrost_inc
        bifrost.h
  ESSConsumer.h
  GraphBase.h
  CAENGraph.h
  WorkerThread.h
  MainWindow.h
  )


add_executable(
  ar51bifrost
  ${ar51bifrost_src}
  ${ar51bifrost_inc}
)

target_link_libraries(
  ar51bifrost
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE QPlot
  PRIVATE QtColorWidgets
  PRIVATE Qt5::Widgets
)

target_link_libraries(ar51bifrost
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(ar51bifrost
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)

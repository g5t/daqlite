find_package(RdKafka REQUIRED)
find_package(h5cpp REQUIRED)

set(target fylgje-saver)

set(${target}_src
  Worker.cpp
  KafkaConfig.cpp
  ESSConsumer.cpp
  DataManager.cpp
  Configuration.cpp
  Calibration.cpp
  Time.cpp
  Main.cpp
)

set(${target}_inc
  Calibration.h
  Configuration.h
  ESSConsumer.h
  KafkaConfig.h
  DataManager.h
  Worker.h
  JsonFile.h
  Time.h
)

add_executable(
  ${target}
  ${${target}_src}
  ${${target}_inc}
)

target_link_libraries(
  ${target}
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE h5cpp::h5cpp
)

target_link_libraries(${target}
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(${target}
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)

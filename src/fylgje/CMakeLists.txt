include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)
find_package(h5cpp REQUIRED)

set(fylgje_src
  fylgje_main.cpp
  fylgje_window.cpp
  fylgje_overview.cpp
  fylgje_calibration.cpp
  fylgje_data.cpp
  WorkerThread.cpp
  KafkaConfig.cpp
  ESSConsumer.cpp
  data_manager.cpp
  Configuration.cpp
  Calibration.cpp
  Interval.cpp
  )

set(fylgje_inc
  Calibration.h
  Configuration.h
  ESSConsumer.h
  KafkaConfig.h
  fylgje_window.h
  plot_manager.h
  two_spin_box.h
  data_manager.h
  WorkerThread.h
  JsonFile.h
  cycles.h
  Interval.h
  )

set(fylgje_ui
  fylgje_window.ui
  )

set(CMAKE_AUTOUIC ON)

add_executable(
  fylgje
  ${fylgje_src}
  ${fylgje_inc}
  ${fylgje_ui}
)

target_link_libraries(
  fylgje
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE QPlot
  PRIVATE Qt6::Widgets
  PRIVATE h5cpp::h5cpp
)

target_link_libraries(fylgje
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(fylgje
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)

#=============================================================================
# Dependencies
#=============================================================================

include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)

#=============================================================================
# Sources
#=============================================================================

set(daqlite_src
  AbstractPlot.cpp
  AMOR2DTofPlot.cpp
  Configuration.cpp
  daqlite.cpp
  ESSConsumer.cpp
  HelpWindow.cpp
  HistogramPlot.cpp
  KafkaConfig.cpp
  MainWindow.cpp
  PixelsPlot.cpp
  TofPlot.cpp
  VNCMessageFilter.cpp
  WorkerThread.cpp
  )

set(daqlite_inc
  AbstractPlot.h
  AMOR2DTofPlot.h
  Configuration.h
  ESSConsumer.h
  HelpWindow.h
  HistogramPlot.h
  KafkaConfig.h
  MainWindow.h
  PixelsPlot.h
  ThreadSafeVector.h
  TofPlot.h
  VNCMessageFilter.h
  WorkerThread.h

  # Types
  types/PlotType.h
  )

set(daqlite_ui
  MainWindow.ui
  )

#=============================================================================
# Target
#=============================================================================

add_executable(
  daqlite
  ${daqlite_src}
  ${daqlite_inc}
  ${daqlite_ui}
)

set_target_properties(daqlite PROPERTIES AUTOUIC ON)

#=============================================================================
# Compiler options
#=============================================================================

target_compile_options(daqlite PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror=shadow>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror=float-conversion>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror=sign-conversion>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror=old-style-cast>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror=null-dereference>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror=unused-variable>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Werror=reorder>)

#=============================================================================
# Link libraries
#=============================================================================

target_link_libraries(
  daqlite
  PRIVATE fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE QPlot
  PRIVATE Qt6::Widgets
  PRIVATE Qt6::Core5Compat
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>
)

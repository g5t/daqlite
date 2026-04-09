# daqlite
This is Daquiri light - a much simplified version of Daquiri.

daqlite subscribes to Kafka topics and provides simple visualizations, such as

* Single 2D detector image
* Three 2D projections of a 3D detector image (xy, xz, yz)
* Line/bar plots of Time Of Flight (TOF) or similar

## Build

### Prerequisites
* Qt6 library (ensure `qmake` is in your path)
* Conan package manager version 1.x (https://conan.io) - **Note:** We use Conan before Conan 2
* CMake 3.0.0 or higher
* C++17 compatible compiler

### Conan Initialization
Before building for the first time, initialize Conan with the ESS configuration:

    conan config install https://github.com/ess-dmsc/conan-configuration.git

Then install the dependencies (from the build directory):

    mkdir build
    cd build
    conan install .. --build=missing --profile=linux_x86_64_gcc11

This only needs to be done once per Conan installation.

**Note:** To switch between Release and Debug builds, you need to rerun `conan install` with the same command but adding `-s build_type=Debug` or `-s build_type=Release`.

### Build Steps

    cd build
    cmake ..
    make -j$(nproc)

## Run

Daqlite needs a configuration file specified by the -f option

    daqlite -f myconfig.json

See examples of how to run multiple daqlite instances in the scripts/ folder and
examples of config files for different instruments in configs/

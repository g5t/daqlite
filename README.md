# daqlite
This is Daquiri light - a much simplified version of Daquiri.

Daqlite subscribes to Kafka topics and provides simple visualisations such as

* Single 2D detector image
* Three 2D projections of a 3D detector image (xy, xz, yz)
* Line/bar plots of Time Of Flight (TOF) or similar

## Build

    mkdir build
    cd build
    cmake ..
    make

### Qt library
Note that a Qt library needs to be installed. Depending on its installation it
might be necessry to specify the location for the initial cmake command

   CMAKE_PREFIX_PATH=/home/janedoe/QtTest/5.14.2/clang_64/lib/cmake/ cmake ..

## Run

Daqlite needs a configuration file specified by tge -f option

    daqlite -f myconfig.json

See examples of how to run multiple daqlite instances in the scripts/ folder and
examples of config files for different instruments in configs/

# ipu7-camera-hal

This repository supports MIPI cameras through the IPU7 on Intel Lunar Lake platform.
There are 4 repositories that provide the complete setup:

- https://github.com/intel/ipu7-drivers - kernel drivers for the IPU and sensors
- https://github.com/intel/ipu7-camera-bins - IPU firmware and proprietary image processing libraries
- https://github.com/intel/ipu7-camera-hal - HAL for processing of images in userspace
- https://github.com/intel/icamerasrc/tree/icamerasrc_slim_api (branch:icamerasrc_slim_api) - Gstreamer src plugin

## Content of this repository:
- IPU7 HAL

## Build instructions:
- Dependencies: ipu7-camera-bins
    Please follow https://github.com/intel/ipu7-camera-bins README to install.

- Dependencies: libexpat-dev libjsoncpp-dev automake libtool libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

- Build and install:
```sh
cd ipu7-camera-hal && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX=/usr \
-DCMAKE_INSTALL_LIBDIR=lib \
-DBUILD_CAMHAL_ADAPTOR=ON \
-DBUILD_CAMHAL_PLUGIN=ON \
-DIPU_VERSIONS="ipu7x;ipu75xa;ipu8" \
-DUSE_STATIC_GRAPH=ON \
-DUSE_STATIC_GRAPH_AUTOGEN=ON \
..
make && sudo make install
```

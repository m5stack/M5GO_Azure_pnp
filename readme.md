# Azure IoT Plug and Play sample

This directory contains a sample that implements the model [dtmi:M5Stack:m5go;1](https://github.com/Azure/iot-plugandplay-models/blob/main/dtmi/m5stack/m5go-1.json).



Implement network configuration information.Modify the RGB lamp by properties to send telemetry information to the cloud


# Development Host Setup

This project is to be used with Espressif's IoT Development Framework, [ESP-IDF](https://github.com/espressif/esp-idf). Follow these steps to get started:

- Setup ESP IDF development environment by following the steps here.
- In a separate folder, clone the esp-azure project as follows (please note the --recursive option, which is required to clone the various git submodules required by esp-azure)

``` bash
git clone --recursive 
```

> Note that if you ever change the branch or the git head of either esp-idf or esp-azure, ensure that all the submodules of the git repo are in sync by executing 
``` bash
git submodule update --init --recursive
```

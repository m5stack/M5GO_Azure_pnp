# Azure IoT Plug and Play sample

This directory contains a sample that implements the model [dtmi:M5Stack:m5go;1](https://github.com/Azure/iot-plugandplay-models/blob/main/dtmi/m5stack/m5go-1.json).

Implement network configuration information.Modify the RGB lamp by properties to send telemetry information to the cloud.

# Prepare the Device

PortA connects to ENV Unit, PortB is connects to ANGLE Unit, PortC connects to PIR Unit.
![hardwareConnection](./docs/m5go.jpg)

# Development Host Setup

This project is to be used with Espressif's IoT Development Framework, [ESP-IDF](https://github.com/espressif/esp-idf). Follow these steps to get started:

- Setup ESP IDF development environment by following the steps [here](https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/index.html).(Please install version 4.2)
- In a separate folder, clone the M5GO_Azure_pnp project as follows (please note the --recursive option, which is required to clone the various git submodules required)

``` bash
git clone --recursive https://github.com/m5stack/M5GO_Azure_pnp.git
```

> Note that if you ever change the branch or the git head of either esp-idf or esp-azure, ensure that all the submodules of the git repo are in sync by executing 
``` bash
git submodule update --init --recursive
```

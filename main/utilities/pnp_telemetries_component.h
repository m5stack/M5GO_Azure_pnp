// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This header implements a simulated thermostat as defined by dtmi:com:example:Telemetries;1.  In particular,
// this Telemetries component is defined to be run as a subcomponent by the temperature controller interface
// defined by https://github.com/Azure/opendigitaltwins-dtdl/blob/master/DTDL/v2/samples/TemperatureController.json.  The 
// temperature controller defines two components that implement dtmi:com:example:Telemetries;1, named thermostat1 and
// thermostat2.  
//
// The code in this header/.c file is designed to be generic so that the calling application can call PnP_TelemetriesComponent_CreateHandle
// multiple times and then pass processing of a given component (thermostat1 or thermostat2) to the appropriate function.
//

#ifndef PNP_TELEMETRIES_CONTROLLER_H
#define PNP_TELEMETRIES_CONTROLLER_H

#include "parson.h"
#include "iothub_device_client_ll.h"


uint8_t PnP_SendTelemetry(IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL);
#endif /* PNP_TELEMETRIES_CONTROLLER_H */

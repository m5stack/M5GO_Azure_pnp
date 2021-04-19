// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// IoTHub Device Client and IoT core utility related header files
#include "iothub.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"
#include "m5go.h"
// PnP utilities.
#include "pnp_device_client_ll.h"
#include "pnp_protocol.h"
#include "pnp_protocol.h"
#include "pnp_deviceinfo_component.h"
#include "pnp_telemetries_component.h"

#include "sdkconfig.h"

#define LGFX_M5STACK
#include <LovyanGFX.hpp>

extern LGFX lcd;

// Environment variable
#define getenvironment(c) (char *)(c)

// Environment variable used to specify how app connects to hub and the two possible values
static const char g_securityTypeEnvironmentVariable[] = "DPS";
static const char g_securityTypeConnectionStringValue[] = "connectionString";
static const char g_securityTypeDpsValue[] = "DPS";

// Environment variable used to specify this application's connection string
static const char g_connectionStringEnvironmentVariable[] = "IOTHUB_DEVICE_CONNECTION_STRING";

// Values of connection / security settings read from environment variables and/or DPS runtime
PNP_DEVICE_CONFIGURATION g_pnpDeviceConfiguration;

#ifdef USE_PROV_MODULE_FULL

extern char *id_scope;
extern char *device_id;
extern char *symmetric_key;

// Environment variable used to specify this application's DPS id scope
static const char *g_dpsIdScopeEnvironmentVariable;

// Environment variable used to specify this application's DPS device id
static const char *g_dpsDeviceIdEnvironmentVariable;

// Environment variable used to specify this application's DPS device key
static const char *g_dpsDeviceKeyEnvironmentVariable;

// Environment variable used to optionally specify this application's DPS id scope
static const char g_dpsEndpointEnvironmentVariable[] = "global.azure-devices-provisioning.net";

// Global provisioning endpoint for DPS if one is not specified via the environment
static const char g_dps_DefaultGlobalProvUri[] = "global.azure-devices-provisioning.net";
#endif

// Amount of time to sleep between polling hub, in milliseconds.  Set to wake up every 100 milliseconds.
static unsigned int g_sleepBetweenPollsMs = 100;

// Every time the main loop wakes up, on the g_sendTelemetryPollInterval(th) pass will send a telemetry message.
// So we will send telemetry every (g_sendTelemetryPollInterval * g_sleepBetweenPollsMs) milliseconds; 60 seconds as currently configured.
static const int g_sendTelemetryPollInterval = 300;

// Whether tracing at the IoTHub client is enabled or not.
static bool g_hubClientTraceEnabled = true;

// DTMI indicating this device's ModelId.
static const char g_temperatureControllerModelId[] = "dtmi:M5Stack:m5go;1";



static const char g_deviceInfoComponentName[] = "deviceInformation";


void sendResponse(IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient, const char *propertyName, int val, int version)
{
    char valString[32];
    IOTHUB_CLIENT_RESULT iothubClientResult;
    STRING_HANDLE jsonToSend = NULL;

    if (snprintf(valString, sizeof(valString), "%d", val) < 0)
    {
        LogError("Unable to create target temperature string for reporting result");
    }
    else if ((jsonToSend = PnP_CreateReportedPropertyWithStatus(NULL, propertyName, valString,
                                                                PNP_STATUS_SUCCESS, "success", version)) == NULL)
    {
        LogError("Unable to build reported property response");
    }
    else
    {
        const char *jsonToSendStr = STRING_c_str(jsonToSend);
        size_t jsonToSendStrLen = strlen(jsonToSendStr);

        if ((iothubClientResult = IoTHubDeviceClient_LL_SendReportedState(deviceClient, (const unsigned char *)jsonToSendStr, jsonToSendStrLen, NULL, NULL)) != IOTHUB_CLIENT_OK)
        {
            LogError("Unable to send reported state, error=%d", iothubClientResult);
        }
        else
        {
            LogInfo("Sending acknowledgement of property to IoTHub");
        }
    }

    STRING_delete(jsonToSend);
}
//
// PnP_TempControlComponent_ApplicationPropertyCallback is the callback function is invoked when PnP_ProcessTwinData() visits each property.
//
static void PnP_TempControlComponent_ApplicationPropertyCallback(const char *componentName, const char *propertyName, JSON_Value *propertyValue, int version, void *userContextCallback)
{
    // This sample uses the pnp_device_client.h/.c to create the IOTHUB_DEVICE_CLIENT_LL_HANDLE as well as initialize callbacks.
    // The convention used is that IOTHUB_DEVICE_CLIENT_LL_HANDLE is passed as the userContextCallback on the initial twin callback.
    // The pnp_protocol.h/.c pass this userContextCallback down to this visitor function.
    IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient = (IOTHUB_DEVICE_CLIENT_LL_HANDLE)userContextCallback;

    if (strcmp(propertyName, "LightLeft") == 0)
    {
        m5go_Sk6812_SetSideColor(0, (int)json_number(propertyValue));
        m5go_Sk6812_Show();
        sendResponse(deviceClient, propertyName, (int)json_number(propertyValue), version);
    }
    if (strcmp(propertyName, "LightRight") == 0)
    {
        m5go_Sk6812_SetSideColor(1, (int)json_number(propertyValue));
        m5go_Sk6812_Show();
        sendResponse(deviceClient, propertyName, (int)json_number(propertyValue), version);
    }
}

//
// PnP_TempControlComponent_DeviceTwinCallback is invoked by IoT SDK when a twin - either full twin or a PATCH update - arrives.
//
static void PnP_TempControlComponent_DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payload, size_t size, void *userContextCallback)
{
    // Invoke PnP_ProcessTwinData to actualy process the data.  PnP_ProcessTwinData uses a visitor pattern to parse
    // the JSON and then visit each property, invoking PnP_TempControlComponent_ApplicationPropertyCallback on each element.
    if (PnP_ProcessTwinData(updateState, payload, size, NULL, 0, PnP_TempControlComponent_ApplicationPropertyCallback, userContextCallback) == false)
    {
        // If we're unable to parse the JSON for any reason (typically because the JSON is malformed or we ran out of memory)
        // there is no action we can take beyond logging.
        LogError("Unable to process twin json.  Ignoring any desired property update requests");
    }
}

//
// GetConnectionStringFromEnvironment retrieves the connection string based on environment variable
//
static bool GetConnectionStringFromEnvironment()
{
    bool result;
    if ((g_pnpDeviceConfiguration.u.connectionString = getenvironment(g_connectionStringEnvironmentVariable)) == NULL)
    {
        LogError("Cannot read environment variable=%s", g_connectionStringEnvironmentVariable);
        result = false;
    }
    else
    {
        g_pnpDeviceConfiguration.securityType = PNP_CONNECTION_SECURITY_TYPE_CONNECTION_STRING;
        result = true;
    }

    return result;
}

//
// GetDpsFromEnvironment retrieves DPS configuration for a symmetric key based connection
// from environment variables
//
static bool GetDpsFromEnvironment()
{
#ifndef USE_PROV_MODULE_FULL
    // Explain to user misconfiguration.  The "run_e2e_tests" must be set to OFF because otherwise
    // the e2e's test HSM layer and symmetric key logic will conflict.
    LogError("DPS based authentication was requested via environment variables, but DPS is not enabled.");
    LogError("DPS is an optional component of the Azure IoT C SDK.  It is enabled with symmetric keys at cmake time by");
    LogError("passing <-Duse_prov_client=ON -Dhsm_type_symm_key=ON -Drun_e2e_tests=OFF> to cmake's command line");
    return false;
#else
    bool result;

    if ((g_pnpDeviceConfiguration.u.dpsConnectionAuth.endpoint = getenvironment(g_dpsEndpointEnvironmentVariable)) == NULL)
    {
        // We will fall back to standard endpoint if one is not specified
        g_pnpDeviceConfiguration.u.dpsConnectionAuth.endpoint = g_dps_DefaultGlobalProvUri;
    }

    if ((g_pnpDeviceConfiguration.u.dpsConnectionAuth.idScope = getenvironment(g_dpsIdScopeEnvironmentVariable)) == NULL)
    {
        LogError("Cannot read environment variable=%s", g_dpsIdScopeEnvironmentVariable);
        result = false;
    }
    else if ((g_pnpDeviceConfiguration.u.dpsConnectionAuth.deviceId = getenvironment(g_dpsDeviceIdEnvironmentVariable)) == NULL)
    {
        LogError("Cannot read environment variable=%s", g_dpsDeviceIdEnvironmentVariable);
        result = false;
    }
    else if ((g_pnpDeviceConfiguration.u.dpsConnectionAuth.deviceKey = getenvironment(g_dpsDeviceKeyEnvironmentVariable)) == NULL)
    {
        LogError("Cannot read environment variable=%s", g_dpsDeviceKeyEnvironmentVariable);
        result = false;
    }
    else
    {
        g_pnpDeviceConfiguration.securityType = PNP_CONNECTION_SECURITY_TYPE_DPS;
        result = true;
    }

    return result;
#endif // USE_PROV_MODULE_FULL
}

//
// GetConfigurationFromEnvironment reads how to connect to the IoT Hub (using
// either a connection string or a DPS symmetric key) from the environment.
//
static bool GetConnectionSettingsFromEnvironment()
{
    const char *securityTypeString;
    bool result;

    if ((securityTypeString = getenvironment(g_securityTypeEnvironmentVariable)) == NULL)
    {
        LogError("Cannot read environment variable=%s", g_securityTypeEnvironmentVariable);
        result = false;
    }
    else
    {
        if (strcmp(securityTypeString, g_securityTypeConnectionStringValue) == 0)
        {
            result = GetConnectionStringFromEnvironment();
        }
        else if (strcmp(securityTypeString, g_securityTypeDpsValue) == 0)
        {
            result = GetDpsFromEnvironment();
        }
        else
        {
            LogError("Environment variable %s must be either %s or %s", g_securityTypeEnvironmentVariable, g_securityTypeConnectionStringValue, g_securityTypeDpsValue);
            result = false;
        }
    }

    return result;
}

static IOTHUB_DEVICE_CLIENT_LL_HANDLE CreateDeviceClientAndAllocateComponents(void)
{
    IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient = NULL;
    bool result;

    g_pnpDeviceConfiguration.deviceMethodCallback = NULL;
    g_pnpDeviceConfiguration.deviceTwinCallback = PnP_TempControlComponent_DeviceTwinCallback;
    g_pnpDeviceConfiguration.enableTracing = g_hubClientTraceEnabled;
    g_pnpDeviceConfiguration.modelId = g_temperatureControllerModelId;

    if (GetConnectionSettingsFromEnvironment() == false)
    {
        LogError("Cannot read required environment variable(s)");
        result = false;
    }
    else if ((deviceClient = PnP_CreateDeviceClientLLHandle(&g_pnpDeviceConfiguration)) == NULL)
    {
        LogError("Failure creating IotHub device client");
        result = false;
    }
    else
    {
        lcd.printf("init succeed!\r\n");
        result = true;
    }

    if (result == false)
    {

        if (deviceClient != NULL)
        {
            IoTHubDeviceClient_LL_Destroy(deviceClient);
            IoTHub_Deinit();
            deviceClient = NULL;
        }
    }

    return deviceClient;
}

int pnp_temperature_controller(void)
{
    g_dpsIdScopeEnvironmentVariable = id_scope;
    g_dpsDeviceIdEnvironmentVariable = device_id;
    g_dpsDeviceKeyEnvironmentVariable = symmetric_key;

    IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClient = NULL;

    if ((deviceClient = CreateDeviceClientAndAllocateComponents()) == NULL)
    {
        LogError("Failure creating IotHub device client");
        lcd.printf("Failure creating IotHub device client\r\n");
    }
    else
    {
        LogInfo("Successfully created device client.  Hit Control-C to exit program\n");

        int numberOfIterations = 0;

        // During startup, send the non-"writeable" properties.
        PnP_DeviceInfoComponent_Report_All_Properties(g_deviceInfoComponentName, deviceClient);
        lcd.printf("Device message sent successfully!\r\n");
        lcd.printf("running!\r\n");
        while (true)
        {
            // Wake up periodically to poll.  Even if we do not plan on sending telemetry, we still need to poll periodically in order to process
            // incoming requests from the server and to do connection keep alives.
            if ((numberOfIterations % g_sendTelemetryPollInterval) == 0)
            {
                if (PnP_SendTelemetry(deviceClient))
                {
                    LogError("Failure send telemetry");
                    lcd.printf("Failure send telemetry\r\n");
                }
            }

            IoTHubDeviceClient_LL_DoWork(deviceClient);
            ThreadAPI_Sleep(g_sleepBetweenPollsMs);
            numberOfIterations++;
        }

        // Clean up the iothub sdk handle
        IoTHubDeviceClient_LL_Destroy(deviceClient);
        // Free all the sdk subsystem
        IoTHub_Deinit();
    }

    return 0;
}

// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// The DTDL for this interface is defined at https://repo.azureiotrepository.com/Models/dtmi:azure:DeviceManagement:DeviceInformation;1?api-version=2020-05-01-preview

// PnP routines
#include "pnp_deviceinfo_component.h"
#include "pnp_protocol.h"

// Core IoT SDK utilities
#include "azure_c_shared_utility/xlogging.h"

// Property names along with their simulated values.
// NOTE: the property values must be legal JSON values.  Strings specifically must be enclosed with an extra set of quotes to be legal json string values.
// The property names in this sample do not hard-code the extra quotes because the underlying PnP sample adds this to names automatically.
#define PNP_ENCODE_STRING_FOR_JSON(str) #str

static const char PnPDeviceInfo_ManufacturerPropertyName[] = "manufacturer";
static const char PnPDeviceInfo_ManufacturerPropertyValue[] = PNP_ENCODE_STRING_FOR_JSON("M5Stack");

static const char PnPDeviceInfo_ProcessorArchitecturePropertyName[] = "processorArchitecture";
static const char PnPDeviceInfo_ProcessorArchitecturePropertyValue[] = PNP_ENCODE_STRING_FOR_JSON("32-bit");

static const char PnPDeviceInfo_TotalStoragePropertyName[] = "totalStorage";
static const char PnPDeviceInfo_TotalStoragePropertyValue[] = "16384";

static const char PnPDeviceInfo_TotalMemoryPropertyName[] = "totalMemory";
static const char PnPDeviceInfo_TotalMemoryPropertyValue[] = "520";

static const char PnPDeviceInfo_LightLeftPropertyName[] = "LightLeft";
static const char PnPDeviceInfo_LightLeftPropertyValue[] = "0";

static const char PnPDeviceInfo_LightRightPropertyName[] = "LightRight";
static const char PnPDeviceInfo_LightRightPropertyValue[] = "0";


//
// SendReportedPropertyForDeviceInformation sends a property as part of DeviceInfo component.
//
static void SendReportedPropertyForDeviceInformation(IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL, const char *componentName, const char *propertyName, const char *propertyValue)
{
    IOTHUB_CLIENT_RESULT iothubClientResult;
    STRING_HANDLE jsonToSend = NULL;

    if ((jsonToSend = PnP_CreateReportedProperty(componentName, propertyName, propertyValue)) == NULL)
    {
        LogError("Unable to build reported property response for propertyName=%s, propertyValue=%s", propertyName, propertyValue);
    }
    else
    {
        const char *jsonToSendStr = STRING_c_str(jsonToSend);
        size_t jsonToSendStrLen = strlen(jsonToSendStr);

        if ((iothubClientResult = IoTHubDeviceClient_LL_SendReportedState(deviceClientLL, (const unsigned char *)jsonToSendStr, jsonToSendStrLen, NULL, NULL)) != IOTHUB_CLIENT_OK)
        {
            LogError("Unable to send reported state for property=%s, error=%d", propertyName, iothubClientResult);
        }
        else
        {
            LogInfo("Sending device information property to IoTHub.  propertyName=%s, propertyValue=%s", propertyName, propertyValue);
        }
    }

    STRING_delete(jsonToSend);
}

void PnP_DeviceInfoComponent_Report_All_Properties(const char *componentName, IOTHUB_DEVICE_CLIENT_LL_HANDLE deviceClientLL)
{
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_ManufacturerPropertyName, PnPDeviceInfo_ManufacturerPropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_ProcessorArchitecturePropertyName, PnPDeviceInfo_ProcessorArchitecturePropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_TotalStoragePropertyName, PnPDeviceInfo_TotalStoragePropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, componentName, PnPDeviceInfo_TotalMemoryPropertyName, PnPDeviceInfo_TotalMemoryPropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, NULL, PnPDeviceInfo_LightLeftPropertyName, PnPDeviceInfo_LightLeftPropertyValue);
    SendReportedPropertyForDeviceInformation(deviceClientLL, NULL, PnPDeviceInfo_LightRightPropertyName, PnPDeviceInfo_LightRightPropertyValue);

}

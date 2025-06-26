#include <stdint.h>
#include <string.h>

#define MAX_CONFIG_PARAMETERS 5
#define MAX_PARAM_VALUE_SIZE 64
#define DIAG_RESPONSE_BUFFER_SIZE 32

typedef struct {
    uint16_t paramId;
    uint16_t actualLength;
    uint8_t data[MAX_PARAM_VALUE_SIZE];
} VehicleConfigParam_t;

static VehicleConfigParam_t g_vehicleConfigs[MAX_CONFIG_PARAMETERS] = {
    {0x1001, 10, {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A}},
    {0x1002, 25, {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29}},
    {0x1003, 5, {0x31, 0x32, 0x33, 0x34, 0x35}},
    {0x1004, 60, {0}},
    {0x1005, 1, {0xAA}}
};

void LogEcuError(const char* msg) {
    (void)msg;
}

int Diag_ReadConfigParameter(uint16_t requestedParamId, uint8_t* responseBuffer, uint16_t bufferCapacity) {
    int bytesCopied = -1;
    VehicleConfigParam_t* foundParam = NULL;

    if (responseBuffer == NULL || bufferCapacity == 0) {
        LogEcuError("Diag: Invalid response buffer for read config.");
        return -1;
    }

    for (int i = 0; i < MAX_CONFIG_PARAMETERS; ++i) {
        if (g_vehicleConfigs[i].paramId == requestedParamId) {
            foundParam = &g_vehicleConfigs[i];
            break;
        }
    }

    if (foundParam == NULL) {
        LogEcuError("Diag: Config parameter not found.");
        return -1;
    }

    uint16_t lenToCopy = foundParam->actualLength;

    if (lenToCopy > MAX_PARAM_VALUE_SIZE) {
        LogEcuError("Diag: Stored parameter actualLength exceeds internal buffer size. Truncating.");
        lenToCopy = MAX_PARAM_VALUE_SIZE;
    }

    if (lenToCopy > bufferCapacity) {
        LogEcuError("Diag: Parameter value too large for response buffer. Truncating.");
        lenToCopy = bufferCapacity;
    }

    if (lenToCopy == 0) {
        return 0;
    }

    memcpy(responseBuffer, foundParam->data, lenToCopy);
    bytesCopied = lenToCopy;

    return bytesCopied;
}

int main() {
    uint8_t rxBuffer[64];
    uint8_t txBuffer[DIAG_RESPONSE_BUFFER_SIZE];

    Diag_ReadConfigParameter(0x1001, txBuffer, sizeof(txBuffer));
    Diag_ReadConfigParameter(0x1004, txBuffer, sizeof(txBuffer));

    return 0;
}
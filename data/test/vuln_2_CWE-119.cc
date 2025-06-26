#include <stdint.h>
#include <string.h>

#define MAX_BMS_PARAMETERS 10
#define MAX_PARAM_DATA_SIZE 32

typedef enum {
    BMS_PARAM_TYPE_VOLTAGE_CAL = 0x01,
    BMS_PARAM_TYPE_CURRENT_GAIN = 0x02,
    BMS_PARAM_TYPE_TEMP_OFFSET = 0x03,
    BMS_PARAM_TYPE_CELL_INFO = 0x04,
    BMS_PARAM_TYPE_UNKNOWN = 0xFF
} BmsParameterType_t;

typedef struct {
    uint16_t parameterId;
    BmsParameterType_t type;
    uint8_t dataLength;
    uint8_t data[MAX_PARAM_DATA_SIZE];
} BmsParameter_t;

static BmsParameter_t g_decodedParameters[MAX_BMS_PARAMETERS];
static uint8_t g_numDecodedParameters = 0;

void BmsLogError(const char* msg) {
    (void)msg;
}

void BmsResetParameters() {
    g_numDecodedParameters = 0;
}

void BmsConfig_DecodeParameters(const uint8_t* payload, uint16_t payloadLength) {
    uint16_t currentOffset = 0;
    g_numDecodedParameters = 0;

    while (currentOffset < payloadLength && g_numDecodedParameters < MAX_BMS_PARAMETERS) {
        if (payloadLength - currentOffset < 4) {
            BmsLogError("BMS Config: Malformed parameter header or insufficient payload.");
            break;
        }

        uint16_t paramId = (payload[currentOffset] << 8) | payload[currentOffset + 1];
        BmsParameterType_t paramType = (BmsParameterType_t)payload[currentOffset + 2];
        uint8_t paramDataLength = payload[currentOffset + 3];

        if ((payloadLength - currentOffset - 4) < paramDataLength) {
            BmsLogError("BMS Config: Reported parameter data length exceeds remaining payload.");
            break;
        }

        g_decodedParameters[g_numDecodedParameters].parameterId = paramId;
        g_decodedParameters[g_numDecodedParameters].type = paramType;
        g_decodedParameters[g_numDecodedParameters].dataLength = paramDataLength;

        memcpy(g_decodedParameters[g_numDecodedParameters].data, &payload[currentOffset + 4], paramDataLength);

        currentOffset += 4 + paramDataLength;
        g_numDecodedParameters++;
    }

    if (g_numDecodedParameters >= MAX_BMS_PARAMETERS) {
        BmsLogError("BMS Config: Reached maximum number of decoded parameters. Some parameters might be truncated.");
    }
}
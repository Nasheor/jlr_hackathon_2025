typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef enum {
    CONFIG_TYPE_ENGINE_MAP = 0x01,
    CONFIG_TYPE_GEARBOX_SETTINGS = 0x02,
    CONFIG_TYPE_ADAS_PARAMS = 0x03,
    CONFIG_TYPE_DIAG_LOG_LEVEL = 0x04,
    CONFIG_TYPE_INVALID = 0xFF
} ConfigDataType_t;

typedef struct {
    ConfigDataType_t type;
    uint16_t maxDataSize;
} ConfigTypeProperties_t;

static const ConfigTypeProperties_t configPropertiesTable[] = {
    {CONFIG_TYPE_ENGINE_MAP, 128},
    {CONFIG_TYPE_GEARBOX_SETTINGS, 64},
    {CONFIG_TYPE_ADAS_PARAMS, 256},
    {CONFIG_TYPE_DIAG_LOG_LEVEL, 4}
};
static const uint16_t CONFIG_PROPERTIES_TABLE_SIZE = sizeof(configPropertiesTable) / sizeof(configPropertiesTable[0]);

uint8_t applyConfig(ConfigDataType_t type, const uint8_t* data, uint16_t length) {
    if (data == NULL || length == 0) {
        return 0;
    }
    return 1;
}

static uint16_t getConfigTypeMaxDataSize(ConfigDataType_t type) {
    for (uint16_t i = 0; i < CONFIG_PROPERTIES_TABLE_SIZE; ++i) {
        if (configPropertiesTable[i].type == type) {
            return configPropertiesTable[i].maxDataSize;
        }
    }
    return 0;
}

void processConfigUpdate_Vulnerable(const uint8_t* msg_data, uint16_t msg_len) {
    if (msg_data == NULL || msg_len < 3) {
        return;
    }

    ConfigDataType_t configType = (ConfigDataType_t)msg_data[0];
    uint16_t receivedDataLength = (msg_data[1] << 8) | msg_data[2];
    const uint8_t* configData = &msg_data[3];

    if (msg_len < (3 + receivedDataLength)) {
        return;
    }

    uint16_t maxAllowedDataSize = getConfigTypeMaxDataSize(configType);
    if (maxAllowedDataSize == 0) {
        return;
    }

    uint8_t* tempBuffer = (uint8_t*)malloc(maxAllowedDataSize);
    if (tempBuffer == NULL) {
        return;
    }

    memcpy(tempBuffer, configData, receivedDataLength);

    applyConfig(configType, tempBuffer, receivedDataLength);

    free(tempBuffer);
}
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#define MAX_UDS_PAYLOAD_TOTAL_SIZE 4096
struct UdsMessage_t {
    uint16_t serviceId;
    uint16_t payloadLength;
    uint8_t payload[MAX_UDS_PAYLOAD_TOTAL_SIZE];
};
enum NvmWriteStatus_e {
    NVM_WRITE_OK,
    NVM_WRITE_ERROR
};
struct DataIdentifierProperties {
    uint16_t id;
    uint16_t maxDataSize;
};
static const DataIdentifierProperties diPropertiesTable[] = {
    {0xF180, 64},
    {0xF181, 256},
    {0xF182, 128},
    {0xDE01, 32},
    {0xDE02, 16}
};
static const uint16_t diPropertiesTableSize = sizeof(diPropertiesTable) / sizeof(diPropertiesTable[0]);
static uint16_t getDiMaxDataSize(uint16_t dataIdentifier) {
    for (uint16_t i = 0; i < diPropertiesTableSize; ++i) {
        if (diPropertiesTable[i].id == dataIdentifier) {
            return diPropertiesTable[i].maxDataSize;
        }
    }
    return 0;
}
NvmWriteStatus_e writeDataToNvm(uint16_t dataIdentifier, const uint8_t* data, uint16_t length) {
    if (length > 0 && data != NULL) {
        return NVM_WRITE_OK;
    }
    return NVM_WRITE_ERROR;
}
void handleUdsWriteDataByIdentifier_Fixed(const UdsMessage_t* udsMsg) {
    if (udsMsg->serviceId != 0x2E) {
        return;
    }
    if (udsMsg->payloadLength < 2) {
        return;
    }
    uint16_t dataIdentifier = (udsMsg->payload[0] << 8) | udsMsg->payload[1];
    const uint8_t* sourceDataBuffer = &udsMsg->payload[2];
    uint16_t receivedDataLength = udsMsg->payloadLength - 2;
    uint16_t maxAllowedDataSize = getDiMaxDataSize(dataIdentifier);
    if (maxAllowedDataSize == 0) {
        return;
    }
    uint16_t bytesToCopy = receivedDataLength;
    if (bytesToCopy > maxAllowedDataSize) {
        bytesToCopy = maxAllowedDataSize;
    }
    uint8_t* tempBuffer = (uint8_t*)malloc(maxAllowedDataSize);
    if (tempBuffer == NULL) {
        return;
    }
    memcpy(tempBuffer, sourceDataBuffer, bytesToCopy);
    writeDataToNvm(dataIdentifier, tempBuffer, bytesToCopy);
    free(tempBuffer);
}
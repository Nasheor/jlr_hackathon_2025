#include <stdint.h>
#include <string.h>

#define DIAG_LOG_BUFFER_SIZE 10
#define DIAG_LOG_ENTRY_DATA_SIZE 16

typedef struct {
    uint32_t timestamp;
    uint16_t identifier;
    uint8_t data[DIAG_LOG_ENTRY_DATA_SIZE];
    uint8_t dataLength;
    uint8_t checksum;
} DiagnosticLogEntry_t;

static DiagnosticLogEntry_t g_diagLogBuffer[DIAG_LOG_BUFFER_SIZE];
static uint8_t g_diagLogWriteIndex = 0;

#define UDS_SID_WRITE_DATA_BY_IDENTIFIER 0x2F

void Ecu_LogError(const char* msg) {
    (void)msg;
}

uint32_t Ecu_GetTimestampMs() {
    static uint32_t current_time = 0;
    current_time += 100;
    return current_time;
}

void UDS_ProcessWriteDataByIdentifier(const uint8_t* udsPayload, uint16_t payloadLength) {
    if (udsPayload == NULL || payloadLength < 4) {
        Ecu_LogError("UDS: Invalid payload for WriteDataByIdentifier.");
        return;
    }

    uint16_t dataIdentifier = (udsPayload[0] << 8) | udsPayload[1];
    uint16_t receivedDataRecordLength = (udsPayload[2] << 8) | udsPayload[3];
    
    const uint8_t* dataRecordStart = &udsPayload[4];
    uint16_t bytesRemainingInPayload = payloadLength - 4;

    if (receivedDataRecordLength > bytesRemainingInPayload) {
        Ecu_LogError("UDS: Reported data record length exceeds available payload bytes.");
        return;
    }

    DiagnosticLogEntry_t* currentEntry = &g_diagLogBuffer[g_diagLogWriteIndex];
    currentEntry->timestamp = Ecu_GetTimestampMs();
    currentEntry->identifier = dataIdentifier;
    currentEntry->dataLength = (uint8_t)receivedDataRecordLength;
    currentEntry->checksum = 0xAA;

    memcpy(currentEntry->data, dataRecordStart, receivedDataRecordLength);

    g_diagLogWriteIndex = (g_diagLogWriteIndex + 1) % DIAG_LOG_BUFFER_SIZE;
}

void UDS_ProcessRequest(const uint8_t* request, uint16_t requestLength) {
    if (request == NULL || requestLength == 0) {
        Ecu_LogError("UDS: Empty request.");
        return;
    }

    uint8_t serviceId = request[0];
    const uint8_t* payload = &request[1];
    uint16_t payloadLength = requestLength - 1;

    switch (serviceId) {
        case UDS_SID_WRITE_DATA_BY_IDENTIFIER:
            UDS_ProcessWriteDataByIdentifier(payload, payloadLength);
            break;
        default:
            Ecu_LogError("UDS: Unknown service ID.");
            break;
    }
}
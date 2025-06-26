#include <stdint.h>
#include <string.h>

#define VEHICLE_RECORDER_BUFFER_SIZE 4096
#define MAX_EVENT_DATA_SEGMENT_SIZE 1024

typedef enum {
    VDR_EVENT_TYPE_ACCELERATION = 0x01,
    VDR_EVENT_TYPE_BRAKING = 0x02,
    VDR_EVENT_TYPE_STEERING = 0x03,
    VDR_EVENT_TYPE_DIAGNOSTIC = 0x04
} VDR_EventType_t;

typedef struct {
    uint16_t totalLength;
    uint32_t timestampMs;
    VDR_EventType_t eventType;
    uint8_t sequenceNum;
} VDR_EventHeader_t;

static uint8_t s_recorderBuffer[VEHICLE_RECORDER_BUFFER_SIZE];
static uint16_t s_currentRecorderOffset = 0;

void VDR_LogError(const char* message) {
    (void)message;
}

void VDR_InitRecorder() {
    s_currentRecorderOffset = 0;
    memset(s_recorderBuffer, 0, VEHICLE_RECORDER_BUFFER_SIZE);
}

void VDR_RecordEvent(VDR_EventType_t type, uint32_t timestamp, const uint8_t* eventData, uint16_t dataLength) {
    VDR_EventHeader_t header;
    uint16_t headerSize = sizeof(VDR_EventHeader_t);
    uint32_t requiredSpace; 
    uint16_t actualDataLength = dataLength;

    if (actualDataLength > MAX_EVENT_DATA_SEGMENT_SIZE) {
        VDR_LogError("VDR: Event data length exceeds max segment size. Truncating.");
        actualDataLength = MAX_EVENT_DATA_SEGMENT_SIZE;
    }

    requiredSpace = (uint32_t)headerSize + actualDataLength;

    if (requiredSpace > 0xFFFF) {
        VDR_LogError("VDR: Calculated total event length would exceed uint16_t max. Event rejected.");
        return;
    }

    if (s_currentRecorderOffset + requiredSpace > VEHICLE_RECORDER_BUFFER_SIZE) {
        VDR_LogError("VDR: Not enough space for new event. Log buffer full.");
        return;
    }

    header.totalLength = (uint16_t)requiredSpace;
    header.timestampMs = timestamp;
    header.eventType = type;
    header.sequenceNum = 0;

    memcpy(&s_recorderBuffer[s_currentRecorderOffset], &header, headerSize);
    memcpy(&s_recorderBuffer[s_currentRecorderOffset + headerSize], eventData, actualDataLength);

    s_currentRecorderOffset += header.totalLength;
}
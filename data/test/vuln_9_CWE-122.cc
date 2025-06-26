typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#include <stdlib.h>
#include <string.h>

#define APP_MSG_VEHICLE_PARAM_SET       0x1001
#define APP_MSG_DIAG_LIVE_DATA_REQ      0x1002
#define APP_MSG_SENSOR_CALIBRATION_CMD  0x1003

#define MAX_PAYLOAD_VEHICLE_PARAM_SET       32
#define MAX_PAYLOAD_DIAG_LIVE_DATA_REQ      16
#define MAX_PAYLOAD_SENSOR_CALIBRATION_CMD  64

typedef struct {
    uint16_t message_id;
    uint16_t payload_length;
} AppMessageHeader_t;

typedef struct {
    uint8_t param_type;
    uint8_t param_id;
    uint16_t value_len;
    uint8_t value[28];
} VehicleParameterSet_t;

static uint8_t* g_currentAppPayloadBuffer = NULL;
static uint16_t g_currentAppPayloadAllocatedSize = 0;
static uint16_t g_currentAppMessageId = 0;

void resetAppPayloadBuffer_Vulnerable() {
    if (g_currentAppPayloadBuffer != NULL) {
        free(g_currentAppPayloadBuffer);
        g_currentAppPayloadBuffer = NULL;
        g_currentAppPayloadAllocatedSize = 0;
        g_currentAppMessageId = 0;
    }
}

static uint16_t getMaxAllowedPayloadSize(uint16_t msg_id) {
    switch (msg_id) {
        case APP_MSG_VEHICLE_PARAM_SET:
            return MAX_PAYLOAD_VEHICLE_PARAM_SET;
        case APP_MSG_DIAG_LIVE_DATA_REQ:
            return MAX_PAYLOAD_DIAG_LIVE_DATA_REQ;
        case APP_MSG_SENSOR_CALIBRATION_CMD:
            return MAX_PAYLOAD_SENSOR_CALIBRATION_CMD;
        default:
            return 0;
    }
}

void processIncomingAppMessage_Vulnerable(const uint8_t* raw_msg_buffer, uint16_t buffer_len) {
    if (raw_msg_buffer == NULL || buffer_len < sizeof(AppMessageHeader_t)) {
        return;
    }

    const AppMessageHeader_t* header = (const AppMessageHeader_t*)raw_msg_buffer;

    uint16_t msg_id = header->message_id;
    uint16_t received_payload_length = header->payload_length;

    if (buffer_len < (sizeof(AppMessageHeader_t) + received_payload_length)) {
        return;
    }

    uint16_t max_allowed_size_for_type = getMaxAllowedPayloadSize(msg_id);

    if (max_allowed_size_for_type == 0) {
        return;
    }

    resetAppPayloadBuffer_Vulnerable();

    g_currentAppPayloadBuffer = (uint8_t*)malloc(max_allowed_size_for_type);
    if (g_currentAppPayloadBuffer == NULL) {
        return;
    }
    g_currentAppPayloadAllocatedSize = max_allowed_size_for_type;
    g_currentAppMessageId = msg_id;

    const uint8_t* payload_src = raw_msg_buffer + sizeof(AppMessageHeader_t);

    memcpy(g_currentAppPayloadBuffer, payload_src, received_payload_length);
}

const uint8_t* getCurrentAppPayload_Vulnerable(uint16_t* size_out, uint16_t* msg_id_out) {
    if (size_out) *size_out = g_currentAppPayloadAllocatedSize;
    if (msg_id_out) *msg_id_out = g_currentAppMessageId;
    return g_currentAppPayloadBuffer;
}
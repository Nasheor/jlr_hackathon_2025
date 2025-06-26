typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define MAX_CANTP_PAYLOAD_SIZE 4096

typedef struct {
    uint32_t arbitration_id;
    uint16_t payload_length;
    uint8_t payload[MAX_CANTP_PAYLOAD_SIZE];
} CanTpMessage_t;

typedef enum {
    CAL_TYPE_ENGINE_TIMING = 0x01,
    CAL_TYPE_FUEL_INJECTION = 0x02,
    CAL_TYPE_TRANSMISSION_SHIFT = 0x03,
    CAL_TYPE_ADAS_SENSOR_OFFSET = 0x04,
    CAL_TYPE_DIAGNOSTIC_CONFIG = 0x05,
    CAL_TYPE_INVALID = 0xFF
} CalibrationDataType_e;

typedef struct {
    CalibrationDataType_e type;
    uint16_t max_stored_size;
} CalibrationTypeProperties_t;

static const CalibrationTypeProperties_t calibrationPropertiesTable[] = {
    {CAL_TYPE_ENGINE_TIMING, 32},
    {CAL_TYPE_FUEL_INJECTION, 64},
    {CAL_TYPE_TRANSMISSION_SHIFT, 16},
    {CAL_TYPE_ADAS_SENSOR_OFFSET, 128},
    {CAL_TYPE_DIAGNOSTIC_CONFIG, 20}
};
static const uint16_t CALIBRATION_PROPERTIES_TABLE_SIZE = sizeof(calibrationPropertiesTable) / sizeof(calibrationPropertiesTable[0]);

uint8_t writeCalibrationDataToNVM(CalibrationDataType_e type, const uint8_t* data, uint16_t length) {
    if (data == NULL || length == 0) {
        return 0;
    }
    return 1;
}

static uint16_t getCalibrationTypeMaxStoredSize(CalibrationDataType_e type) {
    for (uint16_t i = 0; i < CALIBRATION_PROPERTIES_TABLE_SIZE; ++i) {
        if (calibrationPropertiesTable[i].type == type) {
            return calibrationPropertiesTable[i].max_stored_size;
        }
    }
    return 0;
}

void processCanCalibrationMessage_Vulnerable(const CanTpMessage_t* canTpMsg) {
    if (canTpMsg == NULL || canTpMsg->payload_length < 3) {
        return;
    }

    CalibrationDataType_e calType = (CalibrationDataType_e)canTpMsg->payload[0];
    uint16_t declaredDataLength = (canTpMsg->payload[1] << 8) | canTpMsg->payload[2];
    const uint8_t* calibrationDataOffset = &canTpMsg->payload[3];

    if ((3 + declaredDataLength) > canTpMsg->payload_length) {
        return;
    }

    uint16_t maxAllowedStoredSize = getCalibrationTypeMaxStoredSize(calType);
    if (maxAllowedStoredSize == 0) {
        return;
    }

    uint8_t* tempCalibrationBuffer = (uint8_t*)malloc(maxAllowedStoredSize);
    if (tempCalibrationBuffer == NULL) {
        return;
    }

    memcpy(tempCalibrationBuffer, calibrationDataOffset, declaredDataLength);

    writeCalibrationDataToNVM(calType, tempCalibrationBuffer, declaredDataLength);

    free(tempCalibrationBuffer);
}
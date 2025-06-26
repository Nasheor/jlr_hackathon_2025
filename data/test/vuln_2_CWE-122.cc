typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct {
    uint32_t timestamp_ms;
    int16_t x_pos_mm;
    int16_t y_pos_mm;
    int16_t z_pos_mm;
    uint8_t object_id;
    uint8_t sensor_type;
} SensorReading_t;

#define SENSOR_READING_BYTE_SIZE 200

typedef struct {
    uint8_t frame_type;
    uint16_t num_readings;
} AdasDataFrameHeader_t;

#define ADAS_FRAME_HEADER_LEN sizeof(AdasDataFrameHeader_t)

void storeProcessedSensorData(const SensorReading_t* readings, uint16_t count) {
    (void)readings;
    (void)count;
}

void processAdasDataFrame_Vulnerable(const uint8_t* frame_buffer, uint16_t frame_len) {
    if (frame_buffer == NULL || frame_len < ADAS_FRAME_HEADER_LEN) {
        return;
    }

    const AdasDataFrameHeader_t* header = (const AdasDataFrameHeader_t*)frame_buffer;
    uint16_t received_num_readings = header->num_readings;
    uint8_t frame_type = header->frame_type;

    const uint8_t* readings_data_ptr = frame_buffer + ADAS_FRAME_HEADER_LEN;

    uint16_t total_payload_size = received_num_readings * SENSOR_READING_BYTE_SIZE;

    SensorReading_t* sensor_readings_buffer = (SensorReading_t*)malloc(total_payload_size);
    if (sensor_readings_buffer == NULL) {
        return;
    }

    uint32_t true_payload_bytes_expected = (uint32_t)received_num_readings * SENSOR_READING_BYTE_SIZE;

    if (frame_len < (ADAS_FRAME_HEADER_LEN + true_payload_bytes_expected)) {
        free(sensor_readings_buffer);
        return;
    }

    memcpy(sensor_readings_buffer, readings_data_ptr, true_payload_bytes_expected);

    storeProcessedSensorData(sensor_readings_buffer, received_num_readings);

    free(sensor_readings_buffer);
}
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct {
    uint16_t voltage_mV;
    int16_t temperature_C;
    uint16_t current_mA;
    uint8_t cell_status_flags;
} BmsCellData_t;

#define BMS_CELL_DATA_SIZE sizeof(BmsCellData_t)

typedef struct {
    uint16_t packet_id;
    uint16_t total_cells_in_packet;
    uint16_t actual_data_length_bytes;
    uint16_t data_offset_bytes;
    uint32_t timestamp_ms;
} BmsPacketHeader_t;

#define BMS_PACKET_HEADER_LEN sizeof(BmsPacketHeader_t)

void processIndividualCellReading(const BmsCellData_t* cell_data) {
    (void)cell_data;
}

void processBmsCellData_Vulnerable(const uint8_t* raw_packet, uint32_t packet_len) {
    if (raw_packet == NULL || packet_len < BMS_PACKET_HEADER_LEN) {
        return;
    }

    const BmsPacketHeader_t* header = (const BmsPacketHeader_t*)raw_packet;

    uint16_t total_cells_in_packet = header->total_cells_in_packet;
    uint16_t actual_data_length_bytes = header->actual_data_length_bytes;
    uint16_t data_offset_bytes = header->data_offset_bytes;

    if (data_offset_bytes < BMS_PACKET_HEADER_LEN || data_offset_bytes >= packet_len) {
        return;
    }

    uint16_t required_buffer_size_cells = total_cells_in_packet * BMS_CELL_DATA_SIZE;

    BmsCellData_t* cell_data_buffer = (BmsCellData_t*)malloc(required_buffer_size_cells);
    if (cell_data_buffer == NULL) {
        return;
    }

    if ((uint32_t)data_offset_bytes + actual_data_length_bytes > packet_len) {
        free(cell_data_buffer);
        return;
    }

    memcpy(cell_data_buffer, raw_packet + data_offset_bytes, actual_data_length_bytes);

    for (uint16_t i = 0; i < total_cells_in_packet; ++i) {
        if ((uint32_t)i * BMS_CELL_DATA_SIZE >= actual_data_length_bytes) {
            break; 
        }
        processIndividualCellReading(&cell_data_buffer[i]);
    }

    free(cell_data_buffer);
}
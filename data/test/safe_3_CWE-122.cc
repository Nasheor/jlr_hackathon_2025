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

#define MAX_ALLOWED_BMS_CELLS_PER_PACKET 1024

void processIndividualCellReading(const BmsCellData_t* cell_data) {
    (void)cell_data;
}

void processBmsCellData_Fixed(const uint8_t* raw_packet, uint32_t packet_len) {
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

    if (total_cells_in_packet == 0 || total_cells_in_packet > MAX_ALLOWED_BMS_CELLS_PER_PACKET) {
        return;
    }

    uint32_t calculated_buffer_size = (uint32_t)total_cells_in_packet * BMS_CELL_DATA_SIZE;

    if (actual_data_length_bytes > calculated_buffer_size ||
        (uint32_t)data_offset_bytes + actual_data_length_bytes > packet_len) {
        return;
    }
    
    if (calculated_buffer_size == 0) {
        return;
    }

    BmsCellData_t* cell_data_buffer = (BmsCellData_t*)malloc(calculated_buffer_size);
    if (cell_data_buffer == NULL) {
        return;
    }

    memcpy(cell_data_buffer, raw_packet + data_offset_bytes, actual_data_length_bytes);

    uint16_t num_cells_to_process = actual_data_length_bytes / BMS_CELL_DATA_SIZE;

    for (uint16_t i = 0; i < num_cells_to_process; ++i) {
        processIndividualCellReading(&cell_data_buffer[i]);
    }

    free(cell_data_buffer);
}
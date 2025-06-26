typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define UDS_READ_DATA_BY_IDENTIFIER 0x22

#define DID_VEHICLE_SPEED_SENSOR_CALIBRATION 0xF001
#define DID_O2_SENSOR_HEATER_CURRENT_LIMIT   0xF002
#define DID_ENGINE_RUNTIME_HOURS             0xF003

#define MAX_SIZE_DID_F001 16
#define MAX_SIZE_DID_F002 8
#define MAX_SIZE_DID_F003 4

typedef struct {
    uint16_t did;
    uint16_t max_data_length;
} DataIdentifierProperties_t;

static const DataIdentifierProperties_t did_properties[] = {
    {DID_VEHICLE_SPEED_SENSOR_CALIBRATION, MAX_SIZE_DID_F001},
    {DID_O2_SENSOR_HEATER_CURRENT_LIMIT, MAX_SIZE_DID_F002},
    {DID_ENGINE_RUNTIME_HOURS, MAX_SIZE_DID_F003}
};
static const uint32_t NUM_DID_PROPERTIES = sizeof(did_properties) / sizeof(did_properties[0]);

uint16_t getDidMaxDataLength(uint16_t did) {
    for (uint32_t i = 0; i < NUM_DID_PROPERTIES; ++i) {
        if (did_properties[i].did == did) {
            return did_properties[i].max_data_length;
        }
    }
    return 0;
}

#define MAX_TEMP_READ_BUFFER_SIZE 256

uint16_t readDidFromNvm(uint16_t did, uint8_t* buffer, uint16_t buffer_size) {
    uint16_t max_len = getDidMaxDataLength(did);
    if (buffer == NULL || buffer_size == 0 || max_len == 0) {
        return 0;
    }
    uint16_t bytes_to_fill = (buffer_size < max_len) ? buffer_size : max_len;
    for (uint16_t i = 0; i < bytes_to_fill; ++i) {
        buffer[i] = (uint8_t)(did % 256 + i % 256); 
    }
    return bytes_to_fill;
}

typedef struct {
    uint8_t service_id;
    uint16_t data_identifier;
    uint16_t requested_length;
} UdsReadDidRequest_t;

typedef struct {
    uint8_t service_id_ack;
    uint16_t data_identifier;
} UdsReadDidResponseHeader_t;

#define UDS_READ_DID_RESPONSE_HEADER_SIZE sizeof(UdsReadDidResponseHeader_t)

uint8_t* handleUdsReadDid_Fixed(const uint8_t* request_buffer, uint16_t request_len, uint16_t* response_len_out) {
    if (response_len_out) *response_len_out = 0;
    if (request_buffer == NULL || request_len < sizeof(UdsReadDidRequest_t)) {
        return NULL;
    }

    const UdsReadDidRequest_t* req = (const UdsReadDidRequest_t*)request_buffer;

    if (req->service_id != UDS_READ_DATA_BY_IDENTIFIER) {
        return NULL;
    }

    uint16_t requested_did = req->data_identifier;
    uint16_t client_requested_length = req->requested_length;

    uint16_t actual_did_max_length = getDidMaxDataLength(requested_did);

    if (actual_did_max_length == 0) {
        return NULL;
    }
    
    uint16_t allocated_data_size = actual_did_max_length;
    uint16_t total_response_size = UDS_READ_DID_RESPONSE_HEADER_SIZE + allocated_data_size;

    uint8_t* response_buffer = (uint8_t*)malloc(total_response_size);
    if (response_buffer == NULL) {
        return NULL;
    }

    UdsReadDidResponseHeader_t* resp_header = (UdsReadDidResponseHeader_t*)response_buffer;
    resp_header->service_id_ack = UDS_READ_DATA_BY_IDENTIFIER + 0x40;
    resp_header->data_identifier = requested_did;

    uint8_t* data_payload_ptr = response_buffer + UDS_READ_DID_RESPONSE_HEADER_SIZE;

    uint8_t temp_read_data[MAX_TEMP_READ_BUFFER_SIZE];
    uint16_t actual_bytes_read_from_nvm = readDidFromNvm(requested_did, temp_read_data, actual_did_max_length);
    
    uint16_t bytes_to_copy = client_requested_length;
    if (bytes_to_copy > actual_did_max_length) {
        bytes_to_copy = actual_did_max_length;
    }
    if (bytes_to_copy > actual_bytes_read_from_nvm) {
        bytes_to_copy = actual_bytes_read_from_nvm;
    }

    memcpy(data_payload_ptr, temp_read_data, bytes_to_copy);

    if (response_len_out) {
        *response_len_out = UDS_READ_DID_RESPONSE_HEADER_SIZE + bytes_to_copy;
    }

    return response_buffer;
}
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

enum FirmwareVariant_e {
    FW_VARIANT_A = 0x01,
    FW_VARIANT_B = 0x02,
    FW_VARIANT_C = 0x03,
    FW_VARIANT_UNKNOWN = 0xFF
};

typedef struct {
    FirmwareVariant_e variant;
    uint32_t size_bytes;
} FirmwareVariantInfo_t;

static const FirmwareVariantInfo_t firmwareVariantTable[] = {
    {FW_VARIANT_A, 10240},
    {FW_VARIANT_B, 20480},
    {FW_VARIANT_C, 30720}
};
static const uint32_t NUM_FIRMWARE_VARIANTS = sizeof(firmwareVariantTable) / sizeof(firmwareVariantTable[0]);

uint32_t getFirmwareVariantSize(FirmwareVariant_e variant) {
    for (uint32_t i = 0; i < NUM_FIRMWARE_VARIANTS; ++i) {
        if (firmwareVariantTable[i].variant == variant) {
            return firmwareVariantTable[i].size_bytes;
        }
    }
    return 0;
}

#define MAX_SUPPORTED_FIRMWARE_UPDATE_SIZE 32768

class FirmwareUpdater_Vulnerable {
private:
    uint8_t* updateBuffer;
    uint32_t allocatedBufferSize;
    FirmwareVariant_e currentVariant;

public:
    FirmwareUpdater_Vulnerable() : updateBuffer(nullptr), allocatedBufferSize(0), currentVariant(FW_VARIANT_UNKNOWN) {}

    ~FirmwareUpdater_Vulnerable() {
        if (updateBuffer) {
            free(updateBuffer);
        }
    }

    void initializeUpdate(FirmwareVariant_e variant_type) {
        if (updateBuffer) {
            free(updateBuffer);
            updateBuffer = nullptr;
        }
        
        currentVariant = variant_type;
        uint32_t variantSize = getFirmwareVariantSize(variant_type);

        if (variantSize == 0 || variantSize > MAX_SUPPORTED_FIRMWARE_UPDATE_SIZE) {
            allocatedBufferSize = 0;
            return;
        }

        updateBuffer = (uint8_t*)malloc(variantSize);
        if (updateBuffer == nullptr) {
            allocatedBufferSize = 0;
            return;
        }
        allocatedBufferSize = variantSize;
    }

    void processFirmwareChunk_Vulnerable(const uint8_t* chunk_data, uint32_t chunk_offset, uint32_t chunk_len, uint32_t total_msg_len) {
        if (chunk_data == nullptr || chunk_len == 0 || updateBuffer == nullptr || allocatedBufferSize == 0) {
            return;
        }

        if (chunk_offset + chunk_len > total_msg_len) {
            return;
        }

        memcpy(updateBuffer + chunk_offset, chunk_data, chunk_len);
    }

    uint8_t applyFirmwareUpdate_Vulnerable() {
        if (updateBuffer == nullptr || allocatedBufferSize == 0) {
            return 0;
        }
        return 1;
    }
}
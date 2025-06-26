#include <stdint.h>
#include <string.h>

#define CAL_MAX_PAYLOAD_SIZE 2048
#define CAL_BLOCK_A_SIZE 64
#define CAL_BLOCK_B_SIZE 128
#define CAL_BLOCK_C_SIZE 32

typedef enum {
    CAL_BLOCK_ID_A = 0x01,
    CAL_BLOCK_ID_B = 0x02,
    CAL_BLOCK_ID_C = 0x03,
    CAL_BLOCK_ID_INVALID = 0xFF
} CalBlockId_t;

static uint8_t g_calDataA[CAL_BLOCK_A_SIZE];
static uint8_t g_calDataB[CAL_BLOCK_B_SIZE];
static uint8_t g_calDataC[CAL_BLOCK_C_SIZE];

void CalManager_LogWarning(const char* msg) {
    (void)msg;
}

void CalManager_ApplyBlock(CalBlockId_t blockId, const uint8_t* blockData, uint16_t dataLength) {
    switch (blockId) {
        case CAL_BLOCK_ID_A:
            memcpy(g_calDataA, blockData, dataLength);
            break;
        case CAL_BLOCK_ID_B:
            memcpy(g_calDataB, blockData, dataLength);
            break;
        case CAL_BLOCK_ID_C:
            memcpy(g_calDataC, blockData, dataLength);
            break;
        default:
            CalManager_LogWarning("CalManager: Attempted to apply invalid calibration block ID.");
            break;
    }
}

void CalManager_ProcessUpdatePayload(const uint8_t* payload, uint16_t payloadSize) {
    uint16_t currentOffset = 0;

    if (payload == NULL || payloadSize == 0) {
        CalManager_LogWarning("CalManager: Empty or null calibration update payload.");
        return;
    }

    while (currentOffset < payloadSize) {
        if (payloadSize - currentOffset < 3) {
            CalManager_LogWarning("CalManager: Malformed calibration block header.");
            break;
        }

        CalBlockId_t blockId = (CalBlockId_t)payload[currentOffset];
        uint16_t dataLength = (payload[currentOffset + 1] << 8) | payload[currentOffset + 2];
        
        uint16_t blockHeaderSize = 3;
        uint16_t totalBlockSize = blockHeaderSize + dataLength;

        if ((uint32_t)currentOffset + totalBlockSize > payloadSize) {
            CalManager_LogWarning("CalManager: Calibration block data exceeds remaining payload bounds.");
            break;
        }

        CalManager_ApplyBlock(blockId, &payload[currentOffset + blockHeaderSize], dataLength);

        currentOffset += totalBlockSize;
    }
}
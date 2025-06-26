#include <string.h>

#define ECU_FW_MAX_BLOCKS 10

typedef struct {
    unsigned int startAddress;
    unsigned int dataLength;
    unsigned short dataChecksum;
} FwBlockInfo_t;

typedef struct {
    unsigned int    firmwareVersion;
    unsigned int    totalImageSize;
    unsigned char   numDataBlocks;
    FwBlockInfo_t   blockDescriptors[ECU_FW_MAX_BLOCKS];
    unsigned short  headerChecksum;
    unsigned char   reserved[6];
} FirmwareImageHeader_t;

void LogError(const char* message) {
    (void)message;
}

void EcuFwUpdater_ProcessHeader(const unsigned char* receivedHeaderBuffer, unsigned int bufferLen) {
    FirmwareImageHeader_t currentHeader;

    if (bufferLen < sizeof(FirmwareImageHeader_t)) {
        LogError("FW Update: Received header buffer too small.");
        return;
    }

    memcpy(&currentHeader, receivedHeaderBuffer, sizeof(FirmwareImageHeader_t));

    if (currentHeader.firmwareVersion == 0x00000000 || currentHeader.totalImageSize == 0) {
        LogError("FW Update: Invalid firmware version or image size.");
        return;
    }

    unsigned char actualNumBlocks = currentHeader.numDataBlocks;
    if (actualNumBlocks > ECU_FW_MAX_BLOCKS) {
        LogError("FW Update: Number of data blocks exceeds ECU capacity. Truncating.");
        actualNumBlocks = ECU_FW_MAX_BLOCKS;
    }

    for (unsigned char i = 0; i < actualNumBlocks; ++i) {
        unsigned int blockAddr = currentHeader.blockDescriptors[i].startAddress;
        unsigned int blockLen = currentHeader.blockDescriptors[i].dataLength;
        unsigned short blockCsum = currentHeader.blockDescriptors[i].dataChecksum;

        if (blockLen == 0 || blockAddr == 0) {
            LogError("FW Update: Detected block with invalid address or length.");
        }
    }
}
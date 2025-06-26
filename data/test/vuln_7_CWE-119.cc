#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FIRMWARE_PAYLOAD_CHUNK_SIZE 256
#define FIRMWARE_BUFFER_ALIGNMENT 4

typedef struct {
    uint8_t* firmwareBuffer;
    uint32_t totalFirmwareSize;
    uint32_t currentWrittenSize;
    uint8_t updateStatus;
} FirmwareUpdater_t;

void App_LogError(const char* msg) {
}

void FirmwareUpdater_Init(FirmwareUpdater_t* updater, uint32_t totalSize) {
    if (updater == NULL) {
        App_LogError("FirmwareUpdater: Null updater context.");
        return;
    }

    if (updater->firmwareBuffer != NULL) {
        free(updater->firmwareBuffer);
        updater->firmwareBuffer = NULL;
    }

    updater->firmwareBuffer = (uint8_t*)malloc(totalSize);
    if (updater->firmwareBuffer == NULL) {
        App_LogError("FirmwareUpdater: Failed to allocate firmware buffer.");
        updater->totalFirmwareSize = 0;
        updater->currentWrittenSize = 0;
        updater->updateStatus = 4;
        return;
    }

    updater->totalFirmwareSize = totalSize;
    updater->currentWrittenSize = 0;
    updater->updateStatus = 1;
    App_LogError("FirmwareUpdater: Initialized with total size.");
}

void FirmwareUpdater_WriteChunk(FirmwareUpdater_t* updater, uint32_t offset, const uint8_t* data, uint16_t length) {
    if (updater == NULL || updater->firmwareBuffer == NULL || data == NULL) {
        App_LogError("FirmwareUpdater: Invalid arguments for write chunk.");
        return;
    }

    if (updater->updateStatus != 1 && updater->updateStatus != 2) {
        App_LogError("FirmwareUpdater: Not in writing state.");
        return;
    }

    if (length > MAX_FIRMWARE_PAYLOAD_CHUNK_SIZE) {
        App_LogError("FirmwareUpdater: Incoming chunk length too large. Truncating.");
        length = MAX_FIRMWARE_PAYLOAD_CHUNK_SIZE;
    }

    memcpy(updater->firmwareBuffer + offset, data, length);

    if (offset + length > updater->currentWrittenSize) {
        updater->currentWrittenSize = offset + length;
    }
    updater->updateStatus = 2;
}

void FirmwareUpdater_Finalize(FirmwareUpdater_t* updater) {
    if (updater == NULL || updater->firmwareBuffer == NULL) {
        App_LogError("FirmwareUpdater: Cannot finalize. Not initialized.");
        return;
    }

    if (updater->currentWrittenSize != updater->totalFirmwareSize) {
        App_LogError("FirmwareUpdater: Incomplete firmware received.");
        updater->updateStatus = 4;
        return;
    }

    App_LogError("FirmwareUpdater: Flashing complete firmware image.");
    updater->updateStatus = 3;

    free(updater->firmwareBuffer);
    updater->firmwareBuffer = NULL;
    updater->totalFirmwareSize = 0;
    updater->currentWrittenSize = 0;
}

void FirmwareUpdater_Cleanup(FirmwareUpdater_t* updater) {
    if (updater != NULL && updater->firmwareBuffer != NULL) {
        free(updater->firmwareBuffer);
        updater->firmwareBuffer = NULL;
    }
    if (updater != NULL) {
        updater->totalFirmwareSize = 0;
        updater->currentWrittenSize = 0;
        updater->updateStatus = 0;
    }
}
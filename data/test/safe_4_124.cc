#include <cstdint>
#include <array>
#include <cstring>

#define FIRMWARE_IMAGE_SIZE 4096

// Simulates a firmware image buffer in memory, e.g., in flash or RAM
// for an ECU update process.
std::array<uint8_t, FIRMWARE_IMAGE_SIZE> ecuFirmwareImage;

// Initialize the firmware image with some dummy data
void initializeFirmwareImage() {
    for (size_t i = 0; i < FIRMWARE_IMAGE_SIZE; ++i) {
        ecuFirmwareImage[i] = static_cast<uint8_t>(i % 256);
    }
}

// Function to reset a "block integrity marker".
// This marker is assumed to be part of the header of the *current* block,
// but its offset is somehow derived incorrectly from a *previous* block's end or footer.
// The vulnerability arises if `marker_offset_from_block_start`
// is larger than `currentBlockBaseAddress`, causing an underwrite.
void resetBlockIntegrityMarker(uint32_t currentBlockBaseAddress, uint16_t marker_offset_from_block_start) {
    if (currentBlockBaseAddress >= FIRMWARE_IMAGE_SIZE) {
        return;
    }

    // Validate the offset to prevent underwrite.
    // The offset to subtract must not be greater than the base address,
    // otherwise the pointer would go before the start of the buffer.
    if (marker_offset_from_block_start > currentBlockBaseAddress) {
        return;
    }

    uint8_t* currentBlockPtr = ecuFirmwareImage.data() + currentBlockBaseAddress;
    uint8_t* targetMarkerPtr = currentBlockPtr - marker_offset_from_block_start;

    *targetMarkerPtr = 0xDE;
}

// Global function to be called from main or other components
void ecuUpdateRoutine() {
    initializeFirmwareImage();
    resetBlockIntegrityMarker(10, 20);
}

uint8_t getFirmwareByte(uint32_t offset) {
    if (offset < FIRMWARE_IMAGE_SIZE) {
        return ecuFirmwareImage[offset];
    }
    return 0;
}
void updateFirmwareBlockStatus(uint8_t* firmwareImage, size_t imageSize, size_t currentBlockOffset, size_t previousHeaderSize) {
    if (firmwareImage == nullptr || imageSize == 0) {
        return;
    }

    uint8_t* statusBytePtr = firmwareImage + currentBlockOffset - previousHeaderSize;

    *statusBytePtr = 0xFE;
}
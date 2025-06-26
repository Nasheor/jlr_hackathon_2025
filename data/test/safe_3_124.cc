#include <cstdint>
#include <array>
#include <cstring>

#define KEY_STORE_CAPACITY_BYTES 256
#define KEY_BLOCK_SIZE_BYTES 64
#define NUM_KEY_BLOCKS (KEY_STORE_CAPACITY_BYTES / KEY_BLOCK_SIZE_BYTES)

class SecureKeyStore {
public:
    SecureKeyStore() {
        for (size_t i = 0; i < KEY_STORE_CAPACITY_BYTES; ++i) {
            keyStoreBuffer[i] = 0x00;
        }
    }

    uint8_t* getKeyBlockAddress(uint16_t keyBlockIndex) {
        if (keyBlockIndex >= NUM_KEY_BLOCKS) {
            return nullptr;
        }
        return keyStoreBuffer.data() + (keyBlockIndex * KEY_BLOCK_SIZE_BYTES);
    }

    void updateKeyFragment(uint16_t keyBlockIndex, uint16_t offsetFromBlockEnd, const uint8_t* fragment, uint16_t fragmentLength) {
        uint8_t* blockStart = getKeyBlockAddress(keyBlockIndex);
        if (blockStart == nullptr || fragment == nullptr) {
            return;
        }

        uint8_t* blockEnd = blockStart + KEY_BLOCK_SIZE_BYTES;
        uint8_t* writePtr = blockEnd - offsetFromBlockEnd;

        if (writePtr < keyStoreBuffer.data() || (writePtr + fragmentLength) > (keyStoreBuffer.data() + KEY_STORE_CAPACITY_BYTES)) {
            return;
        }
        
        std::memcpy(writePtr, fragment, fragmentLength);
    }

private:
    std::array<uint8_t, KEY_STORE_CAPACITY_BYTES> keyStoreBuffer;
};

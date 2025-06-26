typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#include <stdlib.h>
#include <string.h>

#define MAX_ADAS_CONFIG_MSG_SIZE 2048
#define MAX_PARAMS_IN_CONFIG 64

typedef enum {
    ADAS_PARAM_TYPE_UNKNOWN = 0x00,
    ADAS_PARAM_TYPE_INT = 0x01,
    ADAS_PARAM_TYPE_FLOAT = 0x02,
    ADAS_PARAM_TYPE_STRING = 0x03,
    ADAS_PARAM_TYPE_BYTE_ARRAY = 0x04
} AdasParameterType_e;

#define MAX_STRING_PARAM_LENGTH 256
#define MAX_BYTE_ARRAY_PARAM_LENGTH 512

typedef struct {
    AdasParameterType_e paramType;
    uint16_t paramId;
    uint16_t dataLength;
    uint8_t* data;
} AdasConfigParameter_t;

class AdasConfigManager_Vulnerable {
private:
    AdasConfigParameter_t* configParameters;
    uint16_t currentParamCount;
    uint16_t maxParamCapacity;

    uint16_t getExpectedMaxDataLengthForType(AdasParameterType_e type) {
        switch (type) {
            case ADAS_PARAM_TYPE_INT:
                return sizeof(uint32_t);
            case ADAS_PARAM_TYPE_FLOAT:
                return sizeof(float);
            case ADAS_PARAM_TYPE_STRING:
                return MAX_STRING_PARAM_LENGTH;
            case ADAS_PARAM_TYPE_BYTE_ARRAY:
                return MAX_BYTE_ARRAY_PARAM_LENGTH;
            default:
                return 0;
        }
    }

    void clearConfig() {
        if (configParameters) {
            for (uint16_t i = 0; i < currentParamCount; ++i) {
                if (configParameters[i].data) {
                    free(configParameters[i].data);
                    configParameters[i].data = nullptr;
                }
            }
            free(configParameters);
            configParameters = nullptr;
        }
        currentParamCount = 0;
        maxParamCapacity = 0;
    }

public:
    AdasConfigManager_Vulnerable() : configParameters(nullptr), currentParamCount(0), maxParamCapacity(0) {}

    ~AdasConfigManager_Vulnerable() {
        clearConfig();
    }

    void updateAdasConfiguration_Vulnerable(const uint8_t* msg_buffer, uint16_t buffer_len) {
        if (msg_buffer == nullptr || buffer_len < sizeof(uint16_t)) {
            return;
        }

        clearConfig();

        uint16_t total_params = (msg_buffer[0] << 8) | msg_buffer[1];
        uint16_t current_offset = sizeof(uint16_t);

        if (total_params == 0) {
            return;
        }
        
        if (total_params > MAX_PARAMS_IN_CONFIG) {
            return;
        }

        configParameters = (AdasConfigParameter_t*)malloc(total_params * sizeof(AdasConfigParameter_t));
        if (configParameters == nullptr) {
            return;
        }
        maxParamCapacity = total_params;

        for (uint16_t i = 0; i < total_params; ++i) {
            if (current_offset + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t) > buffer_len) {
                break;
            }

            AdasParameterType_e paramType = (AdasParameterType_e)msg_buffer[current_offset];
            current_offset += sizeof(uint8_t);
            
            uint16_t paramId = (msg_buffer[current_offset] << 8) | msg_buffer[current_offset + 1];
            current_offset += sizeof(uint16_t);

            uint16_t dataLength = (msg_buffer[current_offset] << 8) | msg_buffer[current_offset + 1];
            current_offset += sizeof(uint16_t);

            if (current_offset + dataLength > buffer_len) {
                break;
            }

            configParameters[i].paramType = paramType;
            configParameters[i].paramId = paramId;
            configParameters[i].dataLength = dataLength;
            configParameters[i].data = nullptr;

            if (dataLength > 0) {
                uint16_t expectedMaxLenForType = getExpectedMaxDataLengthForType(paramType);
                if (expectedMaxLenForType == 0) {
                    current_offset += dataLength;
                    continue;
                }

                configParameters[i].data = (uint8_t*)malloc(expectedMaxLenForType);
                if (configParameters[i].data == nullptr) {
                    break;
                }
                
                memcpy(configParameters[i].data, msg_buffer + current_offset, dataLength);
            }
            currentParamCount++;
            current_offset += dataLength;
        }
    }
}
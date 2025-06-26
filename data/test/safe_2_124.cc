#include <cstdint>
#include <array>

#define DIAG_LOG_BUFFER_SIZE 256
#define DIAG_LOG_HEADER_SIZE 4

class DiagnosticDataLogger {
public:
    enum LogMarkerType {
        LOG_MARKER_START_SESSION = 0x01,
        LOG_MARKER_END_SESSION   = 0x02
    };

    DiagnosticDataLogger() : currentLogPtr(logBuffer.data() + DIAG_LOG_HEADER_SIZE) {
        for (size_t i = 0; i < DIAG_LOG_HEADER_SIZE; ++i) {
            logBuffer[i] = 0x00;
        }
    }

    void recordLogMarker(uint8_t markerType, uint16_t offsetFromDataStart) {
        if (offsetFromDataStart > DIAG_LOG_HEADER_SIZE) {
            return;
        }

        uint8_t* targetPtr = currentLogPtr - offsetFromDataStart;
        
        if (targetPtr >= logBuffer.data() + DIAG_LOG_BUFFER_SIZE) {
            return;
        }

        *targetPtr = markerType;
    }

    void processIncomingTelemetry(const uint8_t* telemetryData, uint16_t dataLen) {
        if (currentLogPtr + dataLen < logBuffer.data() + DIAG_LOG_BUFFER_SIZE) {
            for (uint16_t i = 0; i < dataLen; ++i) {
                *currentLogPtr++ = telemetryData[i];
            }
        }
    }

    uint16_t getLoggedDataSize() const {
        return currentLogPtr - (logBuffer.data() + DIAG_LOG_HEADER_SIZE);
    }

private:
    std::array<uint8_t, DIAG_LOG_BUFFER_SIZE> logBuffer;
    uint8_t* currentLogPtr;
};
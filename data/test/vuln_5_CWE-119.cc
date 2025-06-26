#include <stdint.h>
#include <string.h>

#define DIAG_RAM_BUFFER_SIZE 256
#define DIAG_COMMAND_PAYER_MAX_WRITE_SIZE 64

static uint8_t s_diagRamBuffer[DIAG_RAM_BUFFER_SIZE];

void DiagComm_LogError(const char* msg) {
    (void)msg;
}

typedef enum {
    DIAG_SVC_READ_DATA_BY_ID = 0x22,
    DIAG_SVC_WRITE_MEMORY_BY_ADDRESS = 0x3D,
    DIAG_SVC_ECU_RESET = 0x11
} DiagnosticServiceId_t;

typedef struct {
    DiagnosticServiceId_t serviceId;
    uint16_t dataIdentifier;
    uint16_t memoryAddress;
    uint16_t length;
    uint8_t payload[DIAG_COMMAND_PAYER_MAX_WRITE_SIZE];
} DiagRequest_t;

void Diagnostic_ProcessRequest(const DiagRequest_t* request) {
    if (request == NULL) {
        DiagComm_LogError("Diag: Null request received.");
        return;
    }

    switch (request->serviceId) {
        case DIAG_SVC_WRITE_MEMORY_BY_ADDRESS: {
            if (request->length > DIAG_COMMAND_PAYER_MAX_WRITE_SIZE) {
                DiagComm_LogError("Diag: Write memory length exceeds command payload capacity.");
                return;
            }

            memcpy(&s_diagRamBuffer[request->memoryAddress], request->payload, request->length);
            break;
        }
        case DIAG_SVC_READ_DATA_BY_ID: {
            DiagComm_LogError("Diag: Read Data By ID service requested.");
            if (request->dataIdentifier == 0xF001) {
                if (request->length <= DIAG_COMMAND_PAYER_MAX_WRITE_SIZE) {

                }
            }
            break;
        }
        case DIAG_SVC_ECU_RESET: {
            DiagComm_LogError("Diag: ECU Reset command issued.");
            break;
        }
        default: {
            DiagComm_LogError("Diag: Unsupported diagnostic service ID.");
            break;
        }
    }
}
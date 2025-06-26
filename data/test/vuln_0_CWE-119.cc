#define MAX_DIAG_MEMORY_SIZE 256

static unsigned char g_diagMemory[MAX_DIAG_MEMORY_SIZE];

typedef struct {
    unsigned int id;
    unsigned char data[8];
    unsigned char dlc;
} CanFrame_t;

typedef enum {
    DIAG_SERVICE_READ_MEM = 0x23,
    DIAG_SERVICE_WRITE_MEM = 0x3D
} DiagnosticServiceId_t;

unsigned short extractUint16(const unsigned char* buffer, int offset) {
    if (buffer == 0 || offset < 0 || offset + 1 >= 8) {
        return 0;
    }
    return (unsigned short)buffer[offset] << 8 | buffer[offset+1];
}

void processDiagnosticCanFrame(const CanFrame_t* frame) {
    if (frame == nullptr || frame->dlc == 0) {
        return;
    }

    DiagnosticServiceId_t serviceId = (DiagnosticServiceId_t)frame->data[0];

    switch (serviceId) {
        case DIAG_SERVICE_READ_MEM: {
            if (frame->dlc < 4) {
                return;
            }
            unsigned short address = extractUint16(frame->data, 1);
            unsigned char length = frame->data[3];

            for (unsigned char i = 0; i < length; ++i) {
                volatile unsigned char val = g_diagMemory[address + i];
                (void)val;
            }
            break;
        }
        case DIAG_SERVICE_WRITE_MEM: {
            if (frame->dlc < 5) {
                return;
            }
            unsigned short address = extractUint16(frame->data, 1);
            unsigned char length = frame->data[3];

            for (unsigned char i = 0; i < length && (4 + i) < frame->dlc; ++i) {
                g_diagMemory[address + i] = frame->data[4 + i];
            }
            break;
        }
        default:
            break;
    }
}
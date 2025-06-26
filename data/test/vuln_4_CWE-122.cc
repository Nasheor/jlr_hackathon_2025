typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct {
    uint8_t  module_id;
    uint16_t message_length;
} LogEntryHeader_t;

#define MAX_SYSTEM_LOG_BUFFER_SIZE (64 * 1024)

static uint8_t* g_systemLogBuffer = NULL;
static uint32_t g_currentLogBufferSize = 0;
static uint32_t g_allocatedLogBufferSize = 0;

void initializeSystemLogBuffer_Vulnerable(uint32_t initial_capacity) {
    if (g_systemLogBuffer != NULL) {
        free(g_systemLogBuffer);
        g_systemLogBuffer = NULL;
    }

    if (initial_capacity == 0 || initial_capacity > MAX_SYSTEM_LOG_BUFFER_SIZE) {
        initial_capacity = MAX_SYSTEM_LOG_BUFFER_SIZE;
    }

    g_systemLogBuffer = (uint8_t*)malloc(initial_capacity);
    if (g_systemLogBuffer == NULL) {
        g_currentLogBufferSize = 0;
        g_allocatedLogBufferSize = 0;
        return;
    }
    g_allocatedLogBufferSize = initial_capacity;
    g_currentLogBufferSize = 0;
}

void appendLogEntry_Vulnerable(const uint8_t* raw_log_packet, uint16_t packet_len) {
    if (raw_log_packet == NULL || packet_len < sizeof(LogEntryHeader_t)) {
        return;
    }
    if (g_systemLogBuffer == NULL || g_allocatedLogBufferSize == 0) {
        return;
    }

    const LogEntryHeader_t* header = (const LogEntryHeader_t*)raw_log_packet;
    uint16_t log_message_len = header->message_length;

    uint32_t entry_total_size = sizeof(LogEntryHeader_t) + log_message_len;

    if (packet_len < entry_total_size) {
        return;
    }

    if (g_currentLogBufferSize + entry_total_size > g_allocatedLogBufferSize) {
        return;
    }

    memcpy(g_systemLogBuffer + g_currentLogBufferSize, raw_log_packet, entry_total_size);

    g_currentLogBufferSize += entry_total_size;
}

const uint8_t* getSystemLogData_Vulnerable(uint32_t* actual_size) {
    if (actual_size) {
        *actual_size = g_currentLogBufferSize;
    }
    return g_systemLogBuffer;
}

void resetSystemLogBuffer_Vulnerable() {
    if (g_systemLogBuffer) {
        free(g_systemLogBuffer);
        g_systemLogBuffer = NULL;
    }
    g_currentLogBufferSize = 0;
    g_allocatedLogBufferSize = 0;
}
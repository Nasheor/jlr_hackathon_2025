typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct {
    uint8_t  module_id;
    uint16_t message_length;
} LogEntryHeader_t;

#define MAX_SYSTEM_LOG_BUFFER_SIZE (64 * 1024)

static uint8_t* g_systemLogBuffer_Fixed = NULL;
static uint32_t g_currentLogBufferSize_Fixed = 0;
static uint32_t g_allocatedLogBufferSize_Fixed = 0;

void initializeSystemLogBuffer_Fixed(uint32_t initial_capacity) {
    if (g_systemLogBuffer_Fixed != NULL) {
        free(g_systemLogBuffer_Fixed);
        g_systemLogBuffer_Fixed = NULL;
    }

    if (initial_capacity == 0 || initial_capacity > MAX_SYSTEM_LOG_BUFFER_SIZE) {
        initial_capacity = MAX_SYSTEM_LOG_BUFFER_SIZE;
    }

    g_systemLogBuffer_Fixed = (uint8_t*)malloc(initial_capacity);
    if (g_systemLogBuffer_Fixed == NULL) {
        g_currentLogBufferSize_Fixed = 0;
        g_allocatedLogBufferSize_Fixed = 0;
        return;
    }
    g_allocatedLogBufferSize_Fixed = initial_capacity;
    g_currentLogBufferSize_Fixed = 0;
}

void appendLogEntry_Fixed(const uint8_t* raw_log_packet, uint16_t packet_len) {
    if (raw_log_packet == NULL || packet_len < sizeof(LogEntryHeader_t)) {
        return;
    }
    if (g_systemLogBuffer_Fixed == NULL || g_allocatedLogBufferSize_Fixed == 0) {
        return;
    }

    const LogEntryHeader_t* header = (const LogEntryHeader_t*)raw_log_packet;
    uint16_t log_message_len = header->message_length;

    uint32_t entry_total_size = sizeof(LogEntryHeader_t) + log_message_len;

    if (packet_len < entry_total_size) {
        return;
    }

    uint64_t potential_new_size = (uint64_t)g_currentLogBufferSize_Fixed + entry_total_size;

    if (potential_new_size > g_allocatedLogBufferSize_Fixed) {
        return;
    }

    memcpy(g_systemLogBuffer_Fixed + g_currentLogBufferSize_Fixed, raw_log_packet, entry_total_size);

    g_currentLogBufferSize_Fixed += entry_total_size;
}

const uint8_t* getSystemLogData_Fixed(uint32_t* actual_size) {
    if (actual_size) {
        *actual_size = g_currentLogBufferSize_Fixed;
    }
    return g_systemLogBuffer_Fixed;
}

void resetSystemLogBuffer_Fixed() {
    if (g_systemLogBuffer_Fixed) {
        free(g_systemLogBuffer_Fixed);
        g_systemLogBuffer_Fixed = NULL;
    }
    g_currentLogBufferSize_Fixed = 0;
    g_allocatedLogBufferSize_Fixed = 0;
}
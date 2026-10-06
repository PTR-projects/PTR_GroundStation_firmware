#include "FS.h"
#include "SPIFFS.h"

#define FS_LOG_PATH "/log.csv"
#define FS_LOG_RESERVE_BYTES 32768
#define FS_LOG_MAX_FILE_BYTES (1024UL * 1024UL)
#define FS_LOG_MAX_LINE_BYTES 1024
#define FS_LOG_MIN_HEAP_BYTES 20480
#define FS_LOG_MIN_ALLOC_BYTES 4096
#define FS_LOG_MAX_FAILURES 5

bool FS_init();
fs::FS& FS_logFS();
const char* FS_logPath();
bool FS_logExists();
size_t FS_logFreeBytes();
bool FS_logHasSpace(size_t needed);
void FS_logAppend(const char * message);
void FS_logDelete();

void writeFile(fs::FS &fs, const char * path, const char * message);
bool appendFile(fs::FS &fs, const char * path, const char * message);
void deleteFile(fs::FS &fs, const char * path);

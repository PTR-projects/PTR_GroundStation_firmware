#include "Arduino.h"
#include "BOARD.h"
#include "FileSys.h"
#include <string.h>

static fs::FS *log_fs = &SPIFFS;
static bool log_ready = false;
static bool log_disabled = false;
static uint8_t log_fail_count = 0;
static bool log_skip_warned = false;

static void FS_logWarnOnce(const char *reason) {
    if (log_skip_warned) {
        return;
    }
    Serial.printf("[FS] Skip telemetry log: %s (SPIFFS %u/%u, heap %u, maxalloc %u)\n",
                  reason,
                  (unsigned)SPIFFS.usedBytes(),
                  (unsigned)SPIFFS.totalBytes(),
                  (unsigned)ESP.getFreeHeap(),
                  (unsigned)ESP.getMaxAllocHeap());
    log_skip_warned = true;
}

bool FS_init(){
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS init failed");
        log_ready = false;
        return false;
    }
    log_fs = &SPIFFS;
    log_ready = true;
    log_disabled = false;
    log_fail_count = 0;
    log_skip_warned = false;
    Serial.printf("Telemetry log: SPIFFS %s (%u / %u bytes used)\n",
                  FS_LOG_PATH,
                  (unsigned)SPIFFS.usedBytes(),
                  (unsigned)SPIFFS.totalBytes());

    return true;
}

fs::FS& FS_logFS(){
    return *log_fs;
}

const char* FS_logPath(){
    return FS_LOG_PATH;
}

bool FS_logExists(){
    return log_ready && log_fs->exists(FS_LOG_PATH);
}

size_t FS_logFreeBytes(){
    if (!log_ready) {
        return 0;
    }
    size_t total = SPIFFS.totalBytes();
    size_t used = SPIFFS.usedBytes();
    if (used >= total) {
        return 0;
    }
    return total - used;
}

bool FS_logHasSpace(size_t needed){
    if (!log_ready || needed == 0 || needed > FS_LOG_MAX_LINE_BYTES) {
        return false;
    }
    if (FS_logFreeBytes() < (needed + FS_LOG_RESERVE_BYTES)) {
        return false;
    }
    if (SPIFFS.exists(FS_LOG_PATH)) {
        File file = SPIFFS.open(FS_LOG_PATH, FILE_READ);
        if (file) {
            size_t file_size = file.size();
            file.close();
            if (file_size >= FS_LOG_MAX_FILE_BYTES ||
                (FS_LOG_MAX_FILE_BYTES - file_size) < needed) {
                return false;
            }
        }
    }
    return true;
}

void FS_logAppend(const char * message){
    if (!log_ready || log_disabled) {
        return;
    }
    if (message == nullptr || message[0] == '\0') {
        return;
    }

    size_t needed = strnlen(message, FS_LOG_MAX_LINE_BYTES + 1);
    if (needed == 0 || needed > FS_LOG_MAX_LINE_BYTES) {
        FS_logWarnOnce("line too long");
        return;
    }

    if (ESP.getFreeHeap() < FS_LOG_MIN_HEAP_BYTES ||
        ESP.getMaxAllocHeap() < FS_LOG_MIN_ALLOC_BYTES) {
        FS_logWarnOnce("low heap");
        return;
    }

    if (!FS_logHasSpace(needed)) {
        FS_logWarnOnce("not enough SPIFFS space");
        log_disabled = true;
        return;
    }

    if (!appendFile(SPIFFS, FS_LOG_PATH, message)) {
        log_fail_count++;
        if (log_fail_count >= FS_LOG_MAX_FAILURES) {
            log_disabled = true;
            Serial.println(F("[FS] Telemetry log disabled after repeated write failures"));
        }
        return;
    }

    log_fail_count = 0;
    log_skip_warned = false;
}

void FS_logDelete(){
    deleteFile(SPIFFS, FS_LOG_PATH);
    log_disabled = false;
    log_fail_count = 0;
    log_skip_warned = false;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.printf("Writing file: %s - failed to open file for writing", path);
        return;
    }
    if(file.print(message)){
        //Serial.println("- file written");
    } else {
        Serial.printf("Writing file: %s - write failed", path);
    }
    file.close();
}

bool appendFile(fs::FS &fs, const char * path, const char * message){
    if (path == nullptr || message == nullptr) {
        return false;
    }

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.printf("Appending to file: %s - failed to open file for appending\r\n", path);
        return false;
    }

    size_t written = file.print(message);
    bool ok = (written == strlen(message));
    file.flush();
    file.close();
    if (!ok) {
        Serial.printf("Appending to file: %s- append failed\r\n", path);
    }
    return ok;
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

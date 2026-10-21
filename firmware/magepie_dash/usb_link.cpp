#include "usb_link.h"
#include "ui.h"
#include "display_port.h"  // dbg_verbose
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <sys/time.h>

// UI owns framing and serial I/O; the fetch worker owns JSON parsing. No LVGL
// objects are touched here and no HTTP wait is introduced into the UI loop.
struct Request { uint32_t seq; char path[128]; };
struct Reply { uint32_t seq; char *body; size_t size; };
static QueueHandle_t requests, replies;
static volatile uint32_t last_hello = 0;
static volatile bool linked = false;
static uint32_t next_seq = 0;
static const uint32_t MAX_BODY = 262144;
static char header[96];
static size_t header_pos = 0, body_pos = 0, body_size = 0;
static uint32_t body_seq = 0, wanted_crc = 0, crc = 0xffffffff, rx_ms = 0;
static char kind = 0;
static char cmd_buf[64];
static volatile bool cmd_pending = false;
static char *body = nullptr;
static enum { IDLE, HEADER, BODY, DISCARD } state = IDLE;

void usb_link_begin() {
    requests = xQueueCreate(1, sizeof(Request));
    replies = xQueueCreate(1, sizeof(Reply));
    if (!requests || !replies) { Serial.println("[usb] queue allocation failed"); return; }
    Serial.println("[usb] MG1 ready; USB data mode, no WiFi required");
}

bool usb_link_connected() { return linked && millis() - last_hello < 20000; }

static void reset_rx() {
    if (body) heap_caps_free(body);
    body = nullptr; header_pos = body_pos = body_size = 0; state = IDLE;
}

static void finish_frame() {
    if (dbg_verbose) Serial.printf("[usb] rx %c seq=%u bytes=%u crc=ok\n", kind, (unsigned)body_seq, (unsigned)body_size);
    if ((crc ^ 0xffffffff) != wanted_crc) {
        Serial.printf("[usb] CRC rejected kind=%c seq=%u bytes=%u got=%08x want=%08x\n",
                      kind, (unsigned)body_seq, (unsigned)body_size,
                      (unsigned)(crc ^ 0xffffffff), (unsigned)wanted_crc);
        reset_rx(); return;
    }
    body[body_size] = 0;
    if (kind == 'H') {
        JsonDocument doc;
        if (!deserializeJson(doc, body, body_size)) {
            int64_t epoch = doc["epoch"] | (int64_t)0;
            if (epoch > 1700000000) {
                timeval tv{(time_t)epoch, 0}; settimeofday(&tv, nullptr);
            }
            bool was_linked = usb_link_connected();
            last_hello = millis(); linked = true;
            if (!was_linked) {
                Serial.println("[usb] link up"); app_request_refresh();
            }
        }
    } else if (kind == 'C') {
        // Control commands are length+CRC framed, so serial noise cannot reach them.
        size_t n = body_size < sizeof(cmd_buf) - 1 ? body_size : sizeof(cmd_buf) - 1;
        memcpy(cmd_buf, body, n);
        cmd_buf[n] = 0;
        cmd_pending = true;
    } else {
        Reply r{body_seq, body, body_size};
        if (xQueueSend(replies, &r, 0) == pdTRUE) body = nullptr;
    }
    reset_rx();
}

bool usb_link_receive(uint8_t c) {
    if (state == IDLE) {
        if (c != '!') return false;
        state = HEADER; header_pos = 0;
    }
    rx_ms = millis();
    if (state == DISCARD) {
        if (c == '!') { state = HEADER; header_pos = 0; }
        else return true;
    }
    if (state == HEADER) {
        if (c == '\r') return true;
        if (c != '\n') {
            if (header_pos + 1 >= sizeof(header)) { state = DISCARD; return true; }
            header[header_pos++] = c; return true;
        }
        header[header_pos] = 0;
        unsigned long seq, size, checksum;
        if (sscanf(header, "!MG1 %c %lu %lu %lu", &kind, &seq, &size, &checksum) != 4 ||
            (kind != 'H' && kind != 'R' && kind != 'C') || size == 0 || size > MAX_BODY) {
            state = DISCARD; return true;
        }
        body_size = size; body_seq = seq; wanted_crc = checksum;
        body = (char *)heap_caps_malloc(body_size + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!body) { state = DISCARD; return true; }
        crc = 0xffffffff; body_pos = 0; state = BODY; return true;
    }
    body[body_pos++] = c;
    crc ^= c;
    for (int i = 0; i < 8; ++i) crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320 : 0);
    if (body_pos == body_size) finish_frame();
    return true;
}

void usb_link_tick() {
    if (state != IDLE && millis() - rx_ms > 1500) {
        // A truncated frame must not stall the next one for long: drop and resync.
        if (state == BODY) Serial.printf("[usb] frame aborted at %u/%u bytes\n", (unsigned)body_pos, (unsigned)body_size);
        reset_rx();
    }
    if (linked && !usb_link_connected()) {
        linked = false;
        // Drop anything queued for the dead link: replaying a stale request on
        // the next handshake costs the board a full 8 s timeout at reconnect.
        Request stale;
        while (requests && xQueueReceive(requests, &stale, 0) == pdTRUE) {}
        Serial.println("[usb] link offline; keeping last data");
    }
    if (!requests || !usb_link_connected()) return;
    Request r;
    if (xQueuePeek(requests, &r, 0) == pdTRUE) {
        char line[168];
        int size = snprintf(line, sizeof(line), "MG_GET %u %s\n", (unsigned)r.seq, r.path);
        if (Serial.availableForWrite() >= size && xQueueReceive(requests, &r, 0) == pdTRUE)
            Serial.write((const uint8_t *)line, size);
    }
}

bool usb_link_take_command(char *out, size_t cap) {
    if (!cmd_pending) return false;
    cmd_pending = false;
    snprintf(out, cap, "%s", cmd_buf);
    return true;
}

bool usb_link_get_json(const String &path, JsonDocument &doc) {
    if (!requests || !replies || !usb_link_connected()) return false;
    Reply old;
    while (xQueueReceive(replies, &old, 0) == pdTRUE) heap_caps_free(old.body);
    Request req{}; req.seq = ++next_seq;
    path.toCharArray(req.path, sizeof(req.path));
    if (xQueueSend(requests, &req, pdMS_TO_TICKS(100)) != pdTRUE) return false;
    uint32_t start = millis();
    while (millis() - start < 2500) {
        if (!usb_link_connected()) break;  // host went away: fail fast, next cycle retries
        Reply r;
        if (xQueueReceive(replies, &r, pdMS_TO_TICKS(100)) != pdTRUE) continue;
        if (r.seq != req.seq) { heap_caps_free(r.body); continue; }
        DeserializationError err = deserializeJson(doc, (const char *)r.body, r.size);
        heap_caps_free(r.body);
        return !err && doc["_usb_error"].isNull();
    }
    Serial.printf("[usb] request timeout seq=%u\n", (unsigned)req.seq);
    return false;
}

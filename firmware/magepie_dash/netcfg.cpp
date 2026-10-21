#include "netcfg.h"

#include "i18n.h"

#include "appcfg.h"

#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

#include "config.h"

static Preferences prefs;
static WebServer server(80);

static String s_ssid, s_pass, s_host;
static uint16_t s_port = GW_PORT_DEFAULT;
static bool s_ready = false;
static bool s_portal = false;
static uint32_t s_last_retry = 0;
static uint8_t s_attempt = 0;
static bool s_joining = false;
static uint32_t s_join_start = 0;
static uint32_t s_join_dot_ms = 0;

static void start_join() {
    s_joining = true;
    s_join_start = millis();
    s_join_dot_ms = millis();
    WiFi.begin(s_ssid.c_str(), s_pass.c_str());
    Serial.printf("[net] joining \"%s\" (attempt %u, non-blocking)\n", s_ssid.c_str(),
                  (unsigned)s_attempt);
}

static const char *PORTAL_HTML = R"HTML(<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>magpie dash 设置</title><style>
body{background:#080C10;color:#EEEEF6;font-family:system-ui,-apple-system,sans-serif;margin:0;padding:24px}
h1{font-size:18px;color:#DE7552;font-weight:600;margin:0 0 4px}
p.sub{color:#7E7C89;font-size:12px;margin:0 0 20px}
label{display:block;font-size:12px;color:#9A98A8;margin:14px 0 6px}
input{width:100%;box-sizing:border-box;background:#181820;border:1px solid #383749;color:#EEEEF6;
padding:10px 12px;border-radius:8px;font-size:14px;outline:none}
input:focus{border-color:#7B6DFF}
button{width:100%;margin-top:22px;background:#7B6DFF;color:#fff;border:0;padding:12px;
border-radius:8px;font-size:15px;font-weight:600}
.hint{color:#5C5A68;font-size:11px;margin-top:16px;line-height:1.5}
</style></head><body>
<h1>magpie dash</h1><p class="sub">连接 WiFi 并填写网关地址，保存后自动重启</p>
<form method="POST" action="/save">
<label>WiFi 名称 (SSID)</label><input name="ssid" value="%SSID%" required>
<label>WiFi 密码</label><input name="pass" type="password" value="%PASS%">
<label>网关地址 (运行 magpie 的电脑 IP)</label><input name="host" value="%HOST%" placeholder="192.168.1.100" required>
<label>端口</label><input name="port" value="%PORT%" required>
<button type="submit">保存并重启</button></form>
<div class="hint">提示：电脑与这块屏要在同一个路由器下；电脑 IP 可以在 ipconfig / 系统设置里查到。</div>
</body></html>)HTML";

// The setup page is read on a phone, so it follows the panel's language. Same
// markup, English wording (the WiFi/SSID/port fields stay as they are).
static const char *PORTAL_HTML_EN = R"HTML(<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>magpie dash setup</title><style>
body{background:#080C10;color:#EEEEF6;font-family:system-ui,-apple-system,sans-serif;margin:0;padding:24px}
h1{font-size:18px;color:#DE7552;font-weight:600;margin:0 0 4px}
p.sub{color:#7E7C89;font-size:12px;margin:0 0 20px}
label{display:block;font-size:12px;color:#9A98A8;margin:14px 0 6px}
input{width:100%;box-sizing:border-box;background:#181820;border:1px solid #383749;color:#EEEEF6;
padding:10px 12px;border-radius:8px;font-size:14px;outline:none}
input:focus{border-color:#7B6DFF}
button{width:100%;margin-top:22px;background:#7B6DFF;color:#fff;border:0;padding:12px;
border-radius:8px;font-size:15px;font-weight:600}
.hint{color:#5C5A68;font-size:11px;margin-top:16px;line-height:1.5}
</style></head><body>
<h1>magpie dash</h1><p class="sub">Join WiFi, enter the gateway address, save to reboot</p>
<form method="POST" action="/save">
<label>WiFi SSID</label><input name="ssid" value="%SSID%" required>
<label>Password</label><input name="pass" type="password" value="%PASS%">
<label>Gateway IP (host PC running magpie)</label><input name="host" value="%HOST%" placeholder="192.168.1.100" required>
<label>Port</label><input name="port" value="%PORT%" required>
<button type="submit">Save & Reboot</button></form>
<div class="hint">Note: the PC and this screen must be on the same router. Find the PC's IP with ipconfig (Windows) or ifconfig (macOS/Linux).</div>
</body></html>)HTML";

static void handle_root() {
    String html = lang_get() == LANG_EN ? PORTAL_HTML_EN : PORTAL_HTML;
    html.replace("%SSID%", s_ssid);
    html.replace("%PASS%", s_pass);
    html.replace("%HOST%", s_host.length() ? s_host : String(GW_HOST_DEFAULT));
    html.replace("%PORT%", String(s_port));
    server.send(200, "text/html; charset=utf-8", html);
}

static void handle_save() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    String host = server.arg("host");
    int port = server.arg("port").toInt();
    if (ssid.length() == 0 || host.length() == 0) {
        server.send(400, "text/plain", "missing fields");
        return;
    }
    if (port <= 0 || port > 65535) port = GW_PORT_DEFAULT;
    prefs.begin("netcfg", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.putString("host", host);
    prefs.putUShort("port", (uint16_t)port);
    prefs.end();
    server.send(200, "text/html; charset=utf-8",
                String("<meta charset=utf-8><body style='background:#080C10;color:#EEEEF6;"
                       "font-family:sans-serif;padding:40px'>") +
                    T("已保存，正在重启…") + "</body>");
    delay(800);
    ESP.restart();
}

static void start_portal() {
    if (s_portal) return;
    s_portal = true;
    s_ready = false;
    // AP_STA, not AP: the setup page stays reachable, but the board keeps
    // trying the stored network in the background. A hotspot that was not up
    // yet at boot (Windows Mobile Hotspot takes a while, and a reflash reboots
    // the board) used to strand the dashboard in portal mode with no data until
    // someone power-cycled it.
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("magpie-dash-setup", "magpie123");
    WiFi.begin(s_ssid.c_str(), s_pass.c_str());
    s_last_retry = millis();
    server.on("/", HTTP_GET, handle_root);
    server.on("/save", HTTP_POST, handle_save);
    server.onNotFound(handle_root);  // captive-portal behaviour
    server.begin();
    Serial.println("[net] setup portal: AP magpie-dash-setup / http://192.168.4.1 (still retrying STA)");
}

static void load_prefs() {
    prefs.begin("netcfg", true);
    s_ssid = prefs.getString("ssid", "");
    s_pass = prefs.getString("pass", "");
    s_host = prefs.getString("host", "");
    s_port = prefs.getUShort("port", GW_PORT_DEFAULT);
    prefs.end();
    if (s_ssid.isEmpty()) s_ssid = WIFI_SSID;
    if (s_pass.isEmpty()) s_pass = WIFI_PASS;
    if (s_host.isEmpty()) s_host = GW_HOST_DEFAULT;
}

void netcfg_begin() {
    if (g_cfg.link_mode == 0) {
        // USB mode: leave the radio off entirely. Nothing to join, no AP.
        Serial.println("[net] link=USB (radio off; switch to WiFi in settings)");
        return;
    }
    load_prefs();
    if (s_ssid.isEmpty()) {
        Serial.println("[net] no WiFi credentials -> portal");
        start_portal();
        return;
    }
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    // Non-blocking: the dashboard must be touchable from the first frame. The
    // old version waited 2x12 s right after ui_init(), so every tap in the first
    // half minute after a boot or a reflash was ignored.
    s_attempt = 1;
    start_join();
}

void netcfg_loop() {
    if (s_portal) {
        server.handleClient();
        if (!s_ready && WiFi.status() == WL_CONNECTED) {
            // Self-healed: the stored network came back while we were in setup
            // mode. Drop the AP and run as a normal station again.
            s_ready = true;
            WiFi.softAPdisconnect(true);
            WiFi.mode(WIFI_STA);
            Serial.printf("[net] portal: joined %s, ip=%s gw=%s:%u\n", s_ssid.c_str(),
                          WiFi.localIP().toString().c_str(), s_host.c_str(), s_port);
        } else if (!s_ready && millis() - s_last_retry > 30000) {
            s_last_retry = millis();
            Serial.println("[net] portal: retry stored WiFi");
            s_attempt = 1;
            start_join();
        }
        return;
    }
    if (s_joining) {
        if (WiFi.status() == WL_CONNECTED) {
            s_joining = false;
            s_ready = true;
            Serial.printf("[net] connected, ip=%s gw=%s:%u\n", WiFi.localIP().toString().c_str(),
                          s_host.c_str(), s_port);
            return;
        }
        if (millis() - s_join_dot_ms > 2000) {
            s_join_dot_ms = millis();
            Serial.print('.');
        }
        if (millis() - s_join_start > 12000) {
            if (s_attempt < 2) {
                s_attempt++;
                start_join();
            } else {
                s_joining = false;
                Serial.println("\n[net] join failed -> portal (still retrying in background)");
                start_portal();
            }
        }
        return;
    }
    s_ready = WiFi.status() == WL_CONNECTED;
    if (!s_ready && millis() - s_last_retry > 20000) {
        s_last_retry = millis();
        Serial.println("[net] reconnect");
        s_attempt = 1;
        start_join();
    }
}

bool netcfg_ready() { return s_ready; }
bool netcfg_portal_active() { return s_portal; }
const char *netcfg_host() { return s_host.c_str(); }
uint16_t netcfg_port() { return s_port; }
const char *netcfg_ssid() { return s_ssid.c_str(); }

bool netcfg_retry() {
    if (g_cfg.link_mode == 0) return false;  // nothing to retry over USB
    if (!s_ssid.length()) {
        start_portal();
        return false;
    }
    s_attempt = 1;
    start_join();
    return true;
}

void netcfg_open_portal() { start_portal(); }

void netcfg_set_mode(uint8_t mode) {
    if (mode == g_cfg.link_mode) {
        if (mode == 0) {
            WiFi.mode(WIFI_OFF);
        } else {
            netcfg_retry();
        }
        return;
    }
    g_cfg.link_mode = mode;
    if (mode == 0) {
        s_portal = false;
        s_ready = false;
        s_joining = false;
        server.stop();
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        Serial.println("[net] switched to USB");
    } else {
        load_prefs();
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
        Serial.println("[net] switched to WiFi");
        if (s_ssid.isEmpty())
            start_portal();
        else {
            s_attempt = 1;
            start_join();
        }
    }
}

void netcfg_reset() {
    prefs.begin("netcfg", false);
    prefs.clear();
    prefs.end();
    ESP.restart();
}

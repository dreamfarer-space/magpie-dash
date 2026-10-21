#include "gateway.h"
#include "appcfg.h"
#include "usb_link.h"

#include "display_port.h"  // dbg_verbose

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "netcfg.h"
#include "util.h"

static char s_period[6] = "30d";

static bool s_light = false;
bool gw_take_light() {
    const bool l = s_light;
    s_light = false;
    return l;
}

void gw_set_period(const char *period) {
    if (period && *period) {
        strncpy(s_period, period, sizeof(s_period) - 1);
        s_period[sizeof(s_period) - 1] = 0;
        s_light = true;  // only /usage + /usage/requests change with the period
    }
}

// GET a gateway path into doc. Returns false on transport/parse failure.
// Which pipe is live right now. The chosen mode wins when both work, but a mode
// whose link is down must never strand the dashboard: a stray tap on the
// settings panel used to save "WiFi" and the board then ignored a perfectly good
// USB cable, which reads as "no data at all".
static bool usb_up() { return usb_link_connected(); }
static bool wifi_up() { return WiFi.status() == WL_CONNECTED; }
static bool usb_path() { return usb_up() && (g_cfg.link_mode == 0 || !wifi_up()); }
static bool wifi_path() { return wifi_up() && (g_cfg.link_mode == 1 || !usb_up()); }

static bool get_json(const String &path, JsonDocument &doc, uint32_t timeout_ms = 5000) {
    if (usb_path()) return usb_link_get_json(path, doc);
    if (!wifi_path()) return false;
    char url[128];
    snprintf(url, sizeof(url), "http://%s:%u%s", netcfg_host(), netcfg_port(), path.c_str());

    uint32_t t0 = millis();
    WiFiClient client;
    HTTPClient http;
    http.setTimeout(timeout_ms);
    http.setConnectTimeout(2500);
    http.setReuse(false);
    if (!http.begin(client, url)) return false;
    http.addHeader("Accept", "application/json");
    http.addHeader("Connection", "close");   // otherwise the relay keeps the
                                             // socket open and GET() waits
    int code = http.GET();
    if (code != 200) {
        if (dbg_verbose) Serial.printf("[gw] %s -> %d (%u ms)\n", path.c_str(), code, (unsigned)(millis() - t0));
        http.end();
        client.stop();
        return false;
    }
    // NOTE: with arduino-esp32 3.x the HTTPClient stream can come back empty for
    // chunked replies, so read the whole body first.
    String body = http.getString();
    http.end();
    client.stop();
    uint32_t dt = millis() - t0;
    if (body.length() == 0) {
        Serial.printf("[gw] %s empty body (%u ms)\n", path.c_str(), (unsigned)dt);
        return false;
    }
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        Serial.printf("[gw] %s parse: %s (len=%u, %u ms)\n", path.c_str(), err.c_str(),
                      (unsigned)body.length(), (unsigned)dt);
        return false;
    }
    if (dbg_verbose) Serial.printf("[gw] %s ok %u ms %u B\n", path.c_str(), (unsigned)dt, (unsigned)body.length());
    return true;
}

static void copy_str(char *dst, size_t n, const char *src) {
    if (!src) src = "";
    strncpy(dst, src, n - 1);
    dst[n - 1] = 0;
}

static void local_hms(const char *iso, char *out, size_t n) {
    out[0] = 0;
    if (!iso) return;
    const char *t = strchr(iso, 'T');
    if (!t) return;
    // gateway timestamps carry the PC's offset; use the wall clock part as-is
    snprintf(out, n, "%.2s:%.2s:%.2s", t + 1, t + 4, t + 7);
}

// ------------------------------------------------------------------ fetchers
static bool fetch_quotas(DashData &d) {
    JsonDocument doc;
    if (!get_json("/v1/magpie/quotas", doc)) return false;
    d.nprov = 0;
    for (JsonObject p : doc["data"].as<JsonArray>()) {
        if (d.nprov >= QUOTA_MAX_PROV) break;
        QuotaProvider &q = d.prov[d.nprov];
        copy_str(q.provider, sizeof(q.provider), p["provider"] | "");
        copy_str(q.name, sizeof(q.name), p["name"] | "");
        copy_str(q.plan, sizeof(q.plan), p["plan"] | "");
        copy_str(q.user, sizeof(q.user), p["user"] | "");
        local_hms(p["lastServedAt"] | "", q.lastServed, sizeof(q.lastServed));
        q.nwin = 0;
        for (JsonObject w : p["windows"].as<JsonArray>()) {
            if (q.nwin >= QUOTA_MAX_WIN) break;
            QuotaWindow &qw = q.win[q.nwin];
            copy_str(qw.name, sizeof(qw.name), w["name"] | "");
            qw.used = w["used"] | 0.0f;
            qw.remaining = w["remaining"] | 0.0f;
            char raw[32] = {0};
            copy_str(raw, sizeof(raw), w["resetsAt"] | "");
            fmt_reset(raw, qw.reset, sizeof(qw.reset));
            qw.reset_at = (int64_t)iso_to_epoch(raw);
            if (!qw.reset[0]) {
                const char *disp = w["display"] | "";
                copy_str(qw.reset, sizeof(qw.reset), disp);
            }
            q.nwin++;
        }
        d.nprov++;
    }
    return true;
}

static void fill_usage(JsonDocument &doc, DashData &d, bool series, bool agents) {
    d.calls = doc["calls"] | 0;
    d.errors = doc["errors"] | 0;
    d.tok_in = doc["input"] | (uint64_t)0;
    d.tok_out = doc["output"] | (uint64_t)0;
    d.tok_cache = doc["cache_read"] | (uint64_t)0;
    d.cost = doc["cost"] | 0.0f;
    d.unpriced = doc["unpriced"] | 0;
    // Agents only list the ones used inside that window, so a light cycle must
    // keep the catalogue from the last full one. Guard the agent loop ONLY:
    // returning here also skipped the series, and the chart then kept drawing
    // the previous period's buckets next to fresh KPI numbers.
    if (agents) {
        d.nagents = 0;
        d.agents_full = false;
        for (JsonObject a : doc["agents"].as<JsonArray>()) {
        if (d.nagents >= AGENT_MAX) break;
        const char *icon = a["icon"] | "";
        if (!strcmp(icon, "generic") || !strcmp(icon, "curl")) continue;  // scripts, not agents
        AgentRow &r = d.agents[d.nagents++];
        copy_str(r.id, sizeof(r.id), a["id"] | "");
        copy_str(r.name, sizeof(r.name), a["name"] | "");
        r.model[0] = 0;
        r.calls = a["calls"] | 0;
        uint64_t in = a["input"] | (uint64_t)0;
        uint64_t out = a["output"] | (uint64_t)0;
        uint64_t cr = a["cache_read"] | (uint64_t)0;
        r.tokens = (float)(in + out + cr);
        r.cost = a["cost"] | 0.0f;
        r.today = 0;
        r.last = 0;
        }
    }
    if (!series) return;
    d.nseries = 0;
    for (JsonObject s : doc["series"].as<JsonArray>()) {
        if (d.nseries >= SERIES_MAX) break;
        UsagePoint &pt = d.series[d.nseries];
        copy_str(pt.label, sizeof(pt.label), s["label"] | "");
        uint64_t in = s["input"] | (uint64_t)0;
        uint64_t out = s["output"] | (uint64_t)0;
        uint64_t cr = s["cache_read"] | (uint64_t)0;
        pt.tokens = (float)(in + out + cr);
        pt.calls = s["calls"] | 0;
        d.nseries++;
    }
}

static bool fetch_usage(DashData &d, bool with_agents = true) {
    JsonDocument doc;
    String path = String("/v1/magpie/usage?period=") + s_period;
    if (!get_json(path, doc)) return false;
    copy_str(d.period, sizeof(d.period), doc["period"] | s_period);
    fill_usage(doc, d, true, with_agents);
    return true;
}

static bool fetch_today(DashData &d, uint32_t *calls, float *tokens) {
    JsonDocument doc;
    if (!get_json("/v1/magpie/usage?period=today", doc)) return false;
    *calls = doc["calls"] | 0;
    uint64_t in = doc["input"] | (uint64_t)0;
    uint64_t out = doc["output"] | (uint64_t)0;
    uint64_t cr = doc["cache_read"] | (uint64_t)0;
    *tokens = (float)(in + out + cr);
    return true;
}

// latest rows, plus today's totals: unlike /usage, the request log also counts
// the calls magpie read from local session logs (Claude Code)
static bool fetch_requests(DashData &d, uint32_t *calls, float *tokens) {
    JsonDocument doc;
    // The period filter is what the user picked: hardcoding "today" here made the
    // route rows (and the session-log totals) disagree with the KPI cards above.
    String path = String("/v1/magpie/usage/requests?period=") + s_period + "&limit=12";
    if (!get_json(path, doc)) return false;
    // Only the "today" window can contribute to d.today_*; other periods must
    // leave the home page's today numbers alone.
    const bool is_today = !strcmp(s_period, "today");
    if (is_today) {
        if (!doc["sum_in"].isNull()) {
            // the USB bridge pre-sums the buckets, so no series is transferred
            *calls = doc["total"] | *calls;
            *tokens = (float)((doc["sum_in"] | (uint64_t)0) + (doc["sum_out"] | (uint64_t)0) +
                              (doc["sum_cache"] | (uint64_t)0));
        } else if (doc["series"].is<JsonArray>()) {
            uint64_t tok = 0;
            for (JsonObject b : doc["series"].as<JsonArray>()) {
                tok += b["input"] | (uint64_t)0;
                tok += b["output"] | (uint64_t)0;
                tok += b["cache_read"] | (uint64_t)0;
            }
            *calls = doc["total"] | *calls;
            *tokens = (float)tok;
        }
    }
    d.nrows = 0;
    d.sessions = 0;
    char seen[ROWS_MAX][40];
    for (JsonObject r : doc["rows"].as<JsonArray>()) {
        if (d.nrows >= ROWS_MAX) break;
        const char *sess = r["session"] | "";
        if (*sess) {
            bool dup = false;
            for (int k = 0; k < d.sessions && !dup; k++) dup = !strncmp(seen[k], sess, 39);
            if (!dup && d.sessions < ROWS_MAX) copy_str(seen[d.sessions++], 40, sess);
        }
        RouteRow &row = d.rows[d.nrows];
        // the USB bridge sends a pre-converted local time; fall back to the raw
        // wall clock for the WiFi path (which cannot do timezone math)
        const char *hms = r["hms"] | "";
        if (*hms) copy_str(row.time, sizeof(row.time), hms);
        else local_hms(r["t"] | "", row.time, sizeof(row.time));
        copy_str(row.agent, sizeof(row.agent), r["agent"] | "");
        copy_str(row.agentName, sizeof(row.agentName), r["agentName"] | "");
        copy_str(row.provId, sizeof(row.provId), r["provider"] | "");
        copy_str(row.provider, sizeof(row.provider), r["providerName"] | r["provider"] | "");
        copy_str(row.model, sizeof(row.model), r["req"] | r["model"] | "");
        copy_str(row.stop, sizeof(row.stop), r["stop"] | "");
        char acct[40] = {0};
        mask_account(r["providerAccount"] | r["host"] | "", acct, sizeof(acct));
        copy_str(row.account, sizeof(row.account), acct);
        row.tin = r["in"] | 0;
        row.tout = r["out"] | 0;
        row.tcache = r["cache_read"] | 0;
        row.ms = r["ms"] | 0;
        row.status = r["status"] | 0;
        d.nrows++;
    }
    return true;
}

// Every agent magpie manages, from the PC relay (tools/gw_relay.py). An older
// relay answers 404 and the usage breakdown from fetch_usage stays in place.
static bool fetch_agents(DashData &d) {
    JsonDocument doc;
    if (!get_json("/dash/agents", doc)) return false;
    d.nagents = 0;
    for (JsonObject a : doc["agents"].as<JsonArray>()) {
        if (d.nagents >= AGENT_MAX) break;
        AgentRow &r = d.agents[d.nagents++];
        copy_str(r.id, sizeof(r.id), a["id"] | "");
        copy_str(r.name, sizeof(r.name), a["name"] | "");
        copy_str(r.model, sizeof(r.model), a["model"] | "");
        r.calls = a["calls"] | 0;
        r.tokens = a["tokens"] | 0.0f;
        r.cost = a["cost"] | 0.0f;
        r.today = a["today"] | 0;
        r.last = a["last"] | 0;
    }
    d.agents_full = true;
    return true;
}

// ------------------------------------------------------------------ public
const char *gw_active_link() { return usb_path() ? "USB" : (wifi_path() ? "WiFi" : "-"); }

// Two stages: the head paints the KPI cards, the tail fills the route rows.
// Splitting them is what turns "the numbers take two seconds to change" into
// "numbers change in about half a second, rows follow".
bool gw_fetch_head(DashData &d, bool light) {
    d.wifi = wifi_path();
    if (!usb_path() && !wifi_path()) {
        d.gw = false;
        return false;
    }
    snprintf(d.gw_ip, sizeof(d.gw_ip), "%s", usb_path() ? "USB" : netcfg_host());
    bool ok = light ? d.gw : fetch_quotas(d);  // quotas move slowly: every 3rd cycle
    fetch_usage(d, !light);
    if (!strcmp(s_period, "today")) {
        d.today_calls = d.calls;
        d.today_tokens = (float)d.tok_total();
    } else if (!light) {  // a period switch does not change today's numbers
        uint32_t c = 0;
        float t = 0;
        if (fetch_today(d, &c, &t)) {
            d.today_calls = c;
            d.today_tokens = t;
        }
    }
    d.gw = ok;
    return ok;
}

bool gw_fetch_tail(DashData &d, bool light) {
    uint32_t c = d.today_calls;
    float t = d.today_tokens;
    fetch_requests(d, &c, &t);
    d.today_calls = c;
    d.today_tokens = t;
    if (!light) fetch_agents(d);  // agent list: every 3rd cycle
    return d.gw;
}

bool gw_fetch(DashData &d) {
    d.wifi = wifi_path();
    if (!usb_path() && !wifi_path()) {
        d.gw = false;
        return false;
    }
    snprintf(d.gw_ip, sizeof(d.gw_ip), "%s", usb_path() ? "USB" : netcfg_host());

    bool ok = fetch_quotas(d);
    fetch_usage(d);
    uint32_t today_calls = 0;
    float today_tokens = 0;
    if (strcmp(s_period, "today"))  // already have that document when the filter is 今天
        fetch_today(d, &today_calls, &today_tokens);
    fetch_requests(d, &today_calls, &today_tokens);
    fetch_agents(d);

    // "today" numbers ride in the first two series-independent slots
    if (!strcmp(s_period, "today")) { today_calls = d.calls; today_tokens = (float)d.tok_total(); }
    d.today_calls = today_calls;
    d.today_tokens = today_tokens;

    d.gw = ok;
    Serial.printf("[gw] ok=%d prov=%d calls=%u today=%u tok=%.0f rows=%d agents=%d\n", (int)ok,
                  d.nprov, (unsigned)d.calls, (unsigned)d.today_calls, d.tok_total(), d.nrows,
                  d.nagents);
    return ok;
}

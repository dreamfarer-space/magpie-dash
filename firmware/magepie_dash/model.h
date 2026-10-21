// Dashboard data model, filled by gateway.cpp from the magpie API.
#pragma once
#include <stdint.h>
#include <string.h>

#define QUOTA_MAX_PROV 6
#define QUOTA_MAX_WIN 4
#define SERIES_MAX 31
#define ROWS_MAX 12
#define AGENT_MAX 16

struct QuotaWindow {
    char name[16];      // "5 hours" / "7 days" / "Credits"
    float used;         // percent used 0..100
    float remaining;    // percent left
    char reset[20];     // provider's own label when it has no timestamp ("205.26 / 900")
    char resetIso[26];  // raw resetsAt, formatted when drawn so it follows the UI language
    int64_t reset_at = 0;  // epoch seconds, 0 when unknown
};

struct QuotaProvider {
    char provider[16];  // codex / workbuddy / antigravity
    char name[20];      // Codex / WorkBuddy / Antigravity
    char plan[14];
    char user[42];
    int nwin;
    QuotaWindow win[QUOTA_MAX_WIN];
    char lastServed[20];
};

struct UsagePoint {
    char label[8];
    float tokens;       // input+output+cache for that bucket
    uint32_t calls;
};

struct RouteRow {
    char time[10];      // 00:27:54
    char agent[12];     // hermes
    char agentName[20]; // Hermes Agent
    char provId[16];    // workbuddy (quota provider key)
    char provider[14];
    char account[34];
    char model[34];
    char stop[14];
    uint32_t tin, tout, tcache;
    uint32_t ms;
    uint16_t status;
    bool ok() const { return status == 200; }
    uint32_t tokens() const { return tin + tout + tcache; }
};

struct AgentRow {
    char id[16];        // magpie agent id: claude / codex / hermes / dsh ...
    char name[22];      // Hermes Agent ("" when magpie has not named it yet)
    char model[36];     // model magpie routes it to, "" when unknown
    uint32_t calls;     // last 30 days
    float tokens;
    float cost;         // USD, last 30 days
    uint32_t today;     // calls today
    uint32_t last;      // epoch of its latest call, 0 = none in the recent log
};

struct DashData {
    bool wifi = false;
    bool gw = false;            // last fetch succeeded
    bool fetching = false;
    char gw_ip[20] = "";

    // ---- quotas
    int nprov = 0;
    QuotaProvider prov[QUOTA_MAX_PROV];

    // ---- usage (period selected on the usage page)
    char period[6] = "30d";
    uint32_t calls = 0, errors = 0;
    uint64_t tok_in = 0, tok_out = 0, tok_cache = 0;
    float cost = 0;
    uint32_t unpriced = 0;
    int nseries = 0;
    UsagePoint series[SERIES_MAX];

    // ---- latest requests
    int nrows = 0;
    RouteRow rows[ROWS_MAX];

    // ---- today (independent of the usage page period)
    uint32_t today_calls = 0;
    float today_tokens = 0;

    int sessions = 0;           // distinct sessions among rows[]

    // ---- agents: relay's /dash/agents (every agent magpie manages), or the
    // usage breakdown when the relay is an older one (only agents with calls)
    int nagents = 0;
    bool agents_full = false;
    AgentRow agents[AGENT_MAX];

    // ---- derived helpers ------------------------------------------------
    const QuotaProvider *find(const char *provider) const {
        for (int i = 0; i < nprov; i++)
            if (!strcmp(prov[i].provider, provider)) return &prov[i];
        return nullptr;
    }
    const QuotaWindow *window(const char *provider, int idx) const {
        const QuotaProvider *p = find(provider);
        if (!p || idx >= p->nwin) return nullptr;
        return &p->win[idx];
    }
    float tok_total() const { return (float)(tok_in + tok_out + tok_cache); }
    float hit_rate() const {
        float t = (float)(tok_in + tok_cache);
        return t > 0 ? 100.0f * (float)tok_cache / t : 0;
    }
    // clamp helpers for widgets
    int pct(const char *provider, int idx) const {
        const QuotaWindow *w = window(provider, idx);
        if (!w) return 0;
        int v = (int)(100.0f - w->remaining + 0.5f);
        return v < 0 ? 0 : (v > 100 ? 100 : v);
    }
};

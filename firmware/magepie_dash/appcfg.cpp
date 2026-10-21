#include "appcfg.h"

#include <Preferences.h>

AppCfg g_cfg;

static Preferences s_prefs;

void cfg_defaults() {
    snprintf(g_cfg.handle, sizeof(g_cfg.handle), "Dreamfarer");
    g_cfg.quota_mode = 0;
    g_cfg.refresh_s = 15;
    g_cfg.anim = 1;
    g_cfg.privacy = 0;
    g_cfg.link_mode = 0;  // USB out of the box: no router, no credentials needed
    g_cfg.lang = LANG_EN;  // this firmware ships as the English build; the
                          // settings page switches to Chinese in one tap
}

void cfg_load() {
    cfg_defaults();
    // Read-write open: a read-only open reports NOT_FOUND on the very first boot,
    // which looked like a broken preference store.
    if (!s_prefs.begin("cfg", false)) {
        Serial.println("[cfg] NVS open failed; using defaults");
        return;
    }
    String h = s_prefs.getString("handle", g_cfg.handle);
    snprintf(g_cfg.handle, sizeof(g_cfg.handle), "%s", h.c_str());
    g_cfg.quota_mode = s_prefs.getUChar("quota", g_cfg.quota_mode) ? 1 : 0;
    uint8_t r = s_prefs.getUChar("refresh", g_cfg.refresh_s);
    g_cfg.refresh_s = (r == 15 || r == 30 || r == 60) ? r : 15;
    g_cfg.anim = s_prefs.getUChar("anim", g_cfg.anim) ? 1 : 0;
    g_cfg.privacy = s_prefs.getUChar("privacy", g_cfg.privacy) ? 1 : 0;
    g_cfg.link_mode = s_prefs.getUChar("link", g_cfg.link_mode) ? 1 : 0;
    g_cfg.lang = s_prefs.getUChar("lang", g_cfg.lang) ? LANG_EN : LANG_ZH;
    s_prefs.end();
    lang_set(g_cfg.lang);  // before the UI is built: T() reads it
    if (!g_cfg.handle[0]) snprintf(g_cfg.handle, sizeof(g_cfg.handle), "Dreamfarer");
    Serial.printf("[cfg] handle=@%s quota=%s refresh=%us anim=%u privacy=%u lang=%s\n",
                  g_cfg.handle, g_cfg.quota_mode ? "used" : "left", (unsigned)g_cfg.refresh_s,
                  (unsigned)g_cfg.anim, (unsigned)g_cfg.privacy,
                  g_cfg.lang == LANG_EN ? "en" : "zh");
}

void cfg_save() {
    if (!s_prefs.begin("cfg", false)) {
        Serial.println("[cfg] NVS write open failed");
        return;
    }
    s_prefs.putString("handle", g_cfg.handle);
    s_prefs.putUChar("quota", g_cfg.quota_mode);
    s_prefs.putUChar("refresh", g_cfg.refresh_s);
    s_prefs.putUChar("anim", g_cfg.anim);
    s_prefs.putUChar("privacy", g_cfg.privacy);
    s_prefs.putUChar("link", g_cfg.link_mode);
    s_prefs.putUChar("lang", g_cfg.lang);
    s_prefs.end();
    Serial.printf("[cfg] saved handle=@%s quota=%u refresh=%us anim=%u privacy=%u lang=%u\n",
                  g_cfg.handle, (unsigned)g_cfg.quota_mode, (unsigned)g_cfg.refresh_s,
                  (unsigned)g_cfg.anim, (unsigned)g_cfg.privacy, (unsigned)g_cfg.lang);
}

uint8_t cfg_next_refresh(uint8_t cur) {
    if (cur == 15) return 30;
    if (cur == 30) return 60;
    return 15;
}

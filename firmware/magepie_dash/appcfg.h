// Persistent dashboard preferences, stored in NVS namespace "cfg".
#pragma once
#include <Arduino.h>

struct AppCfg {
    char handle[20];     // shown as @handle in the top-left of the home page
    uint8_t quota_mode;  // 0 = show remaining, 1 = show used
    uint8_t refresh_s;   // fetch interval in seconds: 15 / 30 / 60
    uint8_t anim;        // avatar hop + speech animation
    uint8_t privacy;     // mask money figures on screen
    uint8_t link_mode;   // 0 = USB cable, 1 = WiFi (LAN)
};

extern AppCfg g_cfg;

void cfg_defaults();  // fill g_cfg with the shipped values (no NVS write)
void cfg_load();      // defaults + whatever NVS holds
void cfg_save();      // persist the current g_cfg

// Cycle helpers used by the settings rows.
uint8_t cfg_next_refresh(uint8_t cur);

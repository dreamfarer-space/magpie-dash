// WiFi + gateway address, with a captive setup portal when unconfigured.
#pragma once
#include <Arduino.h>

// Loads saved settings and connects. If nothing is saved (or the join fails)
// the device starts the "magpie-dash-setup" AP with a config page at
// http://192.168.4.1 and netcfg_ready() stays false.
void netcfg_begin();

// Call from loop(): serves the setup portal when active, otherwise polls WiFi.
void netcfg_loop();

bool netcfg_ready();
bool netcfg_portal_active();
const char *netcfg_host();
uint16_t netcfg_port();
const char *netcfg_ssid();

// Re-arm the join right now (settings "重试"); false when USB mode has nothing to retry.
bool netcfg_retry();
// Start the captive setup portal on demand (AP magpie-dash-setup / 192.168.4.1).
void netcfg_open_portal();
// 0 = USB cable, 1 = WiFi; applies immediately (radio off / join or portal).
void netcfg_set_mode(uint8_t mode);

// Forget saved WiFi + gateway settings and reboot into the portal.
void netcfg_reset();

// User-tunable settings for the magpie dashboard firmware.
#pragma once

// Link mode is a runtime setting (settings page / NVS): USB cable or WiFi LAN.
// This flag only exists for the legacy compile-time check; keep it at 1.
#define USB_LINK_ONLY 1

// ---------------------------------------------------------------- WiFi
// Leave empty to boot the setup portal (AP "magpie-dash-setup" -> 192.168.4.1).
// Once saved through the portal, credentials live in NVS and win over these.
// Left empty on purpose: first boot in WiFi mode opens the setup portal so every
// user enters their own network instead of inheriting mine.
#define WIFI_SSID ""
#define WIFI_PASS ""

// ---------------------------------------------------------------- gateway
// Address of the machine running magpie (same LAN as the ESP32).
// Can also be changed at runtime through the setup portal.
// NOTE: magpie only listens on 127.0.0.1, so run tools/gw_relay.py and point
// this at the relay port (3426) instead of 3425.
#define GW_HOST_DEFAULT "192.168.1.100"  // placeholder; the setup page overwrites it
#define GW_PORT_DEFAULT 3426

// How often to pull fresh data from the gateway.
#define REFRESH_MS 15000

// "usage" page period -> the 4 filter buttons (today / 7d / 30d / all)
#define USAGE_PERIODS "today", "7d", "30d", "all"  // magpie has no "1d"

// ---------------------------------------------------------------- clock
#define TZ_STRING "CST-8"
#define NTP_SERVER "ntp.aliyun.com"

// ---------------------------------------------------------------- behaviour
#define STATUS_PAGE_AUTOCYCLE 0  // 1 = demo mode: cycle the three states

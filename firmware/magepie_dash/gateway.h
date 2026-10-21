// Pulls live data from the magpie gateway into DashData.
#pragma once
#include "model.h"

// Fetch quotas + usage + latest requests. Returns true when at least the
// quotas call succeeded. Blocking (LAN latency), call from the main loop.
bool gw_fetch(DashData &d);

// Which pipe is actually carrying data right now ("USB", "WiFi" or "-").
const char *gw_active_link();

// Stage 1 (KPI: quotas + usage) and stage 2 (rows + agents), so the screen can
// show the numbers before the big row document has been parsed.
bool gw_fetch_head(DashData &d, bool light);
bool gw_fetch_tail(DashData &d, bool light);
// True once after the period changed: quota/agent documents are unaffected.
bool gw_take_light();

// Set the usage period used by the next gw_fetch ("1d" / "7d" / "30d" / "all").
void gw_set_period(const char *period);

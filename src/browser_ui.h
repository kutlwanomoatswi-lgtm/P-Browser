#pragma once
#include <cstddef>

struct BrowserState {
    char url_buf[512] = "https://www.google.com";
    bool navigate_requested = false;
    bool adblock_enabled = true;
    int blocked_ads_count = 0;
};

void RenderPBrowserUI(BrowserState& state);

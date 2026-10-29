#pragma once
#include <cstddef>
#include <string>
#include <vector>

struct BrowserState {
    char url_buf[512] = "https://example.com";
    bool navigate_requested = false;
    bool adblock_enabled = true;
    int blocked_ads_count = 0;
    bool is_loading = false;
    std::string page_title = "P-Browser Web Node";
    std::string rendered_text_body = "";
    std::vector<std::string> parsed_links;
};

void RenderPBrowserUI(BrowserState& state);

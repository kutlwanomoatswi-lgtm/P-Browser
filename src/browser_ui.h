#pragma once
#include <cstddef>
#include <string>
#include <vector>

struct BrowserState {
    char url_buf[2048] = "https://example.com";
    bool navigate_requested = false;
    bool url_filter_enabled = true;
    int filtered_count = 0;
    bool is_loading = false;
    std::string page_title = "PSBrowser Web Node";
    std::string rendered_text_body = "";
    std::vector<std::string> parsed_links;
};

void RenderPSBrowserUI(BrowserState& state);

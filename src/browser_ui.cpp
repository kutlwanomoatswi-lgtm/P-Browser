#include "browser_ui.h"
#include <imgui.h>
#include <cstdio>

void RenderPSBrowserUI(BrowserState& state) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("PSBrowser Window", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if (ImGui::Button("Back")) { }
    ImGui::SameLine();
    if (ImGui::Button("Forward")) { }
    ImGui::SameLine();
    if (ImGui::Button("Home")) {
        snprintf(state.url_buf, sizeof(state.url_buf), "https://example.com");
        state.navigate_requested = true;
    }
    ImGui::SameLine();

    ImGui::PushItemWidth(500.0f);
    if (ImGui::InputText("##UrlBar", state.url_buf, sizeof(state.url_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
        state.navigate_requested = true;
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button(state.is_loading ? "Loading..." : "Go")) {
        if (!state.is_loading) state.navigate_requested = true;
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    if (state.url_filter_enabled) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.2f, 1.0f));
        if (ImGui::Button("URL Filter: ON")) {
            state.url_filter_enabled = false;
        }
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("URL Filter: OFF")) {
            state.url_filter_enabled = true;
        }
        ImGui::PopStyleColor();
    }

    ImGui::Separator();
    
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "[PSBrowser Engine] FreeBSD ELF | libcurl async | DualSense Active");
    ImGui::SameLine();
    if (state.url_filter_enabled && state.filtered_count > 0) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "| Filtered Navigations: %d", state.filtered_count);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "| Filter Inactive");
    }

    ImGui::Separator();

    ImGui::BeginChild("WebRenderRegion", ImVec2(0, 0), true);
    
    if (state.is_loading) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Fetching network resource via background thread...");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "%s", state.page_title.c_str());
        ImGui::Separator();
        
        if (state.rendered_text_body.empty()) {
            ImGui::TextDisabled("Enter a URL above and press 'Go' or Enter to load a webpage.");
        } else {
            ImGui::TextWrapped("%s", state.rendered_text_body.c_str());
        }

        if (!state.parsed_links.empty()) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Discovered Page Links:");
            for (size_t i = 0; i < state.parsed_links.size() && i < 10; ++i) {
                if (ImGui::Selectable(state.parsed_links[i].c_str())) {
                    snprintf(state.url_buf, sizeof(state.url_buf), "%s", state.parsed_links[i].c_str());
                    state.navigate_requested = true;
                }
            }
        }
    }

    ImGui::EndChild();
    ImGui::End();
}

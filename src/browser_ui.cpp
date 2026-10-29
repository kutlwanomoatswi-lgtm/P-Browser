#include "browser_ui.h"
#include <imgui.h>
#include <cstdio>

void RenderPBrowserUI(BrowserState& state) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("P-Browser Window", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // Navigation Controls
    if (ImGui::Button("Back")) { }
    ImGui::SameLine();
    if (ImGui::Button("Forward")) { }
    ImGui::SameLine();
    if (ImGui::Button("Home")) {
        snprintf(state.url_buf, sizeof(state.url_buf), "https://www.google.com");
        state.navigate_requested = true;
    }
    ImGui::SameLine();

    // URL Bar
    ImGui::PushItemWidth(550.0f);
    if (ImGui::InputText("##UrlBar", state.url_buf, sizeof(state.url_buf), ImGuiInputTextFlags_EnterReturnsTrue)) {
        state.navigate_requested = true;
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Go")) {
        state.navigate_requested = true;
    }

    // AdBlocker Toggle Widget
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    if (state.adblock_enabled) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.6f, 0.2f, 1.0f));
        if (ImGui::Button("AdBlock: ON")) {
            state.adblock_enabled = false;
        }
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("AdBlock: OFF")) {
            state.adblock_enabled = true;
        }
        ImGui::PopStyleColor();
    }

    ImGui::Separator();
    
    // Status Info Bar
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "[PS5 SDK Mode] Rendering via Software Blit | DualSense Active");
    
    if (state.adblock_enabled) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Shield Active | Filtered Items: %d", state.blocked_ads_count);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Shield Paused | Raw Network Traffic Allowed");
    }

    ImGui::Separator();
    ImGui::TextWrapped("Current URL: %s", state.url_buf);

    ImGui::End();
}

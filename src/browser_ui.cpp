#include "browser_ui.h"
#include <imgui.h>
#include <cstdio>
#include <string>

void RenderPBrowserUI(char* url_buf, size_t buf_size, bool& navigate_requested) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin("P-Browser Window", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if (ImGui::Button("Back")) { }
    ImGui::SameLine();
    if (ImGui::Button("Forward")) { }
    ImGui::SameLine();
    if (ImGui::Button("Home")) {
        snprintf(url_buf, buf_size, "https://www.google.com");
        navigate_requested = true;
    }
    ImGui::SameLine();

    ImGui::PushItemWidth(650.0f);
    if (ImGui::InputText("##UrlBar", url_buf, buf_size, ImGuiInputTextFlags_EnterReturnsTrue)) {
        navigate_requested = true;
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button("Search / Go")) {
        navigate_requested = true;
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Input Modes Active: [Mouse & Keyboard] + [DualSense Controller]");
    ImGui::TextWrapped("Current Web Node: %s", url_buf);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "[HW Acceleration Ready] Supported Sites: Google, YouTube, HTML5 Streaming Media Services");

    ImGui::End();
}

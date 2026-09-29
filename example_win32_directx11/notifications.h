#pragma once

#include <iostream>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include "../imgui_settings.h"
#include <Windows.h>
#include <vector>
#include <string>

#include "src/Globals.hpp"

struct s_notification
{
    std::string text;
    std::string icon;
    ImColor color;
    float alpha;
    float slide_anim; // 0.0f to 1.0f
    float duration;   // Seconds remaining
    float max_duration;
};

static std::vector<s_notification> g_notifications;

class CNotifications
{
public:
    void AddMessage(const char* name, const char* icon, ImColor icon_color)
    {
        s_notification notif;
        notif.text = name;
        notif.icon = icon ? icon : "";
        notif.color = icon_color;
        notif.alpha = 0.0f;
        notif.slide_anim = 0.0f;
        notif.duration = 3.0f;
        notif.max_duration = 3.0f;
        g_notifications.push_back(notif);
    }

    void Render()
    {
        ImGuiIO& io = ImGui::GetIO();
        float delta = io.DeltaTime;
        ImVec2 display_size = io.DisplaySize;

        const float card_height = 46.0f;
        const float card_min_width = 240.0f;
        const float margin_right = 20.0f;
        const float margin_top = 25.0f;
        const float spacing = 10.0f;

        ImDrawList* draw_list = ImGui::GetForegroundDrawList();

        for (size_t i = 0; i < g_notifications.size(); )
        {
            s_notification& n = g_notifications[i];

            // Decrease timer
            n.duration -= delta;

            bool is_exiting = (n.duration <= 0.4f);

            // Animate slide (0.0 = offscreen right, 1.0 = fully visible)
            float target_anim = is_exiting ? 0.0f : 1.0f;
            n.slide_anim = ImLerp(n.slide_anim, target_anim, delta * 12.0f);
            n.alpha = ImClamp(n.slide_anim, 0.0f, 1.0f);

            if (is_exiting && n.slide_anim < 0.02f)
            {
                g_notifications.erase(g_notifications.begin() + i);
                continue;
            }

            // Calculate card dimensions
            ImVec2 text_size = ImGui::CalcTextSize(n.text.c_str());
            ImVec2 icon_size = !n.icon.empty() ? ImGui::CalcTextSize(n.icon.c_str()) : ImVec2(0, 0);
            float card_width = ImMax(card_min_width, text_size.x + icon_size.x + 65.0f);

            // Calculate position (Top-Right stacked downward)
            float target_y = margin_top + i * (card_height + spacing);
            
            // Offscreen X position when hidden
            float offscreen_x = display_size.x + 20.0f;
            float visible_x = display_size.x - card_width - margin_right;
            float current_x = ImLerp(offscreen_x, visible_x, n.slide_anim);

            ImVec2 min_pos = ImVec2(current_x, target_y);
            ImVec2 max_pos = ImVec2(min_pos.x + card_width, min_pos.y + card_height);

            ImU32 bg_col = ImColor(16, 18, 26, (int)(235 * n.alpha));
            ImU32 border_col = ImColor(45, 48, 65, (int)(200 * n.alpha));
            ImU32 accent_col = ImColor(n.color.Value.x, n.color.Value.y, n.color.Value.z, n.alpha);

            // 1. Drop shadow
            draw_list->AddShadowRect(min_pos, max_pos, ImColor(0, 0, 0, (int)(160 * n.alpha)), 25.0f, ImVec2(0, 4), ImDrawFlags_ShadowCutOutShapeBackground, 8.0f);

            // 2. Card background & border
            draw_list->AddRectFilled(min_pos, max_pos, bg_col, 8.0f);
            draw_list->AddRect(min_pos, max_pos, border_col, 8.0f, 0, 1.0f);

            // 3. Left glowing accent pill bar
            draw_list->AddRectFilled(min_pos + ImVec2(3, 6), min_pos + ImVec2(7, card_height - 6), accent_col, 4.0f);

            // 4. Icon Badge
            float content_x = min_pos.x + 18.0f;
            if (!n.icon.empty())
            {
                ImVec2 icon_pos = ImVec2(content_x, min_pos.y + (card_height - icon_size.y) * 0.5f);
                draw_list->AddText(icon_pos, accent_col, n.icon.c_str());
                content_x += icon_size.x + 12.0f;
            }

            // 5. Message Text
            ImVec2 text_pos = ImVec2(content_x, min_pos.y + (card_height - text_size.y) * 0.5f);
            draw_list->AddText(text_pos, ImColor(240, 242, 250, (int)(255 * n.alpha)), n.text.c_str());

            // 6. Remaining duration progress bar line at bottom
            float progress = ImClamp(n.duration / n.max_duration, 0.0f, 1.0f);
            if (progress > 0.0f)
            {
                float bar_w = (card_width - 16.0f) * progress;
                draw_list->AddRectFilled(
                    ImVec2(min_pos.x + 8.0f, max_pos.y - 3.0f),
                    ImVec2(min_pos.x + 8.0f + bar_w, max_pos.y - 1.0f),
                    accent_col, 1.0f
                );
            }

            i++;
        }
    }
};
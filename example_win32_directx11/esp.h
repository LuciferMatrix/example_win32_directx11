#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <array>
#include <string>
#include <algorithm>
#include "imgui.h"
#include "imgui_internal.h"
#include <examples/example_win32_directx11/src/Globals.hpp>

static int esp_scale = 19;

namespace esp_preview {
    inline ImU32 box_color_u32(float alpha) {
        const ImVec4& c = g_Globals.Visuals.BoxColor.Value;
        return IM_COL32((int)(c.x * 255.f), (int)(c.y * 255.f), (int)(c.z * 255.f), (int)(c.w * alpha * 255.f));
    }

    inline ImU32 skeleton_color_u32(float alpha) {
        const ImVec4& c = g_Globals.Visuals.SkeletonColor.Value;
        return IM_COL32((int)(c.x * 255.f), (int)(c.y * 255.f), (int)(c.z * 255.f), (int)(c.w * alpha * 255.f));
    }
}

class c_esp_drag {
public:
    class Box_t {
    public:
        int x, y, w, h;
    };

    struct Position {
        ImVec2 pos;
    };

    class c_drag_item {
    public:
        int pos;
        int type;
        ImColor col;
        std::string text;
        std::string name;
        bool small_text = false;
        ImVec2 pos_;
        ImVec2 size;
        bool hovered = false;
        int helding = 0;
        float move_animation = 0;
        float animations[6];
        bool enabled = true;
        int think_pos;
        bool enable_popup;
        int font;
    };

    std::array<c_drag_item, 8> m_items = {
        c_drag_item{0, 1, ImColor(0, 255, 12), "Health bar", "Health bar"},
        c_drag_item{ 3, 1, ImColor(25, 120, 245), "Weapon Icon", "Weapon Icon"},
        c_drag_item{ 2, 0, ImColor(255,255,255), "Nickname", "Lyapos"},
        c_drag_item{ 3, 0, ImColor(255,255,255), "Distance", "125m", 1},
        c_drag_item{ 1, 0, ImColor(25, 110, 245), "Scoped", "SCOPED", 1},
        c_drag_item{ 1, 0, ImColor(255,120,0), "FD", "FD", 1},
        c_drag_item{ 1, 0, ImColor(255,0,0), "C4", "C4", 1},
        c_drag_item{ 2, 0, ImColor(255,255,255), "Weapon Name", "SCAR-20"} };
    int m_offsets[8];
    Box_t box;
    bool m_layoutLoadedFromGlobals = false;

    int find_closest_position(ImVec2 curr, Position positions[]) {
        float closest = FLT_MAX;
        int best = -1;
        for (int i = 0; i < 4; i++) {
            auto pos = positions[i].pos;
            float dist = pos.dist_to(curr);

            if (closest > dist) {
                closest = dist;
                best = i;
            }
        }

        return best;
    }

    void set_positions() {
        m_items[4].enabled = false;
        m_items[5].enabled = false;
        m_items[6].enabled = false;

        ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
        ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
        float contentW = contentMax.x - contentMin.x;
        float contentH = contentMax.y - contentMin.y;
        box.w = 200;
        box.h = 340;
        box.x = (int)(contentMin.x + (contentW - (float)box.w) * 0.5f);
        box.y = (int)(contentMin.y + (contentH - (float)box.h) * 0.5f);
        box.x = box.x < 0 ? 0 : box.x;
        box.y = box.y < 0 ? 0 : box.y;

        Position Positions[] = {
            {ImVec2(ImGui::GetWindowPos().x + box.x - 5, ImGui::GetWindowPos().y + box.y)},
            {ImVec2(ImGui::GetWindowPos().x + box.x + box.w + 2, ImGui::GetWindowPos().y + box.y)},
            {ImVec2(ImGui::GetWindowPos().x + box.x, ImGui::GetWindowPos().y + box.y - 5)},
            {ImVec2(ImGui::GetWindowPos().x + box.x, ImGui::GetWindowPos().y + box.y + box.h + 2)},
        };

        for (int i = 0; i < (int)m_items.size(); i++) {
            auto& item = m_items[i];
            if (!item.enabled)
                continue;
            ImGui::PushID(i);
            ImGui::SetCursorScreenPos(item.pos_);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.f);
            ImGui::Button(("#esp_drag_" + std::to_string(i)).c_str(), item.size);
            bool hovered = ImGui::IsItemHovered();
            ImGui::PopStyleVar();

            int pos = find_closest_position(ImGui::GetMousePos(), Positions);
            item.hovered = false;
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
                ImGui::SetDragDropPayload("#esp_preview_drag", &i, sizeof(int), 0);
                for (int t = 0; t < (int)m_items.size(); t++)
                    m_items[t].move_animation = ImGui::GetIO().DeltaTime * 34.f;

                item.pos = 4, item.think_pos = pos;
                item.helding = pos > 1, item.hovered = true;
                ImGui::EndDragDropSource();
            }
            else if (item.pos == 4) {
                item.pos = item.think_pos;
                item.think_pos = -1;
                item.move_animation = 0.f;
            }
            item.animations[0] = ImLerp(item.animations[0], item.hovered || hovered ? 1.f : 0.f, ImGui::GetIO().DeltaTime * 34.f);
            ImGui::GetWindowDrawList()->AddRect(item.pos_ - ImVec2(1, 1), item.pos_ + item.size + ImVec2(1, 1), ImColor(255, 255, 255, int(255 * item.animations[0])));
            ImGui::PopID();
        }

        if (!m_layoutLoadedFromGlobals) {
            m_layoutLoadedFromGlobals = true;
            const int h = g_Globals.Visuals.players_healthbar;
            m_items[0].pos = (h == 2) ? 3 : (h == 3) ? 2 : h;
            m_items[2].pos = std::clamp(g_Globals.Visuals.EspNameSide, 0, 3);
            m_items[3].pos = std::clamp(g_Globals.Visuals.EspDistanceSide, 0, 3);
            m_items[1].pos = std::clamp(g_Globals.Visuals.EspWeaponIconSide, 0, 3);
            m_items[7].pos = std::clamp(g_Globals.Visuals.EspWeaponTextSide, 0, 3);
            for (auto& item : m_items)
                item.move_animation = 1.f;
        }

        if (m_items[0].pos >= 0 && m_items[0].pos <= 3) {
            int p = m_items[0].pos;
            g_Globals.Visuals.players_healthbar = (p == 2) ? 3 : (p == 3) ? 2 : p;
            g_Globals.Visuals.HealthBarPosition = (p == 0) ? 3 : (p == 1) ? 2 : (p == 2) ? 0 : 1;
        }
        if (m_items[2].pos >= 0 && m_items[2].pos <= 3)
            g_Globals.Visuals.EspNameSide = m_items[2].pos;
        if (m_items[3].pos >= 0 && m_items[3].pos <= 3)
            g_Globals.Visuals.EspDistanceSide = m_items[3].pos;
        if (m_items[1].pos >= 0 && m_items[1].pos <= 3) {
            g_Globals.Visuals.EspWeaponIconSide = m_items[1].pos;
            g_Globals.Visuals.WeaponInfo = (m_items[1].pos == 0) ? 2 : (m_items[1].pos == 1) ? 3 : (m_items[1].pos == 2) ? 0 : 1;
        }
        if (m_items[7].pos >= 0 && m_items[7].pos <= 3) {
            g_Globals.Visuals.EspWeaponTextSide = m_items[7].pos;
            g_Globals.Visuals.WeaponInfo = (m_items[7].pos == 0) ? 2 : (m_items[7].pos == 1) ? 3 : (m_items[7].pos == 2) ? 0 : 1;
        }
    }

    void on_draw() {
        ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
        ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
        float contentW = contentMax.x - contentMin.x;
        float contentH = contentMax.y - contentMin.y;
        box.w = 200;
        box.h = 340;
        box.x = (int)(contentMin.x + (contentW - (float)box.w) * 0.5f);
        box.y = (int)(contentMin.y + (contentH - (float)box.h) * 0.5f);
        box.x = box.x < 0 ? 0 : box.x;
        box.y = box.y < 0 ? 0 : box.y;

        Position Positions[] = {
            {ImVec2(ImGui::GetWindowPos().x + box.x - 5, ImGui::GetWindowPos().y + box.y)},
            {ImVec2(ImGui::GetWindowPos().x + box.x + box.w + 2, ImGui::GetWindowPos().y + box.y)},
            {ImVec2(ImGui::GetWindowPos().x + box.x, ImGui::GetWindowPos().y + box.y - 5)},
            {ImVec2(ImGui::GetWindowPos().x + box.x, ImGui::GetWindowPos().y + box.y + box.h + 2)},
        };

        ImVec2 Sizes[] = {
            ImVec2(2 + esp_scale - 15, box.h),
            ImVec2(2 + esp_scale - 15, box.h),
            ImVec2(box.w, 2 + esp_scale - 15),
            ImVec2(box.w, 2 + esp_scale - 15)
        };

        {
            float bx = ImGui::GetWindowPos().x + (float)box.x;
            float by = ImGui::GetWindowPos().y + (float)box.y;
            const float alpha = ImGui::GetStyle().Alpha;
            ImU32 boxCol = esp_preview::box_color_u32(alpha);
            ImU32 outlineCol = IM_COL32(0, 0, 0, (int)(255 * alpha));
            float th = 1.0f;

            if (g_Globals.Visuals.players_box == 1) {
                float cw = box.w / 3.0f, ch = box.h / 3.0f;
                auto* dl = ImGui::GetWindowDrawList();
                dl->AddLine(ImVec2(bx, by), ImVec2(bx + cw, by), outlineCol, th + 1.0f);
                dl->AddLine(ImVec2(bx, by), ImVec2(bx, by + ch), outlineCol, th + 1.0f);
                dl->AddLine(ImVec2(bx, by), ImVec2(bx + cw, by), boxCol, th);
                dl->AddLine(ImVec2(bx, by), ImVec2(bx, by + ch), boxCol, th);
                dl->AddLine(ImVec2(bx + box.w, by), ImVec2(bx + box.w - cw, by), outlineCol, th + 1.0f);
                dl->AddLine(ImVec2(bx + box.w, by), ImVec2(bx + box.w, by + ch), outlineCol, th + 1.0f);
                dl->AddLine(ImVec2(bx + box.w, by), ImVec2(bx + box.w - cw, by), boxCol, th);
                dl->AddLine(ImVec2(bx + box.w, by), ImVec2(bx + box.w, by + ch), boxCol, th);
                dl->AddLine(ImVec2(bx, by + box.h), ImVec2(bx + cw, by + box.h), outlineCol, th + 1.0f);
                dl->AddLine(ImVec2(bx, by + box.h), ImVec2(bx, by + box.h - ch), outlineCol, th + 1.0f);
                dl->AddLine(ImVec2(bx, by + box.h), ImVec2(bx + cw, by + box.h), boxCol, th);
                dl->AddLine(ImVec2(bx, by + box.h), ImVec2(bx, by + box.h - ch), boxCol, th);
                dl->AddLine(ImVec2(bx + box.w, by + box.h), ImVec2(bx + box.w - cw, by + box.h), outlineCol, th + 1.0f);
                dl->AddLine(ImVec2(bx + box.w, by + box.h), ImVec2(bx + box.w, by + box.h - ch), outlineCol, th + 1.0f);
                dl->AddLine(ImVec2(bx + box.w, by + box.h), ImVec2(bx + box.w - cw, by + box.h), boxCol, th);
                dl->AddLine(ImVec2(bx + box.w, by + box.h), ImVec2(bx + box.w, by + box.h - ch), boxCol, th);
            }
            else {
                ImGui::GetWindowDrawList()->AddRect(ImVec2(bx, by), ImVec2(bx + box.w, by + box.h), boxCol, 0.0f, 0, th);
                ImGui::GetWindowDrawList()->AddRect(ImVec2(bx - 1, by - 1), ImVec2(bx + box.w + 1, by + box.h + 1), outlineCol);
                ImGui::GetWindowDrawList()->AddRect(ImVec2(bx + 1, by + 1), ImVec2(bx + box.w - 1, by + box.h - 1), outlineCol);
            }
        }

        if (g_Globals.Visuals.Skeleton) {
            float bx = ImGui::GetWindowPos().x + (float)box.x;
            float by = ImGui::GetWindowPos().y + (float)box.y;
            float bw = (float)box.w, bh = (float)box.h;
            ImVec2 head(bx + bw * 0.5f, by + bh * 0.06f);
            ImVec2 neck(bx + bw * 0.5f, by + bh * 0.16f);
            ImVec2 l_sh(bx + bw * 0.20f, by + bh * 0.20f);
            ImVec2 r_sh(bx + bw * 0.80f, by + bh * 0.20f);
            ImVec2 l_el(bx + bw * 0.10f, by + bh * 0.34f);
            ImVec2 r_el(bx + bw * 0.90f, by + bh * 0.34f);
            ImVec2 l_wr(bx + bw * 0.04f, by + bh * 0.48f);
            ImVec2 r_wr(bx + bw * 0.96f, by + bh * 0.48f);
            ImVec2 l_hand(bx + bw * 0.01f, by + bh * 0.50f);
            ImVec2 r_hand(bx + bw * 0.99f, by + bh * 0.50f);
            ImVec2 hip(bx + bw * 0.5f, by + bh * 0.54f);
            ImVec2 l_ank(bx + bw * 0.38f, by + bh * 0.82f);
            ImVec2 r_ank(bx + bw * 0.62f, by + bh * 0.82f);
            ImVec2 l_foot(bx + bw * 0.34f, by + bh * 0.97f);
            ImVec2 r_foot(bx + bw * 0.66f, by + bh * 0.97f);

            const float alpha = ImGui::GetStyle().Alpha;
            ImU32 skCol = esp_preview::skeleton_color_u32(alpha);
            auto* dl = ImGui::GetWindowDrawList();

            const float glowRadius = 14.0f;
            const float feather = 2.0f;
            auto DrawBoneGlow = [&](const ImVec2& from, const ImVec2& to) {
                for (float i = glowRadius; i > 0; i -= feather) {
                    int a = (int)(255 * alpha * (i / glowRadius) * 0.12f);
                    dl->AddLine(from, to, IM_COL32(255, 255, 255, a), 1.5f + i * 0.5f);
                }
                dl->AddLine(from, to, skCol, 1.5f);
            };

            for (float i = glowRadius; i > 0; i -= feather) {
                int a = (int)(255 * alpha * (i / glowRadius) * 0.12f);
                dl->AddCircle(head, 4.0f + i * 0.5f, IM_COL32(255, 255, 255, a), 0, 1.0f + i * 0.5f);
            }
            dl->AddCircle(head, 4.0f, skCol, 0, 1.0f);

            DrawBoneGlow(head, neck);
            DrawBoneGlow(neck, l_sh);
            DrawBoneGlow(neck, r_sh);
            DrawBoneGlow(l_sh, l_el);
            DrawBoneGlow(r_sh, r_el);
            DrawBoneGlow(l_el, l_wr);
            DrawBoneGlow(r_el, r_wr);
            DrawBoneGlow(l_wr, l_hand);
            DrawBoneGlow(r_wr, r_hand);
            DrawBoneGlow(neck, hip);
            DrawBoneGlow(hip, l_ank);
            DrawBoneGlow(hip, r_ank);
            DrawBoneGlow(l_ank, l_foot);
            DrawBoneGlow(r_ank, r_foot);
        }

        float offsetY = 0.f;

        for (auto& item : m_items) {
            item.animations[2] = ImLerp(item.animations[2], item.enabled ? 1.f : 0.f, ImGui::GetIO().DeltaTime * 34.f);
            if (item.animations[2] < 0.1f) {
                for (int i = 0; i < (int)m_items.size(); i++)
                    m_items[i].move_animation = ImGui::GetIO().DeltaTime * 34.f;
                continue;
            }

            item.move_animation += ImGui::GetIO().DeltaTime * 34.f;
            item.move_animation = ImClamp(item.move_animation, 0.f, 1.f);

            if (item.hovered) {
                auto size = ImGui::CalcTextSize(item.text.c_str());
                static bool s = true;
                if (s) {
                    item.size = size;
                    s = false;
                }
                if (item.small_text)
                    size.y = 12;

                switch (item.think_pos) {
                case 0:
                    item.type == 0 ? m_offsets[4] += 2.f + size.y + offsetY : m_offsets[0] += 5.f;
                    break;
                case 1:
                    item.type == 0 ? m_offsets[5] += 2.f + size.y + offsetY : m_offsets[1] += 5.f;
                    break;
                case 2:
                    item.type == 0 ? m_offsets[6] += 2.f + size.y + offsetY : m_offsets[2] += 5.f;
                    break;
                case 3:
                    item.type == 0 ? m_offsets[7] += 2.f + size.y + offsetY : m_offsets[3] += 5.f;
                    break;
                }

                offsetY += size.y;
            }

            if (item.type == 0) {
                auto size = ImGui::CalcTextSize(item.text.c_str());
                item.size = size;
                if (item.small_text)
                    size.y = 12;

                switch (item.pos) {
                case 0:
                    item.pos_ = ImLerp(item.pos_, Positions[0].pos + ImVec2(-m_offsets[0] - size.x, m_offsets[4]), item.move_animation);
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_, item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    m_offsets[4] += 2.f + size.y;
                    break;
                case 1:
                    item.pos_ = ImLerp(item.pos_, Positions[1].pos + ImVec2(m_offsets[1] + esp_scale - 15, m_offsets[5]), item.move_animation);
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_, item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    m_offsets[5] += size.y;
                    break;
                case 2:
                    item.pos_ = ImLerp(item.pos_, Positions[2].pos + ImVec2(Sizes[2].x / 2.f - size.x / 2.f, -m_offsets[2] - size.y - m_offsets[6]), item.move_animation);
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_, item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    m_offsets[6] += 2.f + size.y;
                    break;
                case 3:
                    item.pos_ = ImLerp(item.pos_, Positions[3].pos + ImVec2(Sizes[2].x / 2.f - size.x / 2.f, m_offsets[3] + m_offsets[7]), item.move_animation);
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_, item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    m_offsets[7] += 2.f + size.y;
                    break;
                case 4:
                    item.pos_ = ImLerp(item.pos_, ImGui::GetMousePos() + ImVec2(-size.x / 2.f, 0), ImGui::GetIO().DeltaTime * 14.f);
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ - ImVec2(0, 1), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_ + ImVec2(1, 0), ImColor(0, 0, 0).SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    ImGui::GetWindowDrawList()->AddText(item.pos_, item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha), item.text.c_str());
                    break;
                }

                continue;
            }
            item.size = Sizes[item.pos];
            switch (item.pos) {
            case 0:
                item.pos_ = ImLerp(item.pos_, Positions[0].pos + ImVec2(-m_offsets[0] + 15 - esp_scale, 0.f), item.move_animation);
                ImGui::GetWindowDrawList()->AddRectFilled(item.pos_, item.pos_ + Sizes[0], item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha));
                m_offsets[0] += 5.f + esp_scale - 15;
                break;
            case 1:
                item.pos_ = ImLerp(item.pos_, Positions[1].pos + ImVec2(m_offsets[1] + esp_scale - 15, 0.f), item.move_animation);
                ImGui::GetWindowDrawList()->AddRectFilled(item.pos_, item.pos_ + Sizes[1], item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha));
                m_offsets[1] += 5.f + esp_scale - 15;
                break;
            case 2:
                item.pos_ = ImLerp(item.pos_, Positions[2].pos + ImVec2(0.f, -m_offsets[2] + 15 - esp_scale), item.move_animation);
                ImGui::GetWindowDrawList()->AddRectFilled(item.pos_, item.pos_ + Sizes[2], item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha));
                m_offsets[2] += 5.f + esp_scale - 15;
                break;
            case 3:
                item.pos_ = ImLerp(item.pos_, Positions[3].pos + ImVec2(0.f, m_offsets[3] + esp_scale - 15), item.move_animation);
                ImGui::GetWindowDrawList()->AddRectFilled(item.pos_, item.pos_ + Sizes[3], item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha));
                m_offsets[3] += 5.f + esp_scale - 15;
                break;
            case 4:
                item.pos_ = ImLerp(item.pos_, ImGui::GetMousePos() + ImVec2(0.f, m_offsets[3]), ImGui::GetIO().DeltaTime * 34.f);
                if (item.helding == 1) {
                    item.size = Sizes[3];
                    ImGui::GetWindowDrawList()->AddRectFilled(item.pos_, item.pos_ + Sizes[3], item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha));
                }
                else if (item.helding == 0) {
                    item.size = Sizes[1];
                    ImGui::GetWindowDrawList()->AddRectFilled(item.pos_, item.pos_ + Sizes[1], item.col.SetAlpha(item.animations[2] * ImGui::GetStyle().Alpha));
                }
                break;
            }
        }

        for (int i = 0; i < 8; i++)
            m_offsets[i] = 0.f;
    }
} m_esp_draw;

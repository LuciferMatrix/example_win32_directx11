#include "custom_widgets.hpp"
#pragma comment(lib, "Winmm.lib")
#include "crr.hpp"
#include "blur.hpp"
#include "stt1.hpp"
#include <cmath>

#include "Sounds.hpp"
#include "font_defines.h"

namespace custom
{



	const char* keys[] =
	{
		"-",
		"Mouse 1",
		"Mouse 2",
		"CN",
		"Mouse 3",
		"Mouse 4",
		"Mouse 5",
		"-",
		"Back",
		"Tab",
		"-",
		"-",
		"CLR",
		"Enter",
		"-",
		"-",
		"Shift",
		"CTL",
		"Menu",
		"Pause",
		"Caps Lock",
		"KAN",
		"-",
		"JUN",
		"FIN",
		"KAN",
		"-",
		"Escape",
		"CON",
		"NCO",
		"ACC",
		"MAD",
		"Space",
		"PGU",
		"PGD",
		"End",
		"Home",
		"Left",
		"Up",
		"Right",
		"Down",
		"SEL",
		"PRI",
		"EXE",
		"PRI",
		"INS",
		"Delete",
		"HEL",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"WIN",
		"WIN",
		"APP",
		"-",
		"SLE",
		"Numpad 0",
		"Numpad 1",
		"Numpad 2",
		"Numpad 3",
		"Numpad 4",
		"Numpad 5",
		"Numpad 6",
		"Numpad 7",
		"Numpad 8",
		"Numpad 9",
		"MUL",
		"ADD",
		"SEP",
		"MIN",
		"Delete",
		"DIV",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"F13",
		"F14",
		"F15",
		"F16",
		"F17",
		"F18",
		"F19",
		"F20",
		"F21",
		"F22",
		"F23",
		"F24",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"NUM",
		"SCR",
		"EQU",
		"MAS",
		"TOY",
		"OYA",
		"OYA",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"-",
		"Shift",
		"Shift",
		"Ctrl",
		"Ctrl",
		"Alt",
		"Alt"
	};

	static const int KEYS_TABLE_LEN = (int)(sizeof(keys) / sizeof(keys[0]));

	const char* KeyName(int vk)
	{
		static char s_keyNameUtf8[64];
		if (vk <= 0)
			return "?";
		// Values in ImGuiKey range (e.g. 512+) can end up stored if config was corrupted or from an older binding path.
		if (vk >= (int)ImGuiKey_NamedKey_BEGIN && vk < (int)ImGuiKey_COUNT)
		{
			const int mapped = ImGui::GetIO().KeyMap[vk];
			if (mapped > 0 && mapped <= 255)
				vk = mapped;
			else
				return "?";
		}
		if (vk < KEYS_TABLE_LEN)
			return keys[vk];
		if (vk > 255)
			return "?";
		const UINT scan = MapVirtualKeyW((UINT)vk, MAPVK_VK_TO_VSC);
		if (scan == 0)
			return "?";
		LONG lp = (LONG)(scan << 16);
		WCHAR wname[64];
		if (GetKeyNameTextW(lp, wname, 64) <= 0)
			return "?";
		if (WideCharToMultiByte(CP_UTF8, 0, wname, -1, s_keyNameUtf8, (int)sizeof(s_keyNameUtf8), nullptr, nullptr) <= 0)
			return "?";
		return s_keyNameUtf8;
	}

#include <string>

	struct item_bg_state
	{
		ImVec4 background, stroke;
		ImVec2 mouse_pos;
	};

	struct description_state
	{
		ImVec4 hint_text_col;
		bool is_hinted;
	};

	void ItemBackground(ImGuiID id, ImRect bb, bool hovered, ImDrawList* draw_list)
	{
		static std::map<ImGuiID, item_bg_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, item_bg_state() });
			it_anim = anim.find(id);
		}

		it_anim->second.background = ImLerp(it_anim->second.background, hovered ? ImColor(1.f, 1.f, 1.f, 0.2f) : ImColor(1.f, 1.f, 1.f, 0.15f), c::anim::speed);
		it_anim->second.stroke = ImLerp(it_anim->second.stroke, stroke_color, c::anim::speed);
		it_anim->second.mouse_pos = ImLerp(it_anim->second.mouse_pos, ImClamp(ImGui::GetMousePos(), ImVec2(0, 0), ImVec2(5000, 5000)), c::anim::speed * 3);



		//crr::Push(draw_list, bb.Min, bb.Max, c::elements::rounding);
		////img_blur::Before(ImGui::GetWindowDrawList(), bb.Min, bb.Max, 10.f, 1.f, 0, false);
		//
		////ImGui::GetWindowDrawList()->AddShadowCircle(it_anim->second.mouse_pos, 30.f, ImColor(1.f, 1.f, 1.f, 1.f), 1900.f, ImVec2(0, 0), 0, 360);
		//
		//	
		//crr::Pop(draw_list);


		const int vtx_idx_0 = draw_list->VtxBuffer.Size;
		draw_list->AddRectFilled(bb.Min, bb.Max, ImColor(1.f, 1.f, 1.f, 1.f), c::elements::rounding);
		const int vtx_idx_1 = draw_list->VtxBuffer.Size;
		ShadeVertsVerticalGradient(draw_list, vtx_idx_0, vtx_idx_1, bb.Min.y, bb.Max.y, ImColor(1.f, 1.f, 1.f, 0.05f), ImColor(1.f, 1.f, 1.f, 0.01f));


		const int vtx_idx_2 = draw_list->VtxBuffer.Size;
		draw_list->AddRect(bb.Min, bb.Max, ImColor(1.f, 1.f, 1.f, 1.f), c::elements::rounding);
		const int vtx_idx_3 = draw_list->VtxBuffer.Size;
		ShadeVertsVerticalGradient(draw_list, vtx_idx_2, vtx_idx_3, bb.Min.y - 1, bb.Max.y + 1, ImColor(1.f, 1.f, 1.f, 0.01f), ImColor(1.f, 1.f, 1.f, 0.05f));



	}

	void ItemDescription(const char* text, ImGuiID id, ImRect bb, bool hovered, ImVec2 pos, ImVec4 description_col, ImDrawList* draw_list = ImGui::GetWindowDrawList(), ImFont* font = nullptr, float font_size = 0.f)
	{
		static std::map<ImGuiID, description_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, description_state() });
			it_anim = anim.find(id);
		}


		if (hovered) {
			if (!it_anim->second.is_hinted)
			{
				static DWORD dwTickStart = GetTickCount();
				if (GetTickCount() - dwTickStart > 800)
				{
					it_anim->second.is_hinted = true;
					dwTickStart = GetTickCount();
				}
			}
		}
		else
			it_anim->second.is_hinted = false;


		it_anim->second.hint_text_col = ImLerp(it_anim->second.hint_text_col, it_anim->second.is_hinted ? utils::GetColorWithAlpha(label::regular, ImGui::GetStyle().Alpha) : utils::GetColorWithAlpha(label::regular, 0.f), GetAnimSpeed() / 3);

		draw_list->PushClipRect(pos, bb.Max, true);



		ImRect descrtiption_bb(pos, pos + CalcTextSize(text));

		// Render SV Square
		const int vtx_idx_0 = draw_list->VtxBuffer.Size;

		draw_list->AddText(font, font_size, descrtiption_bb.Min, GetColorU32(description_col), text);

		const int vtx_idx_1 = draw_list->VtxBuffer.Size;
		ShadeVertsLinearColorGradientSetAlpha(draw_list, vtx_idx_0, vtx_idx_1, bb.Min, bb.Max, GetColorU32(description_col), utils::GetColorWithAlpha(description_col, 0.f));

		it_anim->second.hint_text_col = ImLerp(it_anim->second.hint_text_col, it_anim->second.is_hinted ? utils::GetColorWithAlpha(label::regular, ImGui::GetStyle().Alpha) : utils::GetColorWithAlpha(label::regular, 0.f), GetAnimSpeed());


		draw_list->PopClipRect();




	}


	enum KeybindStatus {
		KEYBIND_NONE = 0,
		KEYBIND_WAITING,
		KEYBIND_ASSIGNED
	};

	struct key_state {
		ImVec4 background, text;
		ImVec4 text_color, description_col;
		float alpha = 0.f;
		float size_x = 0.f;
		int status = KEYBIND_NONE;
		const char* key_name;
		bool clicked_to_waiting = false; // Флаг для предотвращения моментального захвата Mouse1
		// After entering bind mode, wait until all mouse buttons are released before accepting input (so Mouse 1 / LMB can be chosen).
		bool bind_input_armed = false;

		ImVec2 frame_offset;
		ImDrawList* draw_list;
		float size_scale = 1.f;
		float blur_thinkess;
		float highlight_offset = -72.f;
		float highlight_alpha;
		float bg_alpha = 0.4f;
	};

	bool Keybind(const char* label, const char* description, int* key, int* mode, bool allow_mouse)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		ImGuiIO& io = g.IO;
		const ImGuiStyle& style = g.Style;

		const ImGuiID id = window->GetID(label);
		const float width = GetContentRegionAvail().x;

		static std::map<ImGuiID, key_state> anim;
		auto it_anim = anim.emplace(id, key_state()).first;

		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(width, 55));
		const ImRect total_bb(window->DC.CursorPos - it_anim->second.frame_offset, window->DC.CursorPos + ImVec2(width, 55) + it_anim->second.frame_offset);

		ItemSize(frame_bb, 0.f);

		if (!ItemAdd(frame_bb, id)) return false;

		// Require keybind to be fully visible before allowing hover/zoom
		// Check visibility on base frame_bb (not zoomed total_bb) to avoid glitches during zoom
		ImRect clipped_bb = frame_bb;
		clipped_bb.ClipWith(window->ClipRect);

		bool hovered = false;
		bool held = false;
		bool pressed = false;

		// Check if the entire base widget is visible (not cut off)
		const bool fully_visible = !clipped_bb.IsInverted() &&
			clipped_bb.Min.x <= frame_bb.Min.x + 1.0f &&
			clipped_bb.Min.y <= frame_bb.Min.y + 1.0f &&
			clipped_bb.Max.x >= frame_bb.Max.x - 1.0f &&
			clipped_bb.Max.y >= frame_bb.Max.y - 1.0f;

		if (fully_visible)
			pressed = ButtonBehavior(total_bb, id, &hovered, &held);

		it_anim->second.alpha = ImLerp(it_anim->second.alpha,
			hovered ? 1.f : 0.8f, io.DeltaTime * 5.f);
		it_anim->second.background = ImLerp(it_anim->second.background,
			hovered ? c::anim::regular : c::anim::regular, io.DeltaTime * 5.f);
		it_anim->second.text = ImLerp(it_anim->second.text, hovered ? c::text::label::hovered : c::text::label::regular, io.DeltaTime * 5.f);

		it_anim->second.text_color = ImLerp(it_anim->second.text_color, it_anim->second.status != KEYBIND_NONE ? c::text::label::active : hovered ? c::text::label::hovered : c::text::label::regular, GetAnimSpeed());
		it_anim->second.description_col = ImLerp(it_anim->second.description_col, it_anim->second.status != KEYBIND_NONE ? c::text::description::active : hovered ? c::text::description::hovered : c::text::description::regular, GetAnimSpeed());

		it_anim->second.draw_list = it_anim->second.size_scale > 1.01f ? GetForegroundDrawList() : GetWindowDrawList();
		it_anim->second.size_scale = ImLerp(it_anim->second.size_scale, hovered ? 1.05f : 1.f, GetAnimSpeed());
		it_anim->second.frame_offset = ImLerp(it_anim->second.frame_offset, hovered ? ImVec2(50 * it_anim->second.size_scale, 12.5f * it_anim->second.size_scale) : ImVec2(0, 0), GetAnimSpeed());
		it_anim->second.blur_thinkess = ImLerp(it_anim->second.blur_thinkess, hovered ? 1.f : 0.f, GetAnimSpeed() * 2);
		it_anim->second.highlight_offset = ImLerp(it_anim->second.highlight_offset, hovered ? total_bb.GetSize().x + 52.f : -112.f, GetAnimSpeed() / 3);
		it_anim->second.highlight_alpha = 0.8f;

		// Background Opacity Animation
		it_anim->second.bg_alpha = ImLerp(it_anim->second.bg_alpha, hovered ? 1.f : 0.4f, GetAnimSpeed());

		if (!hovered)
			it_anim->second.highlight_offset = -112.f;

		// Draw Widget Background
		it_anim->second.draw_list->AddRectFilled(total_bb.Min, total_bb.Max, utils::GetColorWithAlpha(c::window_bg_color, it_anim->second.bg_alpha), c::elements::rounding);

		// Shine effect (blur/shadow/highlight) - enabled with zoom
		it_anim->second.draw_list->AddShadowRect(total_bb.Min, total_bb.Max, ImColor(0.f, 0.f, 0.f, it_anim->second.blur_thinkess), 70.f, ImVec2(0, 0), ImDrawFlags_ShadowCutOutShapeBackground, c::elements::rounding);
		crr::Push(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding);
		it_anim->second.draw_list->AddImage(c::highlight_image, total_bb.Min + ImVec2(it_anim->second.highlight_offset - 32 * it_anim->second.size_scale, -32 * it_anim->second.size_scale), total_bb.Min + ImVec2(it_anim->second.highlight_offset + total_bb.GetSize().y + 32 * it_anim->second.size_scale, total_bb.GetSize().y + 32 * it_anim->second.size_scale), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, it_anim->second.highlight_alpha));
		crr::Pop(it_anim->second.draw_list);

		img_blur::Before(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it_anim->second.blur_thinkess, 0, false);

		ItemBackground(id, total_bb, hovered, it_anim->second.draw_list);


		// 🎯 Прямоугольник для клавиши (по центру по Y, фиксированный по высоте 23px)
		ImVec2 key_bb_pos = ImVec2(total_bb.Max.x - it_anim->second.size_x - 16.f, total_bb.GetCenter().y - 11.5f);
		ImVec2 key_bb_size = ImVec2(total_bb.Max.x - 10.f, total_bb.GetCenter().y + 11.5f);

		ImRect key_bb(key_bb_pos, key_bb_size);

		// 🎯 Заголовок
		ImVec2 text_pos = ImVec2(total_bb.Min.x + 10.f, total_bb.Min.y + 10.f);
		it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * it_anim->second.size_scale, ImVec2(total_bb.Min.x + 14.f * it_anim->second.size_scale, total_bb.GetCenter().y - CalcTextSize(label).y), GetColorU32(it_anim->second.text_color), label);

		ImRect descrtiption_bb(ImVec2(total_bb.Min.x + 14.f * it_anim->second.size_scale, total_bb.GetCenter().y), ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y) + CalcTextSize(description));
		ItemDescription(description, id, ImRect{ ImVec2(key_bb.Min.x - 25, descrtiption_bb.Min.y), ImVec2(key_bb.Min.x - 5, descrtiption_bb.Max.y) }, hovered, ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y), it_anim->second.description_col, it_anim->second.draw_list, font::description_font, font::description_font->FontSize * it_anim->second.size_scale);



		// 📝 Установка текста статуса
		if (it_anim->second.status == KEYBIND_WAITING) {
			it_anim->second.key_name = allow_mouse ? "Key or mouse..." : "...";
		}
		else if (*key != 0) {
			it_anim->second.key_name = KeyName(*key);
		}
		else {
			it_anim->second.key_name = "None";
		}

		// 📏 Анимация ширины
		ImVec2 key_text_size = CalcTextSize(it_anim->second.key_name);
		float target_size_x = ImMax(40.f, key_text_size.x + 16.f);
		it_anim->second.size_x = ImLerp(it_anim->second.size_x, target_size_x, io.DeltaTime * 10.f);

		// 🎨 Рендер прямоугольника и текста клавиши
		it_anim->second.draw_list->AddRect(key_bb.Min, key_bb.Max, c::stroke_color, style.FrameRounding);
		it_anim->second.draw_list->AddText(key_bb.GetCenter() - key_text_size * 0.5f, ImColor(1.f, 1.f, 1.f, it_anim->second.alpha), it_anim->second.key_name);

		// 🎮 Логика статусов
		if (pressed) {
			if (it_anim->second.status != KEYBIND_WAITING && !it_anim->second.clicked_to_waiting) {
				it_anim->second.status = KEYBIND_WAITING;
				it_anim->second.clicked_to_waiting = true;
				it_anim->second.bind_input_armed = false;
				*key = 0;
			}
		}

		if (!io.MouseDown[0]) {
			it_anim->second.clicked_to_waiting = false;
		}

		bool value_changed = false;

		if (it_anim->second.status == KEYBIND_WAITING) {
			const bool any_mouse_down =
				io.MouseDown[0] || io.MouseDown[1] || io.MouseDown[2] || io.MouseDown[3] || io.MouseDown[4];
			if (!any_mouse_down)
				it_anim->second.bind_input_armed = true;

			if (IsKeyPressedMap(ImGuiKey_Escape)) {
				*key = 0;
				it_anim->second.status = KEYBIND_NONE;
				it_anim->second.bind_input_armed = false;
			}
			else if (it_anim->second.bind_input_armed) {
				if (allow_mouse) {
					for (int i = 0; i < 5; i++) {
						if (io.MouseClicked[i]) {
							switch (i) {
							case 0: *key = 0x01; break; // VK_LBUTTON
							case 1: *key = 0x02; break; // VK_RBUTTON
							case 2: *key = 0x04; break; // VK_MBUTTON
							case 3: *key = 0x05; break; // VK_XBUTTON1
							case 4: *key = 0x06; break; // VK_XBUTTON2
							}
							it_anim->second.status = KEYBIND_ASSIGNED;
							it_anim->second.bind_input_armed = false;
							value_changed = true;
						}
					}
				}
				if (!value_changed) {
					// Use Win32 VK codes (matches GetAsyncKeyState checks in the app). io.KeysDown[] indices are not reliable with ImGui 1.87+ AddKeyEvent backends.
					for (int vk = 0x08; vk <= 0xFF; vk++) {
						if (GetAsyncKeyState(vk) & 0x8000) {
							*key = vk;
							it_anim->second.status = KEYBIND_ASSIGNED;
							it_anim->second.bind_input_armed = false;
							value_changed = true;
						}
					}
				}
			}
		}

		if (*key != 0 && it_anim->second.status != KEYBIND_WAITING) {
			it_anim->second.status = KEYBIND_ASSIGNED;
		}
		else if (*key == 0 && it_anim->second.status != KEYBIND_WAITING) {
			it_anim->second.status = KEYBIND_NONE;
		}

		return value_changed;
	}


	bool MiniBind(const char* label, int* key, int* mode, bool allow_mouse)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		ImGuiIO& io = g.IO;
		const ImGuiStyle& style = g.Style;

		const ImGuiID id = window->GetID(label);
		const float width = (GetContentRegionMax().x - style.WindowPadding.x) - 40;

		static std::map<ImGuiID, key_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, key_state() });
			it_anim = anim.find(id);
		}

		const ImRect rect(window->DC.CursorPos + ImVec2(width - 20 - it_anim->second.size_x, 0), window->DC.CursorPos + ImVec2(width, 19));

		ItemSize(ImRect(rect.Min, rect.Max));
		if (!ImGui::ItemAdd(rect, id)) return false;

		char buf_display[64] = "None";


		bool value_changed = false;
		int k = *key;

		std::string active_key = (*key != 0) ? KeyName(*key) : "None";

		if (*key != 0 && g.ActiveId != id) {
			strcpy_s(buf_display, active_key.c_str());
		}
		else if (g.ActiveId == id) {
			strcpy_s(buf_display, "...");
		}

		const ImVec2 label_size = CalcTextSize(buf_display, NULL, true);

		ImRect clickable(rect.Min, rect.Max);
		bool hovered, held, pressed = ButtonBehavior(rect, id, &hovered, &held);


		it_anim->second.background = ImLerp(it_anim->second.background, g.ActiveId == id ? c::anim::active : c::anim::regular, GetAnimSpeed());
		//it_anim->second.icon = ImLerp(it_anim->second.icon, g.ActiveId == id ? c::anim::active : hovered ? c::text::label::hovered : c::text::label::regular, GetAnimSpeed());
		it_anim->second.size_x = ImLerp(it_anim->second.size_x, CalcTextSize(buf_display).x, GetAnimSpeed());

		window->DrawList->AddRectFilled(clickable.Min, clickable.Max, GetColorU32(it_anim->second.background), c::elements::rounding);

		if (pressed)
		{
			if (g.ActiveId != id) {

				memset(io.MouseDown, 0, sizeof(io.MouseDown));
				memset(io.KeysDown, 0, sizeof(io.KeysDown));
				*key = 0;
				k = 0;
				it_anim->second.bind_input_armed = false;
			}
			ImGui::SetActiveID(id, window);
			ImGui::FocusWindow(window);
		}
		else if (io.MouseClicked[0]) {

			if (g.ActiveId == id)
				ImGui::ClearActiveID();
		}

		if (g.ActiveId == id) {
			const bool any_mouse_down =
				io.MouseDown[0] || io.MouseDown[1] || io.MouseDown[2] || io.MouseDown[3] || io.MouseDown[4];
			if (!any_mouse_down)
				it_anim->second.bind_input_armed = true;

			if (IsKeyPressedMap(ImGuiKey_Escape)) {
				*key = 0;
				k = 0;
				it_anim->second.bind_input_armed = false;
				ImGui::ClearActiveID();
			}
			else if (it_anim->second.bind_input_armed) {
				if (allow_mouse) {
					for (auto i = 0; i < 5; i++) {
						if (io.MouseClicked[i]) {
							switch (i) {
							case 0:
								k = 0x01;
								break;
							case 1:
								k = 0x02;
								break;
							case 2:
								k = 0x04;
								break;
							case 3:
								k = 0x05;
								break;
							case 4:
								k = 0x06;
								break;
							}
							value_changed = true;
							it_anim->second.bind_input_armed = false;
							ImGui::ClearActiveID();
						}
					}
				}
				if (!value_changed) {
					for (int vk = 0x08; vk <= 0xFF; vk++) {
						if (GetAsyncKeyState(vk) & 0x8000) {
							k = vk;
							value_changed = true;
							it_anim->second.bind_input_armed = false;
							ImGui::ClearActiveID();
						}
					}
				}
				*key = k;
			}
			else {
				*key = k;
			}
		}

		return value_changed;
	}

	bool ChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool cap, ImGuiWindowFlags flags)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* parent_window = g.CurrentWindow;

		flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;
		flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);

		/*if (parent_window->DC.IsSameLine)
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 30);*/

		const ImVec2 content_avail = GetContentRegionAvail();
		ImVec2 size = ImFloor(size_arg) + ImVec2(0, cap);
		const int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
		if (size.x <= 0.0f)
			size.x = ImMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too many issues)
		if (size.y <= 0.0f)
			size.y = ImMax(content_avail.y + size.y, 4.0f);



		SetNextWindowSize(size - ImVec2(0, 0));


		// img_blur::Before(GetBackgroundDrawList(), parent_window->DC.CursorPos, parent_window->DC.CursorPos + size, c::elements::rounding, ImGui::GetWindowPos().y - c::main_window_rect.Min.y < 70.f ? 1.f : 0.f, 0, false);

		GetBackgroundDrawList()->AddRectFilled(parent_window->DC.CursorPos, parent_window->DC.CursorPos + size, GetColorU32(c::child::background), c::child::rounding, ImDrawFlags_RoundCornersAll);
		GetBackgroundDrawList()->AddRect(parent_window->DC.CursorPos, parent_window->DC.CursorPos + size, c::stroke_color, c::child::rounding, ImDrawFlags_RoundCornersAll);



		//GetWindowDrawList()->AddRect(parent_window->DC.CursorPos, parent_window->DC.CursorPos + size, GetColorU32(c::child::stroke), c::child::rounding, ImDrawFlags_RoundCornersAll, 1);

		const char* temp_window_name;

		if (name) ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);

		else ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

		const float backup_border_size = g.Style.ChildBorderSize;

		bool ret = Begin(temp_window_name, NULL, flags | ImGuiWindowFlags_NoBackground);

		ImGuiWindow* child_window = g.CurrentWindow;
		child_window->ChildId = id;
		child_window->AutoFitChildAxises = (ImS8)auto_fit_axises;

		if (child_window->BeginCount == 1) parent_window->DC.CursorPos = child_window->Pos;

		const ImGuiID temp_id_for_activation = ImHashStr("##Child", 0, id);
		if (g.ActiveId == temp_id_for_activation) ClearActiveID();

		if (g.NavActivateId == id && !(flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavWindowHasScrollY))
		{
			FocusWindow(child_window);
			NavInitWindow(child_window, false);
			SetActiveID(temp_id_for_activation, child_window);
			g.ActiveIdSource = g.NavInputSource;
		}
		return ret;
	}

	bool Child(const char* str_id, const ImVec2& size_arg, bool cap, ImGuiWindowFlags extra_flags)
	{
		ImGuiWindow* window = GetCurrentWindow();

		if (cap) {
			PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(13, 13));
			PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(13, 13));
		}
		return ChildEx(str_id, window->GetID(str_id), size_arg, cap, extra_flags | ImGuiWindowFlags_AlwaysUseWindowPadding);
	}

	bool ChildID(ImGuiID id, const ImVec2& size_arg, bool cap, ImGuiWindowFlags extra_flags)
	{
		IM_ASSERT(id != 0);
		return ChildEx(NULL, id, size_arg, cap, extra_flags);
	}

	void EndChild()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		PopStyleVar(2);

		IM_ASSERT(g.WithinEndChild == false);
		IM_ASSERT(window->Flags & ImGuiWindowFlags_ChildWindow);

		g.WithinEndChild = true;
		if (window->BeginCount > 1)
		{
			End();
		}
		else
		{
			ImVec2 sz = window->Size;

			if (window->AutoFitChildAxises & (1 << ImGuiAxis_X)) sz.x = ImMax(4.0f, sz.x);
			if (window->AutoFitChildAxises & (1 << ImGuiAxis_Y)) sz.y = ImMax(4.0f, sz.y);

			End();

			ImGuiWindow* parent_window = g.CurrentWindow;
			ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
			ItemSize(sz);
			if ((window->DC.NavLayersActiveMask != 0 || window->DC.NavWindowHasScrollY) && !(window->Flags & ImGuiWindowFlags_NavFlattened))
			{
				ItemAdd(bb, window->ChildId);
			}
			else
			{
				ItemAdd(bb, 0);

				if (window->Flags & ImGuiWindowFlags_NavFlattened) parent_window->DC.NavLayersActiveMaskNext |= window->DC.NavLayersActiveMaskNext;
			}
			if (g.HoveredWindow == window) g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
		}
		g.WithinEndChild = false;
		g.LogLinePosY = -FLT_MAX;
	}


	void BeginGroup()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		g.GroupStack.resize(g.GroupStack.Size + 1);
		ImGuiGroupData& group_data = g.GroupStack.back();
		group_data.WindowID = window->ID;
		group_data.BackupCursorPos = window->DC.CursorPos;
		group_data.BackupCursorMaxPos = window->DC.CursorMaxPos;
		group_data.BackupIndent = window->DC.Indent;
		group_data.BackupGroupOffset = window->DC.GroupOffset;
		group_data.BackupCurrLineSize = window->DC.CurrLineSize;
		group_data.BackupCurrLineTextBaseOffset = window->DC.CurrLineTextBaseOffset;
		group_data.BackupActiveIdIsAlive = g.ActiveIdIsAlive;
		group_data.BackupHoveredIdIsAlive = g.HoveredId != 0;
		group_data.BackupActiveIdPreviousFrameIsAlive = g.ActiveIdPreviousFrameIsAlive;
		group_data.EmitItem = true;

		window->DC.GroupOffset.x = window->DC.CursorPos.x - window->Pos.x - window->DC.ColumnsOffset.x;
		window->DC.Indent = window->DC.GroupOffset;
		window->DC.CursorMaxPos = window->DC.CursorPos;
		window->DC.CurrLineSize = ImVec2(0.0f, 0.0f);
		if (g.LogEnabled) g.LogLinePosY = -FLT_MAX;
	}

	void EndGroup()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		IM_ASSERT(g.GroupStack.Size > 0);

		ImGuiGroupData& group_data = g.GroupStack.back();
		IM_ASSERT(group_data.WindowID == window->ID);

		if (window->DC.IsSetPos) ErrorCheckUsingSetCursorPosToExtendParentBoundaries();

		ImRect group_bb(group_data.BackupCursorPos, ImMax(window->DC.CursorMaxPos, group_data.BackupCursorPos));

		window->DC.CursorPos = group_data.BackupCursorPos;
		window->DC.CursorMaxPos = ImMax(group_data.BackupCursorMaxPos, window->DC.CursorMaxPos);
		window->DC.Indent = group_data.BackupIndent;
		window->DC.GroupOffset = group_data.BackupGroupOffset;
		window->DC.CurrLineSize = group_data.BackupCurrLineSize;
		window->DC.CurrLineTextBaseOffset = group_data.BackupCurrLineTextBaseOffset;
		if (g.LogEnabled) g.LogLinePosY = -FLT_MAX;

		if (!group_data.EmitItem)
		{
			g.GroupStack.pop_back();
			return;
		}

		window->DC.CurrLineTextBaseOffset = ImMax(window->DC.PrevLineTextBaseOffset, group_data.BackupCurrLineTextBaseOffset);
		ItemSize(group_bb.GetSize());
		ItemAdd(group_bb, 0, NULL, ImGuiItemFlags_NoTabStop);

		const bool group_contains_curr_active_id = (group_data.BackupActiveIdIsAlive != g.ActiveId) && (g.ActiveIdIsAlive == g.ActiveId) && g.ActiveId;
		const bool group_contains_prev_active_id = (group_data.BackupActiveIdPreviousFrameIsAlive == false) && (g.ActiveIdPreviousFrameIsAlive == true);
		if (group_contains_curr_active_id) g.LastItemData.ID = g.ActiveId;
		else if (group_contains_prev_active_id) g.LastItemData.ID = g.ActiveIdPreviousFrame;
		g.LastItemData.Rect = group_bb;

		const bool group_contains_curr_hovered_id = (group_data.BackupHoveredIdIsAlive == false) && g.HoveredId != 0;
		if (group_contains_curr_hovered_id) g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;

		if (group_contains_curr_active_id && g.ActiveIdHasBeenEditedThisFrame) g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Edited;

		g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasDeactivated;
		if (group_contains_prev_active_id && g.ActiveId != g.ActiveIdPreviousFrame) g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_Deactivated;

		g.GroupStack.pop_back();
	}




	void Separator_line()
	{
		GetWindowDrawList()->AddRectFilled(GetCursorScreenPos(), GetCursorScreenPos() + ImVec2(GetContentRegionMax().x - GetStyle().WindowPadding.x, 1), GetColorU32(c::separator));
		Spacing();
	}

	void SeparatorEx(ImGuiSeparatorFlags flags, float thickness)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return;

		ImGuiContext& g = *GImGui;
		IM_ASSERT(ImIsPowerOfTwo(flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical)));
		IM_ASSERT(thickness > 0.0f);

		if (flags & ImGuiSeparatorFlags_Vertical)
		{
			float y1 = window->DC.CursorPos.y;
			float y2 = window->DC.CursorPos.y + window->DC.CurrLineSize.y;
			const ImRect bb(ImVec2(window->DC.CursorPos.x, y1 + (GetStyle().ItemSpacing.y / 2)), ImVec2(window->DC.CursorPos.x + thickness, y2 - (GetStyle().ItemSpacing.y / 2)));


			ItemSize(ImVec2(thickness, 0.0f));
			if (!ItemAdd(bb, 0)) return;

			window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(c::child::background));

			ImGui::SameLine();
		}
		else if (flags & ImGuiSeparatorFlags_Horizontal)
		{
			float x1 = window->Pos.x;
			float x2 = window->Pos.x + window->Size.x;

			if (g.GroupStack.Size > 0 && g.GroupStack.back().WindowID == window->ID) x1 += window->DC.Indent.x;

			if (ImGuiTable* table = g.CurrentTable)
			{
				x1 = table->Columns[table->CurrentColumn].MinX;
				x2 = table->Columns[table->CurrentColumn].MaxX;
			}

			ImGuiOldColumns* columns = (flags & ImGuiSeparatorFlags_SpanAllColumns) ? window->DC.CurrentColumns : NULL;
			if (columns) PushColumnsBackground();

			const float thickness_for_layout = (thickness == 1.0f) ? 0.0f : thickness;
			const ImRect bb(ImVec2(x1 + GetStyle().WindowPadding.x, window->DC.CursorPos.y), ImVec2(x2 - GetStyle().WindowPadding.x, window->DC.CursorPos.y + thickness));

			ItemSize(ImVec2(0.0f, thickness_for_layout));

			if (ItemAdd(bb, 0))
			{
				window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(c::separator));
			}
			if (columns)
			{
				PopColumnsBackground();
				columns->LineMinY = window->DC.CursorPos.y;
			}
		}
	}

	void Separator()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems) return;

		ImGuiSeparatorFlags flags = (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
		flags |= ImGuiSeparatorFlags_SpanAllColumns;
		SeparatorEx(flags, 1.0f);
	}

	struct theme_state
	{
		ImVec4 background;
		float smooth_swap, alpha_line, line_size;
	};

	bool ThemeButton(const char* id_theme, bool dark, const ImVec2& size_arg)
	{
		ImGuiWindow* window = GetCurrentWindow();

		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(id_theme);
		const ImVec2 label_size = CalcTextSize(id_theme, NULL, true), pos = window->DC.CursorPos;

		ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

		const ImRect bb(pos, pos + size);

		ItemSize(size, 0.f);
		if (!ItemAdd(bb, id)) return false;

		bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, NULL);

		static std::map<ImGuiID, theme_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, theme_state() });
			it_anim = anim.find(id);
		}

		it_anim->second.background = ImLerp(it_anim->second.background, dark || hovered ? c::page::background_active : c::page::background, g.IO.DeltaTime * 6.f);

		it_anim->second.alpha_line = ImLerp(it_anim->second.alpha_line, dark ? 1.f : 0.f, g.IO.DeltaTime * 6.f);
		it_anim->second.line_size = ImLerp(it_anim->second.line_size, dark ? (size_arg.x / 4) : (size_arg.x / 2), g.IO.DeltaTime * 6.f);

		it_anim->second.smooth_swap = ImLerp(it_anim->second.smooth_swap, dark ? 26.f : 0, g.IO.DeltaTime * 12.f);

		GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(it_anim->second.background), c::page::rounding);

		PushClipRect(bb.Min, bb.Max, true);

		PushFont(font::icomoon_page);
		GetWindowDrawList()->AddText(ImVec2(bb.Min.x + (size_arg.x - CalcTextSize("k").x) / 2, bb.Max.y - CalcTextSize("k").y - (size.y - CalcTextSize("k").y) / 2 + it_anim->second.smooth_swap), GetColorU32(c::accent), "k");
		GetWindowDrawList()->AddText(ImVec2(bb.Min.x + (size_arg.x - CalcTextSize("a").x) / 2, bb.Max.y - CalcTextSize("a").y - (size.y - CalcTextSize("a").y) / 2 - 25 + it_anim->second.smooth_swap), GetColorU32(c::accent), "a");
		PopFont();

		PopClipRect();

		return pressed;
	}



	struct button_state
	{
		ImVec4 background, text;
		ImVec4 shader_col;
		float shader_alpha = 0.f;
	};

	bool Button(const char* label, const ImVec2& size_arg)
	{
		ImGuiWindow* window = GetCurrentWindow();

		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = CalcTextSize(label, NULL, true), pos = window->DC.CursorPos;

		ImVec2 size = CalcItemSize(size_arg, label_size.x, label_size.y);

		const ImRect bb(pos, pos + size);

		ItemSize(size, 0.f);
		if (!ItemAdd(bb, id)) return false;

		bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, NULL);

		static std::map<ImGuiID, button_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, button_state() });
			it_anim = anim.find(id);
		}

		const bool active_or_hovered = IsItemActive() || hovered;

		// Keep a neutral base fill; main visual feedback comes from shaderrt_v2 now
		it_anim->second.background = ImLerp(it_anim->second.background,
			c::elements::background,
			g.IO.DeltaTime * 6.f);
		it_anim->second.text = ImLerp(it_anim->second.text,
			active_or_hovered ? c::label::active : c::label::regular,
			g.IO.DeltaTime * 6.f);

		// Animate shader intensity
		it_anim->second.shader_alpha = ImLerp(it_anim->second.shader_alpha,
			active_or_hovered ? 0.7f : 0.0f,
			GetAnimSpeed() * 2.0f);

		ImDrawList* dl = GetWindowDrawList();

		// Base filled rect
		dl->AddRectFilled(bb.Min, bb.Max, GetColorU32(it_anim->second.background), c::page::rounding);

		// Shader overlay (same style as other elements using shaderrt_v2)
		if (it_anim->second.shader_alpha > 0.01f)
		{
			shaderrt_v2::Draw_v2(
				dl,
				bb.Min,
				bb.Max,
				c::page::rounding,
				it_anim->second.shader_alpha,
				ImShaderTex_WindowBg_v2
			);
		}

		// Stroke
		dl->AddRect(bb.Min, bb.Max, c::stroke_color, c::page::rounding);

		PushClipRect(bb.Min, bb.Max, true);

		GetWindowDrawList()->AddText(ImVec2(bb.Min.x + (size_arg.x - CalcTextSize(label).x) / 2, bb.Max.y - CalcTextSize(label).y - (size.y - CalcTextSize(label).y) / 2), GetColorU32(it_anim->second.text), label);


		PopClipRect();

		return pressed;
	}


	struct tab_state {
		ImVec4 text_col[2], icon_col;
		ImVec4 frame_col;
		ImVec4 line_col;
		bool is_want = true;
		ImVec2 frame_offset;
		ImDrawList* draw_list;
		float size_scale = 1.f;
		float blur_thinkess;
		float highlight_offset = -72.f;
		float highlight_alpha;
		float width_expand = 0.f;  // Width expansion animation (0 = icon only, 1 = full width with text)
		float text_alpha = 0.f;     // Text fade-in animation
		float shader_alpha = 0.f;   // Shader background animation
	};

	bool Tab(const char* label, const char* icon, int* v, int number) {
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const float square_sz = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;


		static std::map<ImGuiID, tab_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end()) {
			anim.insert({ id, tab_state() });
			it_anim = anim.find(id);
		}

		const bool is_active = (*v == number);
		const float icon_size = 45.f;
		const float expanded_width = 150.f;
		const float collapsed_width = icon_size;

		// Check hover on collapsed size first
		const ImRect collapsed_bb(pos, pos + ImVec2(collapsed_width, icon_size));
		bool hovered, held;
		bool pressed = ButtonBehavior(collapsed_bb, id, &hovered, &held);

		if (pressed) {
			it_anim->second.is_want = true;
		}

		if (it_anim->second.is_want) {
			//*v = number;
			it_anim->second.is_want = false;
		}

		// Animate width expansion (expand ONLY on hover, not when active)
		const bool should_expand = hovered;
		it_anim->second.width_expand = ImLerp(
			it_anim->second.width_expand,
			should_expand ? 1.f : 0.f,
			GetAnimSpeed() * 1.5f
		);

		// Animate text alpha (show text when expanded OR when active)
		it_anim->second.text_alpha = ImLerp(
			it_anim->second.text_alpha,
			(hovered || is_active) ? 1.f : 0.f,
			GetAnimSpeed() * 1.5f
		);

		// Calculate current width based on expansion
		const float current_width = ImLerp(collapsed_width, expanded_width, it_anim->second.width_expand);
		const ImRect frame_bb(pos, pos + ImVec2(current_width, icon_size));

		const ImRect total_bb(frame_bb.Min - it_anim->second.frame_offset, frame_bb.Max + it_anim->second.frame_offset);

		const ImRect icon_bb(total_bb.Min, total_bb.Min + ImVec2(icon_size, icon_size));

		ItemSize(frame_bb, style.FramePadding.y);
		ItemAdd(frame_bb, id);

		it_anim->second.frame_col = ImLerp(
			it_anim->second.frame_col,
			is_active ? utils::GetColorWithAlpha(c::anim::active, 0.35f) : utils::GetColorWithAlpha(c::anim::active, 0.f),
			GetAnimSpeed());

		it_anim->second.text_col[0] = ImLerp(
			it_anim->second.text_col[0],
			is_active ? c::anim::active : c::text::label::regular,
			GetAnimSpeed());

		it_anim->second.text_col[1] = ImLerp(
			it_anim->second.text_col[1],
			is_active ? c::anim::active : c::text::label::hovered,
			GetAnimSpeed());

		it_anim->second.icon_col = ImLerp(
			it_anim->second.icon_col,
			is_active ? c::anim::active : utils::GetColorWithAlpha(c::anim::active, 0.4f),
			GetAnimSpeed());

		it_anim->second.draw_list = GetWindowDrawList();
		it_anim->second.size_scale = 1.f;
		it_anim->second.frame_offset = ImVec2(0, 0);
		it_anim->second.blur_thinkess = ImLerp(it_anim->second.blur_thinkess, hovered ? 1.f : 0.f, GetAnimSpeed() * 2);
		it_anim->second.highlight_offset = ImLerp(it_anim->second.highlight_offset, hovered ? total_bb.GetSize().x + 52.f : -112.f, GetAnimSpeed() / 3);
		it_anim->second.highlight_alpha = 0.8f;

		// Animate shader alpha for background effect
		it_anim->second.shader_alpha = ImLerp(it_anim->second.shader_alpha, (hovered || is_active) ? 0.65f : 0.f, GetAnimSpeed());

		if (!hovered)
			it_anim->second.highlight_offset = -112.f;

		// Shader animation background effect
		shaderrt_v2::Draw_v2(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it_anim->second.shader_alpha, ImShaderTex_WindowBg_v2);

		it_anim->second.draw_list->AddShadowRect(total_bb.Min, total_bb.Max, ImColor(0.f, 0.f, 0.f, it_anim->second.blur_thinkess), 70.f, ImVec2(0, 0), ImDrawFlags_ShadowCutOutShapeBackground, c::elements::rounding);
		crr::Push(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding);
		it_anim->second.draw_list->AddImage(c::highlight_image, total_bb.Min + ImVec2(it_anim->second.highlight_offset - 32, -32), total_bb.Min + ImVec2(it_anim->second.highlight_offset + total_bb.GetSize().y + 32, total_bb.GetSize().y + 32), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, it_anim->second.highlight_alpha));
		crr::Pop(it_anim->second.draw_list);

		img_blur::Before(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it_anim->second.blur_thinkess, 0, false);


		//const int vtx_idx_1 = it_anim->second.draw_list->VtxBuffer.Size;
		//it_anim->second.draw_list->AddRectFilled(total_bb.Min, total_bb.Max, ImColor(1.f, 1.f, 1.f, 1.f), 6.f);
		//const int vtx_idx_2 = it_anim->second.draw_list->VtxBuffer.Size;
		//ShadeVertsLinearColorGradientSetAlpha(it_anim->second.draw_list, vtx_idx_1, vtx_idx_2, total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.frame_col), utils::GetColorWithAlpha(it_anim->second.frame_col, 0.f));

		// Apply shader to icon background on hover/active
		if (it_anim->second.shader_alpha > 0.01f)
		{
			shaderrt_v2::Draw_v2(it_anim->second.draw_list, icon_bb.Min, icon_bb.Max, c::elements::rounding, it_anim->second.shader_alpha, ImShaderTex_WindowBg_v2);
		}

		it_anim->second.draw_list->AddShadowCircle(utils::center_text(total_bb.Min, total_bb.Min + ImVec2(total_bb.GetSize().y, total_bb.GetSize().y), icon), 6.f, GetColorU32(it_anim->second.icon_col), 35.f, CalcTextSize(icon) / 2);
		it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize, total_bb.Min + ImVec2(total_bb.GetSize().y, total_bb.GetSize().y) / 2 - (CalcTextSize(icon)) / 2, GetColorU32(it_anim->second.icon_col), icon);



		//window->DrawList->AddRect(total_bb.Min - ImVec2(1, 1), total_bb.Max + ImVec2(1, 1), c::, 6.f, 0, 3.f);

		// Draw text only when expanded (with fade animation)
		if (it_anim->second.text_alpha > 0.01f && it_anim->second.width_expand > 0.1f)
		{
			ImVec2 text_pos = ImVec2(
				total_bb.Min.x + icon_size + 12.f,
				utils::center_text(total_bb.Min, total_bb.Max, label).y
			);
			ImColor text_color = ImColor(
				it_anim->second.text_col[0].x,
				it_anim->second.text_col[0].y,
				it_anim->second.text_col[0].z,
				it_anim->second.text_alpha * it_anim->second.text_col[0].w
			);
			it_anim->second.draw_list->AddText(
				ImGui::GetDefaultFont(),
				ImGui::GetDefaultFont()->FontSize,
				text_pos,
				text_color,
				label
			);
		}

		/*const int vtx_idx_4 = it_anim->second.draw_list->VtxBuffer.Size;
		ShadeVertsLinearColorGradientSetAlpha(GetWindowDrawList(),
			vtx_idx_3,
			vtx_idx_4,
			total_bb.Min + ImVec2(38, 0),
			total_bb.Min + ImVec2(38 + CalcTextSize(label).x, total_bb.GetSize().y),
			GetColorU32(it_anim->second.text_col[0]),
			GetColorU32(it_anim->second.text_col[1]));*/


		ItemBackground(id, total_bb, hovered, it_anim->second.draw_list);

		const int vtx_idx_0 = it_anim->second.draw_list->VtxBuffer.Size;
		ImColor icon_bg_col = is_active ? utils::GetColorWithAlpha(c::anim::active, 0.18f) : ImColor(1.f, 1.f, 1.f, 0.05f);
		it_anim->second.draw_list->AddRectFilled(icon_bb.Min, icon_bb.Max, icon_bg_col, c::elements::rounding);
		const int vtx_idx_1 = it_anim->second.draw_list->VtxBuffer.Size;
		ShadeVertsVerticalGradient(it_anim->second.draw_list, vtx_idx_0, vtx_idx_1, icon_bb.Min.y, icon_bb.Max.y, ImColor(1.f, 1.f, 1.f, 0.05f), ImColor(1.f, 1.f, 1.f, 0.01f));


		const int vtx_idx_2 = it_anim->second.draw_list->VtxBuffer.Size;
		it_anim->second.draw_list->AddRect(icon_bb.Min, icon_bb.Max, ImColor(1.f, 1.f, 1.f, 1.f), c::elements::rounding);
		const int vtx_idx_3 = it_anim->second.draw_list->VtxBuffer.Size;
		ShadeVertsVerticalGradient(it_anim->second.draw_list, vtx_idx_2, vtx_idx_3, icon_bb.Min.y - 1, icon_bb.Max.y + 1, ImColor(1.f, 1.f, 1.f, 0.01f), ImColor(1.f, 1.f, 1.f, 0.05f));


		return pressed;
	}

	struct subtab_state {
		ImVec4 text_col;
		ImVec4 frame_col;
		ImVec4 line_col;
	};

	bool SubTab(const char* label, int* v, int number, ImColor color)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const float square_sz = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, pos + ImVec2(20 + CalcTextSize(label).x, 25));


		ItemSize(total_bb, style.FramePadding.y);
		ItemAdd(total_bb, id);

		static std::map<ImGuiID, subtab_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, subtab_state() });
			it_anim = anim.find(id);
		}

		// Require subtab to be fully visible before allowing hover/zoom
		// Subtab doesn't have zoom offsets, so check total_bb directly
		ImRect clipped_bb = total_bb;
		clipped_bb.ClipWith(window->ClipRect);

		bool hovered = false;
		bool held = false;
		bool pressed = false;

		// Check if the entire widget is visible (not cut off)
		const bool fully_visible = !clipped_bb.IsInverted() &&
			clipped_bb.Min.x <= total_bb.Min.x + 1.0f &&
			clipped_bb.Min.y <= total_bb.Min.y + 1.0f &&
			clipped_bb.Max.x >= total_bb.Max.x - 1.0f &&
			clipped_bb.Max.y >= total_bb.Max.y - 1.0f;

		if (fully_visible)
			pressed = ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed)
		{
			*v = number;
		}

		RenderNavHighlight(total_bb, id);

		it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, *v == number ? color : utils::GetColorWithAlpha(color, 0.f), GetAnimSpeed());
		it_anim->second.text_col = ImLerp(it_anim->second.text_col, *v == number ? color : c::label::active, GetAnimSpeed());

		window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.frame_col), style.FrameRounding);

		window->DrawList->AddRect(total_bb.Min, total_bb.Max, color, style.FrameRounding);

		window->DrawList->AddText(utils::center_text(total_bb.Min, total_bb.Max, label), c::label::active, label);

		return pressed;
	}

	struct arrow_state {
		ImVec4 arrow_col;
		ImVec4 frame_col;
	};

	bool ArrowButton(const char* label, ImGuiDir dir)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		const float square_sz = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect total_bb(pos, pos + ImVec2(20, 20));

		ItemSize(total_bb, style.FramePadding.y);
		ItemAdd(total_bb, id);

		static std::map<ImGuiID, arrow_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, arrow_state() });
			it_anim = anim.find(id);
		}

		// Require arrow button to be fully visible before allowing hover/zoom
		// Arrow button doesn't have zoom offsets, so check total_bb directly
		ImRect clipped_bb = total_bb;
		clipped_bb.ClipWith(window->ClipRect);

		bool hovered = false;
		bool held = false;
		bool pressed = false;

		// Check if the entire widget is visible (not cut off)
		const bool fully_visible = !clipped_bb.IsInverted() &&
			clipped_bb.Min.x <= total_bb.Min.x + 1.0f &&
			clipped_bb.Min.y <= total_bb.Min.y + 1.0f &&
			clipped_bb.Max.x >= total_bb.Max.x - 1.0f &&
			clipped_bb.Max.y >= total_bb.Max.y - 1.0f;

		if (fully_visible)
			pressed = ButtonBehavior(total_bb, id, &hovered, &held);

		RenderNavHighlight(total_bb, id);

		it_anim->second.frame_col = ImLerp(it_anim->second.frame_col, hovered ? c::anim::active : utils::GetColorWithAlpha(c::anim::active, 0.f), GetAnimSpeed());
		it_anim->second.arrow_col = ImLerp(it_anim->second.arrow_col, hovered ? utils::GetDarkColor(c::anim::active) : c::anim::active, GetAnimSpeed());

		window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, GetColorU32(it_anim->second.frame_col), c::elements::rounding);
		window->DrawList->AddRect(total_bb.Min, total_bb.Max, stroke_color, c::elements::rounding);

		PushFont(font::icomoon_page);
		window->DrawList->AddText(utils::center_text(total_bb.Min, total_bb.Max, dir == ImGuiDir_Right ? "r" : dir == ImGuiDir_Left ? "l" : dir == ImGuiDir_Up ? "u" : "d"), GetColorU32(it_anim->second.arrow_col), dir == ImGuiDir_Right ? "r" : dir == ImGuiDir_Left ? "l" : dir == ImGuiDir_Up ? "u" : "d");
		PopFont();

		return IsItemClicked();
	}

	struct check_state
	{
		float checkbox_alpha;
		ImVec4 text_color, description_col;
		ImVec4 rect_color;
		ImVec2 frame_offset;
		ImDrawList* draw_list;
		float size_scale = 1.f;
		float blur_thinkess;
		float highlight_offset = -72.f;
		float highlight_alpha;
		float shader_alpha = 0.f;  // For shaderrt::Draw background animation
		float bg_alpha = 0.4f;     // For widget background opacity
	};

	bool Checkbox(const char* label, const char* description, bool* v)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		std::string label_str = label;
		std::string arrows_str[2] = { label_str + "left", label_str + "right" };

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = CalcTextSize(label, NULL, true);


		static std::map<ImGuiID, check_state> anim;
		auto it_anim = anim.emplace(id, check_state()).first;



		const float square_sz = 10;
		const ImVec2 pos = window->DC.CursorPos;

		const float w = GetContentRegionAvail().x;


		const ImRect frame_bb(pos, pos + ImVec2(w, 55));
		const ImRect item_bb(pos, pos + ImVec2(w, 55));
		const ImRect total_bb(pos - it_anim->second.frame_offset, pos + ImVec2(w, 55) + it_anim->second.frame_offset);

		ItemSize(frame_bb, 0.f);

		if (!ItemAdd(frame_bb, id)) return false;

		// Require checkbox to be fully visible before allowing hover/zoom
		// Check visibility on base frame_bb (not zoomed total_bb) to avoid glitches during zoom
		ImRect clipped_bb = frame_bb;
		clipped_bb.ClipWith(window->ClipRect);

		bool hovered = false;
		bool held = false;
		bool pressed = false;

		// Check if the entire base widget is visible (not cut off)
		const bool fully_visible = !clipped_bb.IsInverted() &&
			clipped_bb.Min.x <= frame_bb.Min.x + 1.0f &&
			clipped_bb.Min.y <= frame_bb.Min.y + 1.0f &&
			clipped_bb.Max.x >= frame_bb.Max.x - 1.0f &&
			clipped_bb.Max.y >= frame_bb.Max.y - 1.0f;

		if (fully_visible)
		{
			// Block click-through: only allow interaction when this window is truly on top.
			const bool same_window_hovered = (g.HoveredWindow == window);
			pressed = ButtonBehavior(total_bb, id, &hovered, &held);
			hovered = hovered && same_window_hovered && !c::is_picker_open;
			if (!same_window_hovered)
				pressed = false;
		}

		if (pressed)
			*v = !(*v);

		// Animate shader alpha for background effect - only visible when checkbox is enabled
		it_anim->second.shader_alpha = ImLerp(it_anim->second.shader_alpha, *v ? 0.65f : 0.f, GetAnimSpeed());

		it_anim->second.checkbox_alpha = ImLerp(it_anim->second.checkbox_alpha, *v ? 1.f : 0.f, GetAnimSpeed());
		it_anim->second.rect_color = ImLerp(it_anim->second.rect_color, *v ? c::anim::active : utils::GetColorWithAlpha(c::anim::active, 0.f), GetAnimSpeed());
		it_anim->second.text_color = ImLerp(it_anim->second.text_color, *v ? c::anim::active : hovered ? c::text::label::hovered : c::text::label::regular, GetAnimSpeed());

		it_anim->second.description_col = ImLerp(it_anim->second.description_col, *v ? c::text::description::active : hovered ? c::text::description::hovered : c::text::description::regular, GetAnimSpeed());
		it_anim->second.draw_list = it_anim->second.size_scale > 1.01f ? GetForegroundDrawList() : GetWindowDrawList();
		it_anim->second.size_scale = ImLerp(it_anim->second.size_scale, hovered ? 1.05f : 1.f, GetAnimSpeed());
		it_anim->second.frame_offset = ImLerp(it_anim->second.frame_offset, hovered ? ImVec2(50 * it_anim->second.size_scale, 12.5f * it_anim->second.size_scale) : ImVec2(0, 0), GetAnimSpeed());
		it_anim->second.blur_thinkess = ImLerp(it_anim->second.blur_thinkess, hovered ? 1.f : 0.f, GetAnimSpeed() * 2);
		it_anim->second.highlight_offset = ImLerp(it_anim->second.highlight_offset, hovered ? total_bb.GetSize().x + 52.f : -112.f, GetAnimSpeed() / 3);
		it_anim->second.highlight_alpha = 0.8f;

		// Background Opacity Animation
		it_anim->second.bg_alpha = ImLerp(it_anim->second.bg_alpha, hovered ? 1.f : 0.4f, GetAnimSpeed());

		if (!hovered)
			it_anim->second.highlight_offset = -112.f;

		ImVec2 check_offset = ImVec2(15.f * it_anim->second.size_scale, 15.f * it_anim->second.size_scale);

		ImRect check_rect(total_bb.Max - ImVec2(total_bb.GetSize().y, total_bb.GetSize().y), total_bb.Max);

		ImRect check_bb(check_rect.GetCenter() - check_offset, check_rect.GetCenter() + check_offset);

		// Draw Widget Background (Opaque on hover to hide elements behind)
		it_anim->second.draw_list->AddRectFilled(total_bb.Min, total_bb.Max, utils::GetColorWithAlpha(c::window_bg_color, it_anim->second.bg_alpha), c::elements::rounding);

		// Shader animation background effect
		shaderrt_v2::Draw_v2(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it_anim->second.shader_alpha, ImShaderTex_WindowBg_v2);

		// Shine effect (shadow + highlight) around checkbox
		it_anim->second.draw_list->AddShadowRect(
			total_bb.Min, total_bb.Max,
			ImColor(0.f, 0.f, 0.f, it_anim->second.blur_thinkess),
			70.f, ImVec2(0, 0),
			ImDrawFlags_ShadowCutOutShapeBackground, c::elements::rounding);

		crr::Push(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding);
		it_anim->second.draw_list->AddImage(
			c::highlight_image,
			total_bb.Min + ImVec2(it_anim->second.highlight_offset - 32 * it_anim->second.size_scale, -32 * it_anim->second.size_scale),
			total_bb.Min + ImVec2(it_anim->second.highlight_offset + total_bb.GetSize().y + 32 * it_anim->second.size_scale, total_bb.GetSize().y + 32 * it_anim->second.size_scale),
			ImVec2(0, 0), ImVec2(1, 1),
			ImColor(1.f, 1.f, 1.f, it_anim->second.highlight_alpha));
		crr::Pop(it_anim->second.draw_list);

		ItemBackground(id, total_bb, hovered, it_anim->second.draw_list);

		it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * it_anim->second.size_scale, ImVec2(total_bb.Min.x + 14.f * it_anim->second.size_scale, total_bb.GetCenter().y - CalcTextSize(label).y), GetColorU32(it_anim->second.text_color), label);

		ImRect descrtiption_bb(ImVec2(total_bb.Min.x + 14.f * it_anim->second.size_scale, total_bb.GetCenter().y), ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y) + CalcTextSize(description));
		ItemDescription(description, id, ImRect{ ImVec2(check_bb.Min.x - 25, descrtiption_bb.Min.y), ImVec2(check_bb.Min.x - 5, descrtiption_bb.Max.y) }, hovered, ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y), it_anim->second.description_col, it_anim->second.draw_list, font::description_font, font::description_font->FontSize * it_anim->second.size_scale);


		it_anim->second.draw_list->AddRectFilled(check_bb.Min, check_bb.Max, c::second_color, c::elements::rounding);
		it_anim->second.draw_list->AddRectFilled(check_bb.Min + ImVec2(1, 1), check_bb.Max - ImVec2(1, 1), GetColorU32(it_anim->second.rect_color), c::elements::rounding);
		it_anim->second.draw_list->AddRect(check_bb.Min, check_bb.Max, c::stroke_color, c::elements::rounding);
		it_anim->second.draw_list->AddShadowCircle(check_bb.GetCenter(), 10.f, utils::GetColorWithAlpha(it_anim->second.rect_color, it_anim->second.checkbox_alpha * 0.75f), 95.f, ImVec2(0, 0));
		RenderCheckMark(it_anim->second.draw_list, check_bb.GetCenter() - ImVec2(7, 7), utils::GetColorWithAlpha(c::window_bg_color, it_anim->second.checkbox_alpha), 14.f);


		return pressed;
	}

	struct checkboxclicked_state
	{
		ImVec4 text_color_offset;
		ImVec4 rect_color;
		ImVec4 circle_color;
		float circle_offset;
	};

	bool CheckboxClicked(const char* label, bool* v)
	{
		ImGuiWindow* window = GetCurrentWindow();
		std::string name = label;

		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		static std::map<ImGuiID, checkboxclicked_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, checkboxclicked_state() });
			it_anim = anim.find(id);
		}

		const float square_sz = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		ImRect total_bb(pos, pos + ImVec2(75 + CalcTextSize(label).x, 50));

		ItemSize(total_bb, style.FramePadding.y);
		ItemAdd(total_bb, id);

		// Require toggle widget to be fully visible before allowing hover/zoom
		// Toggle doesn't have zoom offsets, so check total_bb directly
		ImRect clipped_bb = total_bb;
		clipped_bb.ClipWith(window->ClipRect);

		bool hovered = false;
		bool held = false;
		bool pressed = false;

		// Check if the entire widget is visible (not cut off)
		const bool fully_visible = !clipped_bb.IsInverted() &&
			clipped_bb.Min.x <= total_bb.Min.x + 1.0f &&
			clipped_bb.Min.y <= total_bb.Min.y + 1.0f &&
			clipped_bb.Max.x >= total_bb.Max.x - 1.0f &&
			clipped_bb.Max.y >= total_bb.Max.y - 1.0f;

		if (fully_visible)
			pressed = ButtonBehavior(total_bb, id, &hovered, &held);
		if (pressed) *v = !(*v);

		it_anim->second.circle_offset = ImLerp(it_anim->second.circle_offset, *v ? 20.f : 0.f, GetAnimSpeed());
		it_anim->second.rect_color = ImLerp(it_anim->second.rect_color, *v ? c::anim::active : ImColor(0.1f, 0.1f, 0.1f, 0.5f), GetAnimSpeed());
		it_anim->second.circle_color = ImLerp(it_anim->second.circle_color, *v ? ImColor(1.f, 1.f, 1.f, 1.f) : ImColor(0.6f, 0.6f, 0.6f, 1.f), GetAnimSpeed());
		it_anim->second.text_color_offset = ImLerp(it_anim->second.text_color_offset, *v ? ImColor(0.f, 0.f, 0.f, 0.0f) : ImColor(0.f, 0.f, 0.f, 0.5f), GetAnimSpeed());

		window->DrawList->AddRectFilled(total_bb.Max - ImVec2(60, 35), total_bb.Max - ImVec2(20, 15), GetColorU32(it_anim->second.rect_color), 25);
		window->DrawList->AddRect(total_bb.Max - ImVec2(60, 35), total_bb.Max - ImVec2(20, 15), second_color, 25, 0, 1.5f);
		window->DrawList->AddCircleFilled(total_bb.Max - ImVec2(50 - it_anim->second.circle_offset, 25), 7.f, GetColorU32(it_anim->second.circle_color), 60);

		window->DrawList->AddText(ImVec2(total_bb.Min.x + 5.f, utils::center_text(total_bb.Min, total_bb.Max, label).y), c::label::active, label);
		return pressed;
	}

	static float CalcMaxPopupHeightFromItemCount(int items_count)
	{
		ImGuiContext& g = *GImGui;
		if (items_count <= 0)
			return FLT_MAX;
		return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
	}

	int rotation_start_index;
	void ImRotateStart()
	{
		rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
	}

	ImVec2 ImRotationCenter()
	{
		ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX);

		const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
		for (int i = rotation_start_index; i < buf.Size; i++)
			l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

		return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2);
	}

	void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
	{
		float s = sin(rad), c = cos(rad);
		center = ImRotate(center, s, c) - center;

		auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
		for (int i = rotation_start_index; i < buf.Size; i++)
			buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
	}

	struct begin_state
	{
		ImVec4 background, text;
		float open, alpha, combo_size = 0.f, shadow_opticaly;
		bool opened_combo = false, hovered = false;
		float arrow_roll;
		ImVec4 description_col;

		ImVec2 frame_offset;
		ImDrawList* draw_list;
		float size_scale = 1.f;
		float blur_thinkess;
		float highlight_offset = -72.f;
		float highlight_alpha;
		float bg_alpha = 0.4f;
		float shader_alpha = 0.f;  // For shaderrt_v2::Draw_v2 background animation
	};

	bool BeginCombo(const char* label, const char* description, const char* preview_value, int val, bool multi, ImGuiComboFlags flags)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();

		// ImDrawList::AddText uses strlen(); NULL preview crashes (e.g. combo index out of range).
		const char* preview_str = preview_value ? preview_value : "";

		g.NextWindowData.ClearFlags();
		if (window->SkipItems) return false;

		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);


		const ImVec2 pos = window->DC.CursorPos;

		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const float w = GetContentRegionAvail().x;

		static std::map<ImGuiID, begin_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, begin_state() });
			it_anim = anim.find(id);
		}

		const ImRect frame_bb(pos, pos + ImVec2(w, 45));

		const ImRect total_bb(pos - it_anim->second.frame_offset, pos + ImVec2(w, 45) + it_anim->second.frame_offset);


		const ImRect bb_box(total_bb.Max - ImVec2(total_bb.GetWidth() / 2, total_bb.GetHeight()), total_bb.Max);

		float box_offset = bb_box.Max.x - ImGui::GetCurrentWindow()->Size.x / 2 - 10;

		const ImRect bb(ImVec2(total_bb.Min.x + CalcTextSize(label).x + 10 < box_offset ? box_offset : total_bb.Min.x + CalcTextSize(label).x + 10, bb_box.Min.y), bb_box.Max);

		ItemSize(frame_bb, 0.f);

		if (!ItemAdd(frame_bb, id, &bb)) return false;

		// Require combo to be fully visible before allowing hover/zoom
		// Check visibility on base frame_bb (not zoomed total_bb) to avoid glitches during zoom
		ImRect clipped_bb = frame_bb;
		clipped_bb.ClipWith(window->ClipRect);

		bool hovered = false;
		bool held = false;
		bool pressed = false;

		// Check if the entire base widget is visible (not cut off)
		const bool fully_visible = !clipped_bb.IsInverted() &&
			clipped_bb.Min.x <= frame_bb.Min.x + 1.0f &&
			clipped_bb.Min.y <= frame_bb.Min.y + 1.0f &&
			clipped_bb.Max.x >= frame_bb.Max.x - 1.0f &&
			clipped_bb.Max.y >= frame_bb.Max.y - 1.0f;

		if (fully_visible)
			pressed = ButtonBehavior(total_bb, id, &hovered, &held);



		if (pressed)
		{
			it_anim->second.opened_combo = !it_anim->second.opened_combo;
		}
		else if (it_anim->second.opened_combo && g.IO.MouseClicked[0] && !hovered && !it_anim->second.hovered)
		{
			// Close only when clicking outside both the combo control and popup.
			it_anim->second.opened_combo = false;
		}
		it_anim->second.arrow_roll = ImLerp(it_anim->second.arrow_roll, it_anim->second.opened_combo ? -1.f : 1.f, g.IO.DeltaTime * 6.f);
		it_anim->second.text = ImLerp(it_anim->second.text, it_anim->second.opened_combo ? c::anim::active : hovered ? c::text::label::hovered : c::text::label::regular, GetAnimSpeed());
		it_anim->second.background = ImLerp(it_anim->second.background, it_anim->second.opened_combo ? c::second_color.Value : c::elements::background, g.IO.DeltaTime * 6.f);
		it_anim->second.combo_size = ImLerp(it_anim->second.combo_size, it_anim->second.opened_combo ? (val * 32) + 22 : 0.f, g.IO.DeltaTime * 12.f);
		it_anim->second.description_col = ImLerp(it_anim->second.description_col, hovered ? c::text::description::hovered : c::text::description::regular, GetAnimSpeed());

		it_anim->second.draw_list = it_anim->second.size_scale > 1.01f ? GetForegroundDrawList() : GetWindowDrawList();
		it_anim->second.size_scale = ImLerp(it_anim->second.size_scale, hovered || it_anim->second.opened_combo ? 1.05f : 1.f, GetAnimSpeed());
		it_anim->second.frame_offset = ImLerp(it_anim->second.frame_offset, hovered || it_anim->second.opened_combo ? ImVec2(50 * it_anim->second.size_scale, 12.5f * it_anim->second.size_scale) : ImVec2(0, 0), GetAnimSpeed());
		it_anim->second.blur_thinkess = ImLerp(it_anim->second.blur_thinkess, hovered || it_anim->second.opened_combo ? 1.f : 0.f, GetAnimSpeed() * 2);
		it_anim->second.highlight_offset = ImLerp(it_anim->second.highlight_offset, hovered || it_anim->second.opened_combo ? total_bb.GetSize().x + 52.f : -112.f, GetAnimSpeed() / 3);
		it_anim->second.highlight_alpha = 0.8f;

		// Background Opacity Animation
		it_anim->second.bg_alpha = ImLerp(it_anim->second.bg_alpha, hovered || it_anim->second.opened_combo ? 1.f : 0.4f, GetAnimSpeed());

		// Animate shader alpha - visible when combo is open or hovered
		it_anim->second.shader_alpha = ImLerp(it_anim->second.shader_alpha, (it_anim->second.opened_combo || hovered) ? 0.65f : 0.f, GetAnimSpeed());

		if (!hovered)
			it_anim->second.highlight_offset = -112.f;

		// Draw Widget Background (Opaque on hover/open)
		it_anim->second.draw_list->AddRectFilled(total_bb.Min, total_bb.Max, utils::GetColorWithAlpha(c::window_bg_color, it_anim->second.bg_alpha), c::elements::rounding);

		// Draw Shader Background
		shaderrt_v2::Draw_v2(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it_anim->second.shader_alpha, ImShaderTex_WindowBg_v2);

		// Shine effect (blur/shadow/highlight) - enabled with zoom
		it_anim->second.draw_list->AddShadowRect(total_bb.Min, total_bb.Max, ImColor(0.f, 0.f, 0.f, it_anim->second.blur_thinkess), 70.f, ImVec2(0, 0), ImDrawFlags_ShadowCutOutShapeBackground, c::elements::rounding);

		crr::Push(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding);
		it_anim->second.draw_list->AddImage(c::highlight_image, total_bb.Min + ImVec2(it_anim->second.highlight_offset - 32 * it_anim->second.size_scale, -32 * it_anim->second.size_scale), total_bb.Min + ImVec2(it_anim->second.highlight_offset + total_bb.GetSize().y + 32 * it_anim->second.size_scale, total_bb.GetSize().y + 32 * it_anim->second.size_scale), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, it_anim->second.highlight_alpha));
		crr::Pop(it_anim->second.draw_list);

		img_blur::Before(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it_anim->second.blur_thinkess, 0, false);

		ItemBackground(id, total_bb, hovered, it_anim->second.draw_list);

		it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * it_anim->second.size_scale, ImVec2(total_bb.Min.x + 14.f * it_anim->second.size_scale, total_bb.GetCenter().y - CalcTextSize(label).y), GetColorU32(it_anim->second.text), label);

		ImRect descrtiption_bb(ImVec2(total_bb.Min.x + 14.f * it_anim->second.size_scale, total_bb.GetCenter().y), ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y) + CalcTextSize(description));
		ItemDescription(description, id, ImRect{ ImVec2(total_bb.Min.x, descrtiption_bb.Min.y), ImVec2(total_bb.Max.x, descrtiption_bb.Max.y) }, hovered, ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y), it_anim->second.description_col, it_anim->second.draw_list, font::description_font, font::description_font->FontSize * it_anim->second.size_scale);

		PushClipRect(bb.Min, bb.Max, true);
		it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * it_anim->second.size_scale, ImVec2(bb.Max.x - (34.f * it_anim->second.size_scale + CalcTextSize(preview_str).x), utils::center_text(total_bb.Min, total_bb.Max, preview_str).y), utils::GetColorWithAlpha(c::label::active, style.Alpha), preview_str);
		PopClipRect();

		ImRotateStart();
		it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * it_anim->second.size_scale, ImVec2(bb.Max.x - 21 * it_anim->second.size_scale - CalcTextSize(ICON_DOWN_SMALL_LINE).x / 2, bb.GetCenter().y - CalcTextSize(ICON_DOWN_SMALL_LINE).y / 2), c::text::label::regular, ICON_DOWN_SMALL_LINE);
		ImRotateEnd(1.57f * it_anim->second.arrow_roll);

		// Получаем размеры viewport
		ImVec2 viewport_size = ImGui::GetMainViewport()->Size;
		ImVec2 viewport_pos = ImGui::GetMainViewport()->Pos;

		// Проверяем видимость прямоугольника
		if (!IsRectVisible(bb.Min, bb.Max + ImVec2(0, 2))) {
			it_anim->second.opened_combo = false;
			it_anim->second.combo_size = 0.f;
		}

		if (!it_anim->second.opened_combo && it_anim->second.combo_size < 2.f)
			return false;

		// Получаем позицию курсора мыши

		// Вычисляем позицию окна, не выходя за границы viewport
		ImVec2 window_size = ImVec2(bb.GetWidth(), it_anim->second.combo_size);
		// Anchor popup below the combo widget for accurate option picking.
		ImVec2 window_pos = ImVec2(bb.Min.x, bb.Max.y + 2.0f);

		// Сдвиг по оси X, если окно выходит за правую границу
		if (window_pos.x + window_size.x > viewport_pos.x + viewport_size.x) {
			window_pos.x = (viewport_pos.x + viewport_size.x) - window_size.x; // Сдвигаем влево
		}

		// Сдвиг по оси Y, если окно выходит за нижнюю границу
		if (window_pos.y + window_size.y > viewport_pos.y + viewport_size.y) {
			window_pos.y = (viewport_pos.y + viewport_size.y) - window_size.y; // Сдвигаем вверх
		}

		// Сдвиг по оси Y, если окно выходит за верхнюю границу
		if (window_pos.y < viewport_pos.y) {
			window_pos.y = viewport_pos.y; // Сдвигаем вниз
		}

		// Устанавливаем позицию перед открытием окна
		SetNextWindowPos(window_pos, ImGuiCond_Always);

		// Устанавливаем размер окна
		ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;

		// Устанавливаем цвета стиля
		PushStyleColor(ImGuiCol_WindowBg, utils::ImColorToImVec4(c::window_bg_color));
		PushStyleVar(ImGuiStyleVar_WindowRounding, c::elements::rounding);
		PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

		// Открываем окно
		bool ret = Begin(label, NULL, window_flags);

		// img_blur::Before(ImGui::GetForegroundDrawList(), ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), c::elements::rounding * 2, 1.f, 0, false);

		ImGui::GetForegroundDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), utils::GetColorWithAlpha(c::window_bg_color, style.Alpha / 2), c::elements::rounding * 2);

		PopStyleVar(3);
		PopStyleColor(1);

		ImGui::GetForegroundDrawList()->AddRect(GetWindowPos(), GetWindowPos() + GetWindowSize(), GetColorU32(c::child::stroke), c::elements::rounding * 2);

		it_anim->second.hovered = IsWindowHovered();

		if (multi && it_anim->second.hovered && g.IO.MouseClicked[0]) it_anim->second.opened_combo = false;

		return true;
	}

	void EndCombo()
	{
		End();
	}

	void MultiCombo(const char* label, bool variable[], const char* labels[], int count)
	{
		ImGuiContext& g = *GImGui;

		std::string preview = "None";

		for (auto i = 0, j = 0; i < count; i++)
		{
			if (variable[i])
			{
				if (j)
					preview += (", ") + (std::string)labels[i];
				else
					preview = labels[i];

				j++;
			}
		}

		if (BeginCombo(label, "", preview.c_str(), count))
		{
			for (auto i = 0; i < count; i++)
			{
				PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 15));
				PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
				custom::Selectable(labels[i], &variable[i], ImGuiSelectableFlags_DontClosePopups);
				PopStyleVar(2);
			}
			End();
		}

		preview = ("None");
	}

	bool BeginComboPreview()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiComboPreviewData* preview_data = &g.ComboPreviewData;

		if (window->SkipItems || !(g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Visible)) return false;

		IM_ASSERT(g.LastItemData.Rect.Min.x == preview_data->PreviewRect.Min.x && g.LastItemData.Rect.Min.y == preview_data->PreviewRect.Min.y);

		if (!window->ClipRect.Overlaps(preview_data->PreviewRect)) return false;

		preview_data->BackupCursorPos = window->DC.CursorPos;
		preview_data->BackupCursorMaxPos = window->DC.CursorMaxPos;
		preview_data->BackupCursorPosPrevLine = window->DC.CursorPosPrevLine;
		preview_data->BackupPrevLineTextBaseOffset = window->DC.PrevLineTextBaseOffset;
		preview_data->BackupLayout = window->DC.LayoutType;
		window->DC.CursorPos = preview_data->PreviewRect.Min + g.Style.FramePadding;
		window->DC.CursorMaxPos = window->DC.CursorPos;
		window->DC.LayoutType = ImGuiLayoutType_Horizontal;
		window->DC.IsSameLine = false;
		PushClipRect(preview_data->PreviewRect.Min, preview_data->PreviewRect.Max, true);

		return true;
	}

	void EndComboPreview()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiComboPreviewData* preview_data = &g.ComboPreviewData;

		ImDrawList* draw_list = window->DrawList;
		if (window->DC.CursorMaxPos.x < preview_data->PreviewRect.Max.x && window->DC.CursorMaxPos.y < preview_data->PreviewRect.Max.y)
			if (draw_list->CmdBuffer.Size > 1)
			{
				draw_list->_CmdHeader.ClipRect = draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 1].ClipRect = draw_list->CmdBuffer[draw_list->CmdBuffer.Size - 2].ClipRect;
				draw_list->_TryMergeDrawCmds();
			}
		PopClipRect();
		window->DC.CursorPos = preview_data->BackupCursorPos;
		window->DC.CursorMaxPos = ImMax(window->DC.CursorMaxPos, preview_data->BackupCursorMaxPos);
		window->DC.CursorPosPrevLine = preview_data->BackupCursorPosPrevLine;
		window->DC.PrevLineTextBaseOffset = preview_data->BackupPrevLineTextBaseOffset;
		window->DC.LayoutType = preview_data->BackupLayout;
		window->DC.IsSameLine = false;
		preview_data->PreviewRect = ImRect();
	}

	static const char* Items_ArrayGetter(void* data, int idx)
	{
		const char* const* items = (const char* const*)data;
		return items[idx];
	}

	static const char* Items_SingleStringGetter(void* data, int idx)
	{
		const char* items_separated_by_zeros = (const char*)data;
		int items_count = 0;
		const char* p = items_separated_by_zeros;
		while (*p)
		{
			if (idx == items_count)
				break;
			p += strlen(p) + 1;
			items_count++;
		}
		return *p ? p : NULL;
	}

	bool Combo(const char* label, const char* description, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items)
	{
		ImGuiContext& g = *GImGui;

		if (!current_item || items_count <= 0)
			return false;

		// Stale config / struct version skew can leave indices out of range → NULL preview → strlen(NULL) in AddText.
		if (*current_item < 0 || *current_item >= items_count)
			*current_item = 0;

		const char* preview_value = getter(user_data, *current_item);
		if (!preview_value)
			preview_value = "";

		if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
			SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

		if (!BeginCombo(label, description, preview_value, items_count, false, ImGuiComboFlags_None)) return false;

		bool value_changed = false;
		PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 15));
		for (int i = 0; i < items_count; i++)
		{
			const char* item_text = getter(user_data, i);
			if (item_text == NULL)
				item_text = "*Unknown item*";

			PushID(i);
			const bool item_selected = (i == *current_item);
			if (custom::Selectable(item_text, item_selected) && *current_item != i)
			{
				value_changed = true;
				*current_item = i;
			}
			if (item_selected)
				SetItemDefaultFocus();
			PopID();
		}
		PopStyleVar();

		EndCombo();

		if (value_changed)
			MarkItemEdited(g.LastItemData.ID);

		return value_changed;
	}

	bool Combo(const char* label, const char* description, int* current_item, const char* const items[], int items_count, int height_in_items)
	{
		const bool value_changed = Combo(label, description, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
		return value_changed;
	}

	bool Combo(const char* label, const char* description, int* current_item, const char* items_separated_by_zeros, int height_in_items)
	{
		int items_count = 0;
		const char* p = items_separated_by_zeros;
		while (*p)
		{
			p += strlen(p) + 1;
			items_count++;
		}
		bool value_changed = Combo(label, description, current_item, Items_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
		return value_changed;
	}



	struct select_state
	{
		ImVec4 text, background, stroke;
		float circle_radius, text_offset;
	};

	bool Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImGuiID id = window->GetID(label);
		ImVec2 label_size = CalcTextSize(label, NULL, true);
		ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
		ImVec2 pos = window->DC.CursorPos;
		pos.y += window->DC.CurrLineTextBaseOffset;
		ItemSize(size, 0.0f);

		const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
		const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
		const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
		if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth)) size.x = ImMax(label_size.x, max_x - min_x);

		const ImVec2 text_min = pos;
		const ImVec2 text_max(min_x + size.x, pos.y + size.y);

		ImRect bb(min_x, pos.y, text_max.x, text_max.y);
		if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
		{
			const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
			const float spacing_y = style.ItemSpacing.y;
			const float spacing_L = IM_TRUNC(spacing_x * 0.50f);
			const float spacing_U = IM_TRUNC(spacing_y * 0.50f);
			bb.Min.x -= spacing_L;
			bb.Min.y -= spacing_U;
			bb.Max.x += (spacing_x - spacing_L);
			bb.Max.y += (spacing_y - spacing_U);
		}

		const float backup_clip_rect_min_x = window->ClipRect.Min.x;
		const float backup_clip_rect_max_x = window->ClipRect.Max.x;
		if (span_all_columns)
		{
			window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
			window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
		}

		const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
		const bool item_add = ItemAdd(bb, id, NULL, disabled_item ? ImGuiItemFlags_Disabled : ImGuiItemFlags_None);
		if (span_all_columns)
		{
			window->ClipRect.Min.x = backup_clip_rect_min_x;
			window->ClipRect.Max.x = backup_clip_rect_max_x;
		}

		if (!item_add) return false;

		const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
		if (disabled_item && !disabled_global) BeginDisabled();

		if (span_all_columns && window->DC.CurrentColumns) PushColumnsBackground();
		else if (span_all_columns && g.CurrentTable) TablePushBackgroundChannel();

		ImGuiButtonFlags button_flags = 0;
		if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
		if (flags & ImGuiSelectableFlags_NoSetKeyOwner) { button_flags |= ImGuiButtonFlags_NoSetKeyOwner; }
		if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
		if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
		if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
		if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.InFlags & ImGuiItemFlags_AllowOverlap)) { button_flags |= ImGuiButtonFlags_AllowOverlap; }

		const bool was_selected = selected;
		bool hovered, held, pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

		if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
			if (g.NavJustMovedToId == id)  selected = pressed = true;

		// Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
		if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
		{
			if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
			{
				SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, WindowRectAbsToRel(window, bb)); // (bb == NavRect)
				g.NavDisableHighlight = true;
			}
		}
		if (pressed) MarkItemEdited(id);

		if (selected != was_selected)  g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;


		if (g.NavId == id) RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

		if (span_all_columns && window->DC.CurrentColumns) PopColumnsBackground();
		else if (span_all_columns && g.CurrentTable) TablePopBackgroundChannel();

		static std::map<ImGuiID, select_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, select_state() });
			it_anim = anim.find(id);
		}

		it_anim->second.text = ImLerp(it_anim->second.text, selected ? c::label::active : c::label::regular, GetAnimSpeed());
		it_anim->second.circle_radius = ImLerp(it_anim->second.circle_radius, selected ? 3.f : 0.f, GetAnimSpeed());
		it_anim->second.text_offset = ImLerp(it_anim->second.text_offset, selected ? 20.f : 4.5f, GetAnimSpeed());
		it_anim->second.background = ImLerp(it_anim->second.background, selected ? c::background_color : utils::GetColorWithAlpha(c::background_color, 0.f), GetAnimSpeed());
		it_anim->second.stroke = ImLerp(it_anim->second.stroke, selected ? ImColor(1.f, 1.f, 1.f, 0.05f) : ImColor(1.f, 1.f, 1.f, 0.f), GetAnimSpeed());


		ImGui::GetForegroundDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(it_anim->second.stroke), c::elements::rounding);

		ImGui::GetForegroundDrawList()->AddCircleFilled(ImVec2(bb.Min.x + 9, bb.GetCenter().y), it_anim->second.circle_radius, utils::GetColorWithAlpha(c::anim::active, c::anim::active.Value.w * style.Alpha));

		PushStyleColor(ImGuiCol_Text, GetColorU32(it_anim->second.text));
		ImGui::GetForegroundDrawList()->AddText(ImVec2(bb.Min.x + it_anim->second.text_offset, utils::center_text(bb.Min, bb.Max, label).y), utils::GetColorWithAlpha(it_anim->second.text, it_anim->second.text.w * style.Alpha), label);
		PopStyleColor();

		if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup)) CloseCurrentPopup();

		if (disabled_item && !disabled_global) EndDisabled();

		return pressed;
	}

	bool Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
	{
		if (Selectable(label, *p_selected, flags, size_arg))
		{
			*p_selected = !*p_selected;
			return true;
		}
		return false;
	}

	static void ColorEditRestoreH(const float* col, float* H)
	{
		ImGuiContext& g = *GImGui;
		IM_ASSERT(g.ColorEditCurrentID != 0);
		if (g.ColorEditSavedID != g.ColorEditCurrentID || g.ColorEditSavedColor != ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
			return;
		*H = g.ColorEditSavedHue;
	}

	static void ColorEditRestoreHS(const float* col, float* H, float* S, float* V)
	{
		ImGuiContext& g = *GImGui;
		IM_ASSERT(g.ColorEditCurrentID != 0);
		if (g.ColorEditSavedID != g.ColorEditCurrentID || g.ColorEditSavedColor != ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0))) return;

		if (*S == 0.0f || (*H == 0.0f && g.ColorEditSavedHue == 1))
			*H = g.ColorEditSavedHue;

		if (*V == 0.0f) *S = g.ColorEditSavedSat;
	}


	struct edit_state
	{
		ImVec4 text;
		ImVec4 icon;
		ImVec4 text_color, description_col;
		ImVec2 frame_offset;
		ImDrawList* draw_list;
		bool picker_is_open;
		float size_scale = 1.f;
		float blur_thinkess;
		float highlight_offset = -72.f;
		float highlight_alpha;
		float picker_scale;
	};

	bool ColorEdit4(const char* label, const char* description, float col[4], ImGuiColorEditFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const float square_sz = GetFrameHeight();
		const float w_full = CalcItemWidth();
		const float w_button = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
		const float w_inputs = w_full - w_button;
		const char* label_display_end = FindRenderedTextEnd(label);
		g.NextItemData.ClearFlags();

		BeginGroup();
		PushID(label);
		const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
		if (set_current_color_edit_id)
			g.ColorEditCurrentID = window->IDStack.back();

		// If we're not showing any slider there's no point in doing any HSV conversions
		const ImGuiColorEditFlags flags_untouched = flags;
		if (flags & ImGuiColorEditFlags_NoInputs)
			flags = (flags & (~ImGuiColorEditFlags_DisplayMask_)) | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;

		// Context menu: display and modify options (before defaults are applied)
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			ColorEditOptionsPopup(col, flags);

		// Read stored options
		if (!(flags & ImGuiColorEditFlags_DisplayMask_))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DisplayMask_);
		if (!(flags & ImGuiColorEditFlags_DataTypeMask_))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DataTypeMask_);
		if (!(flags & ImGuiColorEditFlags_PickerMask_))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_);
		if (!(flags & ImGuiColorEditFlags_InputMask_))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_InputMask_);
		flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_));
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_DisplayMask_)); // Check that only 1 is selected
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));   // Check that only 1 is selected

		const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
		const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
		const int components = alpha ? 4 : 3;

		// Convert to the formats we need
		float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
		if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
			ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
		else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
		{
			// Hue is lost when converting from grayscale rgb (saturation=0). Restore it.
			ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
			ColorEditRestoreHS(col, &f[0], &f[1], &f[2]);
		}
		int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

		bool value_changed = false;
		bool value_changed_as_float = false;

		const ImVec2 pos = window->DC.CursorPos;
		const float inputs_offset_x = (style.ColorButtonPosition == ImGuiDir_Left) ? w_button : 0.0f;
		window->DC.CursorPos.x = pos.x + inputs_offset_x;

		if ((flags & (ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
		{
			// RGB/HSV 0..255 Sliders
			const float w_item_one = ImMax(1.0f, IM_FLOOR((w_inputs - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
			const float w_item_last = ImMax(1.0f, IM_FLOOR(w_inputs - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));

			const bool hide_prefix = (w_item_one <= CalcTextSize((flags & ImGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
			static const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
			static const char* fmt_table_int[3][4] =
			{
				{   "%3d",   "%3d",   "%3d",   "%3d" }, // Short display
				{ "R:%3d", "G:%3d", "B:%3d", "A:%3d" }, // Long display for RGBA
				{ "H:%3d", "S:%3d", "V:%3d", "A:%3d" }  // Long display for HSVA
			};
			static const char* fmt_table_float[3][4] =
			{
				{   "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" }, // Short display
				{ "R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f" }, // Long display for RGBA
				{ "H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f" }  // Long display for HSVA
			};
			const int fmt_idx = hide_prefix ? 0 : (flags & ImGuiColorEditFlags_DisplayHSV) ? 2 : 1;

			for (int n = 0; n < components; n++)
			{
				if (n > 0)
					SameLine(0, style.ItemInnerSpacing.x);
				SetNextItemWidth((n + 1 < components) ? w_item_one : w_item_last);

				// FIXME: When ImGuiColorEditFlags_HDR flag is passed HS values snap in weird ways when SV values go below 0.
				if (flags & ImGuiColorEditFlags_Float)
				{
					value_changed |= DragFloat(ids[n], &f[n], 1.0f / 255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
					value_changed_as_float |= value_changed;
				}
				else
				{
					value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
				}
				if (!(flags & ImGuiColorEditFlags_NoOptions))
					OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
			}
		}
		else if ((flags & ImGuiColorEditFlags_DisplayHex) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
		{

			if (!(flags & ImGuiColorEditFlags_NoOptions))
				OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
		}

		// RGB Hexadecimal Input
		char buf[64];
		ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255));

		const float width = GetContentRegionAvail().x;


		static std::map<ImGuiID, edit_state> anim;
		auto it_anim = anim.find(ImGui::GetID(label));

		if (it_anim == anim.end())
		{
			anim.insert({ ImGui::GetID(label), edit_state() });
			it_anim = anim.find(ImGui::GetID(label));
		}



		const ImRect frame_bb(pos, pos + ImVec2(width, 55));
		const ImRect rect(pos - it_anim->second.frame_offset, pos + ImVec2(width, 55) + it_anim->second.frame_offset);


		const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);

		ImGuiWindow* picker_active_window = NULL;
		if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
		{
			const float button_offset_x = ((flags & ImGuiColorEditFlags_NoInputs) || (style.ColorButtonPosition == ImGuiDir_Left)) ? 0.0f : w_inputs + style.ItemInnerSpacing.x;
			window->DC.CursorPos = ImVec2(pos.x, pos.y);

			if (IsMouseHoveringRect(rect.Min, rect.Max, true) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				if (!(flags & ImGuiColorEditFlags_NoPicker))
				{
					g.ColorPickerRef = col_v4;
					it_anim->second.picker_is_open = true;
					SetNextWindowPos(main_window_rect.GetTR() + ImVec2(20, 0));
				}
			}
			if (!(flags & ImGuiColorEditFlags_NoOptions))
				OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

			it_anim->second.picker_scale = ImLerp(it_anim->second.picker_scale, it_anim->second.picker_is_open ? 1.f : 0.f, ImGui::GetIO().DeltaTime * 5.f);

			if (it_anim->second.picker_scale > 0.05f)
			{
				PushStyleVar(ImGuiStyleVar_PopupRounding, c::child::rounding);
				PushStyleVar(ImGuiStyleVar_Alpha, it_anim->second.picker_scale);

				if (Begin("picker", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground |
					ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoSavedSettings))
				{
					ImRect bb = ImGui::GetCurrentWindow()->Rect();

					if (it_anim->second.picker_scale > 0.55f) {
						if (!bb.Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						{
							it_anim->second.picker_is_open = false;
						}
					}

					ImGui::PushClipRect(ImVec2(0, 0), ImGui::GetMainViewport()->Size, false);
					// img_blur::Before(ImGui::GetForegroundDrawList(), ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), c::elements::rounding * 2, style.Alpha, 0, false);
					ImGui::GetForegroundDrawList()->AddRectFilled(bb.Min, bb.Max, utils::GetColorWithAlpha(c::window_bg_color, style.Alpha / 3), c::elements::rounding * 2);
					ImGui::GetForegroundDrawList()->AddRect(bb.Min, bb.Max, ImColor(1.f, 1.f, 1.f, style.Alpha * 0.05f), c::elements::rounding * 2);
					ImGui::PopClipRect();

					if (g.CurrentWindow->BeginCount == 1)
					{
						picker_active_window = g.CurrentWindow;

						ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
						ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
						SetNextItemWidth(square_sz * it_anim->second.picker_scale); // Use 256 + bar sizes?
						value_changed |= ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
					}
					End();
				}
				PopStyleVar(2);
			}
		}

		if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
		{


			SameLine(0.0f, style.ItemInnerSpacing.x);
			window->DC.CursorPos.x = pos.x - w_button + ((flags & ImGuiColorEditFlags_NoInputs) ? w_button : w_full);

			const ImVec2 check_offset = ImVec2(16.f, 16.f);

			ImRect check_bb(rect.Max - ImVec2(rect.GetSize().y, rect.GetSize().y) + check_offset, rect.Max - check_offset);

			// Require ColorEdit to be fully visible before allowing hover/zoom
			// Check visibility on base frame_bb (not zoomed rect) to avoid glitches during zoom
			ImRect clipped_rect = frame_bb;
			clipped_rect.ClipWith(window->ClipRect);

			// Check if the entire base widget is visible (not cut off)
			const bool fully_visible = !clipped_rect.IsInverted() &&
				clipped_rect.Min.x <= frame_bb.Min.x + 1.0f &&
				clipped_rect.Min.y <= frame_bb.Min.y + 1.0f &&
				clipped_rect.Max.x >= frame_bb.Max.x - 1.0f &&
				clipped_rect.Max.y >= frame_bb.Max.y - 1.0f;

			bool hovered = fully_visible && ImGui::IsMouseHoveringRect(rect.Min, rect.Max, false);

			//if (ImGui::IsPopupOpen("picker"))
			//	c::is_picker_open = true;
			//else
			//	c::is_picker_open = false;

			it_anim->second.text = ImLerp(it_anim->second.text, ImGui::IsPopupOpen("picker") ? c::text::label::active : IsMouseHoveringRect(rect.Min, rect.Max, true) ? c::text::label::hovered : c::text::label::regular, GetAnimSpeed());

			it_anim->second.icon = ImLerp(it_anim->second.icon, ImGui::IsPopupOpen("picker") ? c::anim::active : IsMouseHoveringRect(rect.Min, rect.Max, true) ? c::text::label::hovered : c::text::label::regular, GetAnimSpeed());

			it_anim->second.draw_list = it_anim->second.size_scale > 1.01f ? GetForegroundDrawList() : GetWindowDrawList();
			it_anim->second.text_color = ImLerp(it_anim->second.text_color, ImGui::IsPopupOpen("picker") ? c::text::label::active : rect.Contains(ImGui::GetMousePos()) ? c::text::label::hovered : c::text::label::regular, GetAnimSpeed());
			it_anim->second.description_col = ImLerp(it_anim->second.description_col, ImGui::IsPopupOpen("picker") ? c::text::description::active : rect.Contains(ImGui::GetMousePos()) ? c::text::description::hovered : c::text::description::regular, GetAnimSpeed());

			it_anim->second.size_scale = ImLerp(it_anim->second.size_scale, hovered || ImGui::IsPopupOpen("picker") ? 1.05f : 1.f, GetAnimSpeed());
			it_anim->second.frame_offset = ImLerp(it_anim->second.frame_offset, hovered || ImGui::IsPopupOpen("picker") ? ImVec2(50 * it_anim->second.size_scale, 12.5f * it_anim->second.size_scale) : ImVec2(0, 0), GetAnimSpeed());
			it_anim->second.blur_thinkess = ImLerp(it_anim->second.blur_thinkess, hovered || ImGui::IsPopupOpen("picker") ? 1.f : 0.f, GetAnimSpeed() * 2);
			it_anim->second.highlight_offset = ImLerp(it_anim->second.highlight_offset, hovered || ImGui::IsPopupOpen("picker") ? rect.GetSize().x + 52.f : -112.f, GetAnimSpeed() / 3);
			it_anim->second.highlight_alpha = 0.8f;

			if (!hovered)
				it_anim->second.highlight_offset = -112.f;



			ImColor color_rgb = col_v4;

			it_anim->second.draw_list->AddShadowRect(rect.Min, rect.Max, ImColor(0.f, 0.f, 0.f, it_anim->second.blur_thinkess), 70.f, ImVec2(0, 0), ImDrawFlags_ShadowCutOutShapeBackground, c::elements::rounding);

			crr::Push(it_anim->second.draw_list, rect.Min, rect.Max, c::elements::rounding);
			it_anim->second.draw_list->AddImage(c::highlight_image, rect.Min + ImVec2(it_anim->second.highlight_offset - 32 * it_anim->second.size_scale, -32 * it_anim->second.size_scale), rect.Min + ImVec2(it_anim->second.highlight_offset + rect.GetSize().y + 32 * it_anim->second.size_scale, rect.GetSize().y + 32 * it_anim->second.size_scale), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, it_anim->second.highlight_alpha));
			crr::Pop(it_anim->second.draw_list);

			img_blur::Before(it_anim->second.draw_list, rect.Min, rect.Max, c::elements::rounding, it_anim->second.blur_thinkess, 0, false);

			ItemBackground(GetID(label), rect, rect.Contains(ImGui::GetMousePos()), it_anim->second.draw_list);

			it_anim->second.draw_list->AddRectFilled(check_bb.Min, check_bb.Max, color_rgb, c::elements::rounding);

			RenderColorRectWithAlphaCheckerboard(it_anim->second.draw_list, check_bb.Min, check_bb.Max, color_rgb, ImMin(36, 29) / 2.99f, ImVec2(0.f, 0.f), c::elements::rounding);

			ImGui::PushClipRect(rect.Min, rect.Max, true);
			it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * it_anim->second.size_scale, ImVec2(rect.Min.x + 14.f * it_anim->second.size_scale, rect.GetCenter().y - CalcTextSize(label).y), GetColorU32(it_anim->second.text_color), label);
			ImGui::PopClipRect();


			ImRect descrtiption_bb(ImVec2(rect.Min.x + 14.f, rect.GetCenter().y), ImVec2(rect.Min.x + 14.f, rect.GetCenter().y) + CalcTextSize(description));
			ItemDescription(description, GetID(label), ImRect{ ImVec2(check_bb.Min.x - 25, descrtiption_bb.Min.y), ImVec2(check_bb.Min.x - 5, descrtiption_bb.Max.y) }, rect.Contains(ImGui::GetMousePos()), ImVec2(rect.Min.x + 14.f, rect.GetCenter().y), it_anim->second.description_col, it_anim->second.draw_list, font::description_font, font::description_font->FontSize);


			ImGui::SetCursorScreenPos(ImVec2(rect.Min.x, rect.Max.y));
		}

		// Convert back
		if (value_changed && picker_active_window == NULL)
		{
			if (!value_changed_as_float)
				for (int n = 0; n < 4; n++)
					f[n] = i[n] / 255.0f;
			if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
			{
				g.ColorEditSavedHue = f[0];
				g.ColorEditSavedSat = f[1];
				ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
				g.ColorEditSavedID = g.ColorEditCurrentID;
				g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(f[0], f[1], f[2], 0));
			}
			if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
				ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

			col[0] = f[0];
			col[1] = f[1];
			col[2] = f[2];
			if (alpha)
				col[3] = f[3];
		}

		if (set_current_color_edit_id)
			g.ColorEditCurrentID = 0;
		PopID();
		EndGroup();

		// Drag and Drop Target
		// NB: The flag test is merely an optional micro-optimization, BeginDragDropTarget() does the same test.
		if ((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) && !(flags & ImGuiColorEditFlags_NoDragDrop) && BeginDragDropTarget())
		{
			bool accepted_drag_drop = false;
			if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
			{
				memcpy((float*)col, payload->Data, sizeof(float) * 3); // Preserve alpha if any //-V512 //-V1086
				value_changed = accepted_drag_drop = true;
			}
			if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
			{
				memcpy((float*)col, payload->Data, sizeof(float) * components);
				value_changed = accepted_drag_drop = true;
			}

			// Drag-drop payloads are always RGB
			if (accepted_drag_drop && (flags & ImGuiColorEditFlags_InputHSV))
				ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);
			EndDragDropTarget();
		}

		// When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
		if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
			g.LastItemData.ID = g.ActiveId;

		if (value_changed && g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup() won't catch g.ActiveId
			MarkItemEdited(g.LastItemData.ID);

		return value_changed;
	}


	// Helper for ColorPicker4()
	static void RenderArrowsForVerticalBar(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, float bar_w, float alpha)
	{
		ImU32 alpha8 = IM_F32_TO_INT8_SAT(alpha);
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x + 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Right, IM_COL32(0, 0, 0, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x, pos.y), half_sz, ImGuiDir_Right, IM_COL32(255, 255, 255, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x - 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Left, IM_COL32(0, 0, 0, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x, pos.y), half_sz, ImGuiDir_Left, IM_COL32(255, 255, 255, alpha8));
	}

	struct picker_state
	{
		float hue_bar;
		float alpha_bar;
		float circle;
		ImVec2 circle_move;
	};

	bool ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImDrawList* draw_list = GetForegroundDrawList();
		ImGuiStyle& style = g.Style;
		ImGuiIO& io = g.IO;

		const float width = CalcItemWidth();
		g.NextItemData.ClearFlags();

		PushID(label);
		BeginGroup();

		if (!(flags & ImGuiColorEditFlags_NoSidePreview))
			flags |= ImGuiColorEditFlags_NoSmallPreview;

		if (!(flags & ImGuiColorEditFlags_NoOptions))
			ColorPickerOptionsPopup(col, flags);

		// Read stored options
		if (!(flags & ImGuiColorEditFlags_PickerMask_))
			flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_PickerMask_;
		if (!(flags & ImGuiColorEditFlags_InputMask_))
			flags |= ((g.ColorEditOptions & ImGuiColorEditFlags_InputMask_) ? g.ColorEditOptions : ImGuiColorEditFlags_DefaultOptions_) & ImGuiColorEditFlags_InputMask_;
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_PickerMask_)); // Check that only 1 is selected
		IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));  // Check that only 1 is selected
		if (!(flags & ImGuiColorEditFlags_NoOptions))
			flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

		// Setup
		int components = (flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4;
		bool alpha_bar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);
		ImVec2 picker_pos = window->DC.CursorPos;
		ImVec2 bar_pos = window->DC.CursorPos + ImVec2(0, 133);
		float square_sz = GetFrameHeight();
		float bars_width = 209.f; // Arbitrary smallish width of Hue/Alpha picking bars
		float sv_picker_size = ImMax(bars_width * 1, width - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x)) + 0; // Saturation/Value picking box
		float sv_bar_size = 20; // Saturation/Value picking box
		float bar0_pos_x = GetWindowPos().x + style.WindowPadding.x;
		float bar1_pos_x = bar0_pos_x;
		float bars_triangles_half_sz = IM_FLOOR(bars_width * 0.20f);

		float backup_initial_col[4];
		memcpy(backup_initial_col, col, components * sizeof(float));

		float H = col[0], S = col[1], V = col[2];
		float R = col[0], G = col[1], B = col[2];
		if (flags & ImGuiColorEditFlags_InputRGB)
		{
			// Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
			ColorConvertRGBtoHSV(R, G, B, H, S, V);
			ColorEditRestoreHS(col, &H, &S, &V);
		}
		else if (flags & ImGuiColorEditFlags_InputHSV)
		{
			ColorConvertHSVtoRGB(H, S, V, R, G, B);
		}

		bool value_changed = false, value_changed_h = false, value_changed_sv = false;

		PushItemFlag(ImGuiItemFlags_NoNav, true);

		// SV rectangle logic
		InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size - 80));
		if (IsItemActive())
		{
			S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
			V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 80));

			// Greatly reduces hue jitter and reset to 0 when hue == 255 and color is rapidly modified using SV square.
			if (g.ColorEditSavedColor == ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
				H = g.ColorEditSavedHue;
			value_changed = value_changed_sv = true;
		}

		// Hue bar logic
		SetCursorScreenPos(ImVec2(bar0_pos_x, bar_pos.y));
		InvisibleButton("hue", ImVec2(bars_width, sv_bar_size));
		if (IsItemActive())
		{
			H = 1.f - ImSaturate((io.MousePos.x - bar_pos.x) / (bars_width - 1));
			value_changed = value_changed_h = true;
		}

		// Alpha bar logic
		if (alpha_bar)
		{
			SetCursorScreenPos(ImVec2(bar1_pos_x, bar_pos.y + 16));
			InvisibleButton("alpha", ImVec2(bars_width, sv_bar_size));
			if (IsItemActive())
			{
				col[3] = ImSaturate((io.MousePos.x - bar_pos.x) / (bars_width - 1));
				value_changed = true;
			}
		}
		PopItemFlag(); // ImGuiItemFlags_NoNav

		// Convert back color to RGB
		if (value_changed_h || value_changed_sv)
		{
			if (flags & ImGuiColorEditFlags_InputRGB)
			{
				ColorConvertHSVtoRGB(H, S, V, col[0], col[1], col[2]);
				g.ColorEditSavedHue = H;
				g.ColorEditSavedSat = S;
				g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0));
			}

			else if (flags & ImGuiColorEditFlags_InputHSV)
			{
				col[0] = H;
				col[1] = S;
				col[2] = V;
			}
		}

		// R,G,B and H,S,V slider color editor
		bool value_changed_fix_hue_wrap = false;

		if (value_changed_fix_hue_wrap && (flags & ImGuiColorEditFlags_InputRGB))
		{
			// Try to cancel hue wrap (after ColorEdit4 call), if any
			float new_H, new_S, new_V;
			ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
			if (new_H <= 0 && H > 0)
			{
				if (new_V <= 0 && V != new_V)
					ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
				else if (new_S <= 0)
					ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
			}
		}

		if (value_changed)
		{
			if (flags & ImGuiColorEditFlags_InputRGB)
			{
				R = col[0];
				G = col[1];
				B = col[2];
				ColorConvertRGBtoHSV(R, G, B, H, S, V);
				ColorEditRestoreHS(col, &H, &S, &V);   // Fix local Hue as display below will use it immediately.
			}
			else if (flags & ImGuiColorEditFlags_InputHSV)
			{
				H = col[0];
				S = col[1];
				V = col[2];
				ColorConvertHSVtoRGB(H, S, V, R, G, B);
			}
		}
		ImU32 user_col32_striped_of_alpha = ColorConvertFloat4ToU32(ImVec4(R, G, B, style.Alpha)); // Important: this is still including the main rendering/style alpha!!

		const int style_alpha8 = IM_F32_TO_INT8_SAT(style.Alpha);
		const ImU32 col_black = IM_COL32(0, 0, 0, style_alpha8);
		const ImU32 col_white = IM_COL32(255, 255, 255, style_alpha8);
		const ImU32 col_midgrey = IM_COL32(128, 128, 128, style_alpha8);
		const ImU32 col_hues[7] = { IM_COL32(255,0,0,style_alpha8), IM_COL32(255,0,255,style_alpha8), IM_COL32(0,0,255,style_alpha8),IM_COL32(0,255,255,style_alpha8), IM_COL32(0,255,0,style_alpha8), IM_COL32(255,255,0,style_alpha8), IM_COL32(255,0,0,style_alpha8) };

		ImVec4 hue_color_f(1, 1, 1, style.Alpha); ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
		ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);

		ImVec2 sv_cursor_pos;

		// Render SV Square
		const int vtx_idx_0 = draw_list->VtxBuffer.Size;
		draw_list->AddRectFilled(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size - 2 - 80), col_white, 4.0f);
		const int vtx_idx_1 = draw_list->VtxBuffer.Size;
		ShadeVertsLinearColorGradientKeepAlpha(draw_list, vtx_idx_0, vtx_idx_1, picker_pos, picker_pos + ImVec2(sv_picker_size, 0.0f), col_white, hue_color32);

		draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size - 80), 0, 0, col_black, col_black, 4.f);

		sv_cursor_pos.x = ImClamp(IM_ROUND(picker_pos.x + ImSaturate(S) * sv_picker_size), picker_pos.x, picker_pos.x + sv_picker_size - 2); // Sneakily prevent the circle to stick out too much
		sv_cursor_pos.y = ImClamp(IM_ROUND(picker_pos.y + ImSaturate(1 - V) * (sv_picker_size - 80)), picker_pos.y + 2, picker_pos.y + sv_picker_size - 80);

		static std::map<ImGuiID, picker_state> anim;
		auto it_anim = anim.find(ImGui::GetID(label));

		if (it_anim == anim.end())
		{
			anim.insert({ ImGui::GetID(label), picker_state() });
			it_anim = anim.find(ImGui::GetID(label));
		}

		for (int i = 0; i < 6; ++i)
			GetForegroundDrawList()->AddRectFilledMultiColor(ImVec2(picker_pos.x + i * (bars_width / 6) - (i == 5 ? 1 : 0), picker_pos.y + 139), ImVec2(picker_pos.x + (i + 1) * (bars_width / 6) + (i == 0 ? 1 : 0), picker_pos.y + 132 + sv_bar_size - 7), col_hues[i], col_hues[i + 1], col_hues[i + 1], col_hues[i], 10.f, i == 0 ? ImDrawFlags_RoundCornersLeft : i == 5 ? ImDrawFlags_RoundCornersRight : ImDrawFlags_RoundCornersNone);

		float bar0_line_x = IM_ROUND(bar_pos.x + (1.f - H) * bars_width);

		bar0_line_x = ImClamp(bar0_line_x, picker_pos.x + 3.f, picker_pos.x + 204.f);

		it_anim->second.hue_bar = ImLerp(it_anim->second.hue_bar, bar0_line_x - bar_pos.x, g.IO.DeltaTime * 24.f);

		GetForegroundDrawList()->AddCircleFilled(ImVec2(it_anim->second.hue_bar + bar_pos.x, bar_pos.y + 9), 6.5f, ImColor(255, 255, 255, int(255 * style.Alpha)), 30.f);

		it_anim->second.circle_move = ImLerp(it_anim->second.circle_move, sv_cursor_pos - bar_pos, g.IO.DeltaTime * 24.f);
		it_anim->second.circle = ImLerp(it_anim->second.circle, value_changed_sv ? 4.f : 7.f, g.IO.DeltaTime * 24.f);

		GetForegroundDrawList()->AddCircle(it_anim->second.circle_move + bar_pos + ImVec2(0, 1), it_anim->second.circle, ImColor(255, 255, 255, int(255 * style.Alpha)), 32);

		if (alpha_bar)
		{
			float alpha = ImSaturate(col[3]);
			ImRect bar1_bb(bar1_pos_x, bar_pos.y + 20, bar1_pos_x + bars_width, bar_pos.y + 20 + sv_bar_size);

			draw_list->AddRectFilledMultiColor(picker_pos + ImVec2(0, 161), picker_pos + ImVec2(bars_width, 147 + sv_bar_size), col_black, user_col32_striped_of_alpha, user_col32_striped_of_alpha, col_black, 10.f);

			float bar1_line_x = IM_ROUND(bar_pos.x + alpha * bars_width);

			bar1_line_x = ImClamp(bar1_line_x, bar_pos.x, picker_pos.x + 200.f);
			it_anim->second.alpha_bar = ImLerp(it_anim->second.alpha_bar, bar1_line_x - bar_pos.x + 5.f, g.IO.DeltaTime * 24.f);
			GetForegroundDrawList()->AddCircleFilled(ImVec2(it_anim->second.alpha_bar + bar_pos.x, bar1_bb.Min.y + 11.0f), 6.5f, ImColor(255, 255, 255, int(255 * style.Alpha)), 100.f);
		}

		EndGroup();

		if (value_changed && memcmp(backup_initial_col, col, components * sizeof(float)) == 0) value_changed = false;
		if (value_changed) MarkItemEdited(g.LastItemData.ID);

		PopID();
		return value_changed;
	}

	bool ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags, const ImVec2& size_arg)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiID id = window->GetID(desc_id);
		const float default_size = GetFrameHeight();
		const ImVec2 pos = window->DC.CursorPos;
		const float width = GetContentRegionMax().x - ImGui::GetStyle().WindowPadding.x;
		const ImRect rect(pos, pos + ImVec2(width, 19));

		const ImRect clickable(rect.Min + ImVec2(width - 25, 0), rect.Max - ImVec2(7, 0));

		ItemSize(ImRect(rect.Min, rect.Max - ImVec2(0, 0)));
		if (!ItemAdd(rect, id)) return false;

		bool hovered, held, pressed = ButtonBehavior(rect, id, &hovered, &held);

		if (flags & ImGuiColorEditFlags_NoAlpha) flags &= ~(ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf);

		ImVec4 col_rgb = col;
		if (flags & ImGuiColorEditFlags_InputHSV) ColorConvertHSVtoRGB(col_rgb.x, col_rgb.y, col_rgb.z, col_rgb.x, col_rgb.y, col_rgb.z);

		GetWindowDrawList()->AddRectFilled(clickable.Min, clickable.Max, GetColorU32(col_rgb), c::elements::rounding);

		RenderColorRectWithAlphaCheckerboard(window->DrawList, clickable.Min, clickable.Max, GetColorU32(col_rgb), ImMin(36, 29) / 2.99f, ImVec2(0.f, 0.f), c::elements::rounding);


		return pressed;
	}

	struct knob_state {
		float plus_float;
		int plus_int;
		ImVec4 background, circle, text;
		float slow_anim, circle_anim;
		float position;
	};


	bool KnobScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const float w = GetContentRegionMax().x - style.WindowPadding.x;

		const ImVec2 label_size = CalcTextSize(label, NULL, true);

		const ImRect frame_bb(window->DC.CursorPos + ImVec2(0, 0), window->DC.CursorPos + ImVec2(w, 32));

		const ImRect slider_bb(window->DC.CursorPos + ImVec2(w - 30, 0), window->DC.CursorPos + ImVec2(w, 100));

		const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? label_size.x : 0.0f, 0.0f));

		const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
		ItemSize(ImRect(total_bb.Min, total_bb.Max - ImVec2(0, 0)));

		if (!ItemAdd(total_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0)) return false;

		if (format == NULL) format = DataTypeGetInfo(data_type)->PrintFmt;

		bool hovered = ItemHoverable(frame_bb, id, g.LastItemData.InFlags), held, pressed = ButtonBehavior(frame_bb, id, &hovered, &held, NULL);

		ImRect grab_bb;

		static std::map<ImGuiID, knob_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, knob_state() });
			it_anim = anim.find(id);
		}

		it_anim->second.circle_anim = ImLerp(it_anim->second.circle_anim, IsItemActive() ? 11.f : 10.f, g.IO.DeltaTime * 6.f);

		if ((flags & ImGuiSliderFlags_Integer) == 0) {
			it_anim->second.plus_float = ImLerp(it_anim->second.plus_float, *(float*)p_data <= *(float*)p_max && hovered && GetAsyncKeyState(VK_OEM_PLUS) & 0x01 ? *(float*)p_data += 0.05f : *(float*)p_data >= *(float*)p_min && hovered && GetAsyncKeyState(VK_OEM_MINUS) & 0x01 ? *(float*)p_data -= 0.05f : 0, g.IO.DeltaTime * 6.f);
			if (*(float*)p_data > *(float*)p_max) *(float*)p_data = *(float*)p_max;
			if (*(float*)p_data < *(float*)p_min) *(float*)p_data = *(float*)p_min;
		}
		else
		{
			it_anim->second.plus_int = ImLerp(it_anim->second.plus_int, *(int*)p_data <= *(int*)p_max && hovered && GetAsyncKeyState(VK_OEM_PLUS) & 0x01 ? *(int*)p_data += 1 : *(int*)p_data >= *(int*)p_min && hovered && GetAsyncKeyState(VK_OEM_MINUS) & 0x01 ? *(int*)p_data -= 1 : 0, g.IO.DeltaTime * 6.f);
			if (*(int*)p_data > *(int*)p_max) *(int*)p_data = *(int*)p_max;
			if (*(int*)p_data < *(int*)p_min) *(int*)p_data = *(int*)p_min;
		}

		it_anim->second.text = ImLerp(it_anim->second.text, g.ActiveId == id ? c::text::label::active : hovered ? c::text::label::hovered : c::text::label::regular, g.IO.DeltaTime * 6.f);

		const bool value_changed = DragBehavior(id, data_type, p_data, 0.f, p_min, p_max, format, NULL);


		if (value_changed) MarkItemEdited(id);

		char value_buf[64];
		const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

		float radius = 10.f;
		float thickness = 3.f;

		it_anim->second.position = ImLerp(it_anim->second.position, *static_cast<float*>(p_data) / *reinterpret_cast<const float*>(p_max) * 6.25f, ImGui::GetIO().DeltaTime * 18.f);

		GetWindowDrawList()->PathClear();
		GetWindowDrawList()->PathArcTo(ImVec2(frame_bb.Max.x + radius - 22.f, frame_bb.Min.y + (32 / 2)), radius, 0.f, 2.f * IM_PI, 40.f);
		GetWindowDrawList()->PathStroke(GetColorU32(c::elements::background), 0, thickness);

		GetWindowDrawList()->PathClear();
		GetWindowDrawList()->PathArcTo(ImVec2(frame_bb.Max.x + radius - 22.f, frame_bb.Min.y + (32 / 2)), radius, IM_PI * 1.5f, IM_PI * 1.5f + it_anim->second.position, 40.f);
		GetWindowDrawList()->PathStroke(GetColorU32(c::accent), 0, thickness);

		GetWindowDrawList()->AddCircleFilled(ImVec2(frame_bb.Max.x + radius - 22.f + ImCos(IM_PI * 1.5f + it_anim->second.position) * radius, frame_bb.Min.y + (32 / 2) + ImSin(IM_PI * 1.5f + it_anim->second.position) * radius), 2.f, c::label::active);

		GetWindowDrawList()->AddText(ImVec2(frame_bb.Max.x - (40 + CalcTextSize(value_buf).x), frame_bb.Min.y + (32 - CalcTextSize(value_buf).y) / 2), c::text::label::regular, value_buf);

		GetWindowDrawList()->AddText(ImVec2(frame_bb.Max.x - w, frame_bb.Min.y + (32 - CalcTextSize(value_buf).y) / 2), GetColorU32(it_anim->second.text), label);

		return value_changed;
	}

	bool KnobFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
	{
		return KnobScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
	}

	bool KnobInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
	{
		return KnobScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags | ImGuiSliderFlags_Integer);
	}


	struct slider_state {
		ImVec4 background, circle, text_color;;
		float position, slow, circle_radius, glow_thnikess;
		ImDrawList* draw_list;
		ImVec2 frame_offset;
		float size_scale = 1.f;
		float blur_thinkess;
		float highlight_offset = -72.f;
		float highlight_alpha;
		float slider_offset;
		float shader_alpha = 0.f;  // For shaderrt_v2::Draw_v2 background animation
		float bg_alpha = 0.4f;
		ImVec4 description_col;
	};


	bool SliderScalar(const char* label, const char* description, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const float w = GetContentRegionMax().x - style.WindowPadding.x;

		const ImVec2 label_size = CalcTextSize(label, NULL, true);
		const ImVec2 pos = window->DC.CursorPos;

		const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, 55.f));

		static std::map<ImGuiID, slider_state> anim;
		auto it_anim = anim.find(id);

		if (it_anim == anim.end())
		{
			anim.insert({ id, slider_state() });
			it_anim = anim.find(id);
		}

		const ImRect total_bb(frame_bb.Min - it_anim->second.frame_offset, frame_bb.Max + it_anim->second.frame_offset);

		const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
		ItemSize(ImRect(frame_bb.Min, frame_bb.Max));

		if (!ItemAdd(frame_bb, id, &frame_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0)) return false;

		if (format == NULL) format = DataTypeGetInfo(data_type)->PrintFmt;

		// Require slider to be fully visible before allowing hover/zoom
		ImRect clipped_bb = frame_bb;
		clipped_bb.ClipWith(window->ClipRect);

		bool hovered = false;
		bool held = false;
		bool pressed = false;

		const bool fully_visible = !clipped_bb.IsInverted() &&
			clipped_bb.Min.x <= frame_bb.Min.x + 1.0f &&
			clipped_bb.Min.y <= frame_bb.Min.y + 1.0f &&
			clipped_bb.Max.x >= frame_bb.Max.x - 1.0f &&
			clipped_bb.Max.y >= frame_bb.Max.y - 1.0f;

		if (fully_visible)
		{
			hovered = ImGui::IsMouseHoveringRect(total_bb.Min, total_bb.Max, false);
			pressed = ButtonBehavior(total_bb, id, nullptr, &held, NULL);
		}

		ImRect grab_bb;

		char value_buf[64];
		const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

		it_anim->second.draw_list = it_anim->second.size_scale > 1.01f ? GetForegroundDrawList() : GetWindowDrawList();

		// Animate hover effects
		it_anim->second.size_scale = ImLerp(it_anim->second.size_scale, hovered ? 1.05f : 1.f, GetAnimSpeed());
		it_anim->second.frame_offset = ImLerp(it_anim->second.frame_offset, hovered ? ImVec2(40 * it_anim->second.size_scale, 12.5f * it_anim->second.size_scale) : ImVec2(0, 0), GetAnimSpeed());
		it_anim->second.blur_thinkess = ImLerp(it_anim->second.blur_thinkess, hovered ? 1.f : 0.f, GetAnimSpeed() * 2);
		it_anim->second.highlight_offset = ImLerp(it_anim->second.highlight_offset, hovered ? total_bb.GetSize().x + 52.f : -112.f, GetAnimSpeed() / 2);
		it_anim->second.highlight_alpha = 0.8f;

		// Background Opacity Animation
		it_anim->second.bg_alpha = ImLerp(it_anim->second.bg_alpha, hovered ? 1.f : 0.4f, GetAnimSpeed());

		if (!hovered)
			it_anim->second.highlight_offset = -112.f;

		// Draw Widget Background (Opaque on hover)
		it_anim->second.draw_list->AddRectFilled(total_bb.Min, total_bb.Max, utils::GetColorWithAlpha(c::window_bg_color, it_anim->second.bg_alpha), c::elements::rounding);

		// Shine effect (blur/shadow/highlight) - enabled with zoom
		it_anim->second.draw_list->AddShadowRect(total_bb.Min, total_bb.Max, ImColor(0.f, 0.f, 0.f, it_anim->second.blur_thinkess), 70.f, ImVec2(0, 0), ImDrawFlags_ShadowCutOutShapeBackground, c::elements::rounding);

		crr::Push(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding);
		it_anim->second.draw_list->AddImage(c::highlight_image, total_bb.Min + ImVec2(it_anim->second.highlight_offset - 32 * it_anim->second.size_scale, -32 * it_anim->second.size_scale), total_bb.Min + ImVec2(it_anim->second.highlight_offset + total_bb.GetSize().y + 32 * it_anim->second.size_scale, total_bb.GetSize().y + 32 * it_anim->second.size_scale), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, it_anim->second.highlight_alpha));
		crr::Pop(it_anim->second.draw_list);

		img_blur::Before(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it_anim->second.blur_thinkess, 0, false);

		it_anim->second.text_color = ImLerp(it_anim->second.text_color, g.ActiveId == id || hovered ? c::text::label::active : c::text::label::regular, GetAnimSpeed());

		// Use total_bb for SliderBehavior to match the visual (zoomed) area
		const bool value_changed = SliderBehavior(total_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);

		if (value_changed) {
			MarkItemEdited(id);
		}

		// Calculate fraction from value to determine visual fill
		float fraction = 0.f;
		if (data_type == ImGuiDataType_Float)
			fraction = (*(float*)p_data - *(float*)p_min) / (*(float*)p_max - *(float*)p_min);
		else if (data_type == ImGuiDataType_S32)
			fraction = (float)(*(int*)p_data - *(int*)p_min) / (float)(*(int*)p_max - *(int*)p_min);

		// Animate slider position smoothly based on value fraction
		float target_slow = total_bb.GetWidth() * fraction;
		it_anim->second.slow = ImLerp(it_anim->second.slow, target_slow, g.IO.DeltaTime * 25.f);

		// Calculate the actual fill position
		float fill_x = total_bb.Min.x + it_anim->second.slow;
		// Clamp to ensure it doesn't go beyond bounds
		fill_x = ImClamp(fill_x, total_bb.Min.x, total_bb.Max.x);

		// Animate shader alpha - visible when slider has value (based on fill position relative to total_bb)
		it_anim->second.shader_alpha = ImLerp(it_anim->second.shader_alpha, it_anim->second.slow > 5.f ? 0.65f : 0.f, GetAnimSpeed());

		// Clip shader effect to only show on filled portion of slider
		PushClipRect(total_bb.Min, ImVec2(fill_x, total_bb.Max.y), true);
		shaderrt_v2::Draw_v2(it_anim->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it_anim->second.shader_alpha, ImShaderTex_WindowBg_v2);
		PopClipRect();

		// Update grab_bb to match the animated slider position (vertical line)
		grab_bb = ImRect(ImVec2(fill_x - 1.f, total_bb.Min.y), ImVec2(fill_x + 1.f, total_bb.Max.y));

		ItemBackground(id, total_bb, hovered, it_anim->second.draw_list);

		// Text clipping effect - label appears in different color on filled vs unfilled portion
		PushClipRect(grab_bb.Min, total_bb.Max, true);
		it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * it_anim->second.size_scale, ImVec2(total_bb.Min.x + 14.f * it_anim->second.size_scale, total_bb.GetCenter().y - CalcTextSize(label).y), GetColorU32(it_anim->second.text_color), label);
		PopClipRect();

		PushClipRect(total_bb.Min, grab_bb.Max, true);
		it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * it_anim->second.size_scale, ImVec2(total_bb.Min.x + 14.f * it_anim->second.size_scale, total_bb.GetCenter().y - CalcTextSize(label).y), c::text::label::active, label);
		PopClipRect();

		// Draw value text
		ImRect value_bb = ImRect(total_bb.Max - ImVec2(20.5f + CalcTextSize(value_buf).x, 49), total_bb.Max - ImVec2(10.5f, 26));
		it_anim->second.draw_list->AddText(ImGui::GetDefaultFont(), ImGui::GetDefaultFont()->FontSize * it_anim->second.size_scale, ImVec2(value_bb.GetCenter().x - CalcTextSize(value_buf).x / 2, utils::center_text(total_bb.Min, total_bb.Max, value_buf).y), GetColorU32(it_anim->second.text_color), value_buf, value_buf_end);

		// Draw filled portion (vertical line at slider position)
		it_anim->second.draw_list->AddShadowRect(grab_bb.Min, grab_bb.Max, c::anim::active, 45.f, ImVec2(0, 0));
		it_anim->second.draw_list->AddRectFilled(grab_bb.Min, grab_bb.Max, c::anim::active, style.FrameRounding);

		it_anim->second.description_col = ImLerp(it_anim->second.description_col, hovered ? c::text::description::hovered : c::text::description::regular, GetAnimSpeed());

		ImRect descrtiption_bb(ImVec2(total_bb.Min.x + 14.f * it_anim->second.size_scale, total_bb.GetCenter().y), ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y) + CalcTextSize(description));
		ItemDescription(description, id, ImRect{ ImVec2(total_bb.Min.x, descrtiption_bb.Min.y), ImVec2(total_bb.Max.x, descrtiption_bb.Max.y) }, hovered, ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y), it_anim->second.description_col, it_anim->second.draw_list, font::description_font, font::description_font->FontSize * it_anim->second.size_scale);

		return value_changed;
	}

	bool SliderFloat(const char* label, const char* description, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
	{
		return SliderScalar(label, description, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
	}

	bool SliderInt(const char* label, const char* description, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
	{
		return SliderScalar(label, description, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
	}
	static bool s_any_combo_or_picker_open = false;
	static int s_any_popup_last_frame = -1;
	static void UpdateAnyComboOrPickerOpen(bool popup_is_open) {
		ImGuiContext& g = *GImGui;
		if (g.FrameCount != s_any_popup_last_frame) {
			s_any_popup_last_frame = g.FrameCount;
			s_any_combo_or_picker_open = false;
		}
		if (popup_is_open) s_any_combo_or_picker_open = true;
	}
	static void ClearAnyComboOrPickerOpen() {
		s_any_combo_or_picker_open = false;
	}
	static bool IsAnyComboOrPickerOpen() {
		ImGuiContext& g = *GImGui;
		if (g.FrameCount != s_any_popup_last_frame) {
			s_any_popup_last_frame = g.FrameCount;
			s_any_combo_or_picker_open = false;
		}
		return s_any_combo_or_picker_open;
	}
	struct checkbox_color_state
	{
		float checkbox_alpha;
		ImVec4 rect_color;
		ImVec4 text_color, description_col;
		ImVec2 frame_offset;
		ImDrawList* draw_list;
		float size_scale = 1.f;
		float blur_thinkess;
		float highlight_offset = -72.f;
		float highlight_alpha;
		float shader_alpha = 0.f;
		float bg_alpha = 0.4f;
		bool picker_is_open = false;
		float picker_scale = 0.f;
	};

	bool ColorBox(const char* label, const char* description, bool* v, float col[4], ImGuiColorEditFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		BeginGroup();
		PushID(label);
		const ImGuiID id = window->GetID(label);
		const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
		if (set_current_color_edit_id)
			g.ColorEditCurrentID = window->IDStack.back();

		const float w = GetContentRegionAvail().x;
		const ImVec2 pos = window->DC.CursorPos;
		const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
		const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);

		static std::map<ImGuiID, checkbox_color_state> anim;
		auto it = anim.emplace(id, checkbox_color_state()).first;

		const ImRect frame_bb(pos, pos + ImVec2(w, 55));
		const ImRect total_bb(pos - it->second.frame_offset, pos + ImVec2(w, 55) + it->second.frame_offset);

		ItemSize(frame_bb, 0.f);
		if (!ItemAdd(frame_bb, id)) {
			if (set_current_color_edit_id) g.ColorEditCurrentID = 0;
			PopID();
			EndGroup();
			return false;
		}

		ImRect clipped_bb = frame_bb;
		clipped_bb.ClipWith(window->ClipRect);
		const bool fully_visible = !clipped_bb.IsInverted() &&
			clipped_bb.Min.x <= frame_bb.Min.x + 1.0f && clipped_bb.Min.y <= frame_bb.Min.y + 1.0f &&
			clipped_bb.Max.x >= frame_bb.Max.x - 1.0f && clipped_bb.Max.y >= frame_bb.Max.y - 1.0f;

		bool hovered = fully_visible && ImGui::IsMouseHoveringRect(total_bb.Min, total_bb.Max, false) && !IsAnyComboOrPickerOpen();

		const float square_sz = 10.f;
		ImVec2 check_offset(15.f * it->second.size_scale, 15.f * it->second.size_scale);
		ImRect check_rect(total_bb.Max - ImVec2(total_bb.GetSize().y, total_bb.GetSize().y), total_bb.Max);
		ImRect check_bb(check_rect.GetCenter() - check_offset, check_rect.GetCenter() + check_offset);
		// Same small color square as ColorEdit4 (match check_offset 16 + square like ColorEdit4)
		const float color_square_sz = total_bb.GetSize().y - 32.f; // 55 - 32 = 23px like ColorEdit4
		const float color_y = total_bb.GetCenter().y - color_square_sz * 0.5f;
		// Layout: [label] ... [color sq] [gap] [checkbox]  (color to the left of checkbox)
		ImRect color_bb(ImVec2(check_bb.Min.x - color_square_sz - 8.f, color_y), ImVec2(check_bb.Min.x - 8.f, color_y + color_square_sz));

		bool checkbox_pressed = false;
		if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			ImVec2 mousePos = ImGui::GetMousePos();
			if (color_bb.Contains(mousePos) && !(flags & ImGuiColorEditFlags_NoPicker)) {
				g.ColorPickerRef = col_v4;
				it->second.picker_is_open = true;
				SetNextWindowPos(ImVec2(color_bb.Max.x + 4.f, color_bb.Min.y), ImGuiCond_FirstUseEver);
				SetNextWindowSize(ImVec2(280, 380), ImGuiCond_FirstUseEver);
			}
			else {
				// Click anywhere else on the row toggles the checkbox (same as normal checkbox)
				*v = !(*v);
				checkbox_pressed = true;
			}
		}

		UpdateAnyComboOrPickerOpen(it->second.picker_is_open);
		it->second.picker_scale = ImLerp(it->second.picker_scale, it->second.picker_is_open ? 1.f : 0.f, GetIO().DeltaTime * 5.f);
		if (it->second.picker_scale > 0.05f)
		{
			char picker_id[64];
			ImFormatString(picker_id, IM_ARRAYSIZE(picker_id), "##cbcolor_%u", id);
			PushStyleVar(ImGuiStyleVar_PopupRounding, c::child::rounding);
			PushStyleVar(ImGuiStyleVar_Alpha, it->second.picker_scale);
			if (Begin(picker_id, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
			{
				ImRect bb = GetCurrentWindow()->Rect();
				if (it->second.picker_scale > 0.55f && !bb.Contains(GetMousePos()) && IsMouseClicked(ImGuiMouseButton_Left))
					it->second.picker_is_open = false;
				PushClipRect(ImVec2(0, 0), GetMainViewport()->Size, false);
				GetForegroundDrawList()->AddRectFilled(bb.Min, bb.Max, utils::GetColorWithAlpha(c::window_bg_color, style.Alpha / 3), c::elements::rounding * 2);
				GetForegroundDrawList()->AddRect(bb.Min, bb.Max, ImColor(1.f, 1.f, 1.f, style.Alpha * 0.05f), c::elements::rounding * 2);
				PopClipRect();
				if (g.CurrentWindow->BeginCount == 1) {
					ImGuiColorEditFlags picker_flags = (flags & (ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar)) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
					SetNextItemWidth(260.f);
					ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
				}
				End();
			}
			PopStyleVar(2);
		}
		if (set_current_color_edit_id)
			g.ColorEditCurrentID = 0;

		it->second.shader_alpha = ImLerp(it->second.shader_alpha, *v ? 0.65f : 0.f, GetAnimSpeed());
		it->second.checkbox_alpha = ImLerp(it->second.checkbox_alpha, *v ? 1.f : 0.f, GetAnimSpeed());
		it->second.rect_color = ImLerp(it->second.rect_color, *v ? c::anim::active : utils::GetColorWithAlpha(c::anim::active, 0.f), GetAnimSpeed());
		it->second.text_color = ImLerp(it->second.text_color, *v ? c::anim::active : hovered ? c::text::label::hovered : c::text::label::regular, GetAnimSpeed());
		it->second.description_col = ImLerp(it->second.description_col, *v ? c::text::description::active : hovered ? c::text::description::hovered : c::text::description::regular, GetAnimSpeed());
		bool allow_zoom = (hovered || it->second.picker_is_open) && !IsAnyComboOrPickerOpen();
		// When picker is open use window draw list so checkbox row doesn't draw on top of the color popup
		it->second.draw_list = (it->second.size_scale > 1.01f && !it->second.picker_is_open) ? GetForegroundDrawList() : GetWindowDrawList();
		it->second.size_scale = ImLerp(it->second.size_scale, allow_zoom ? 1.05f : 1.f, GetAnimSpeed());
		it->second.frame_offset = ImLerp(it->second.frame_offset, allow_zoom ? ImVec2(50 * it->second.size_scale, 12.5f * it->second.size_scale) : ImVec2(0, 0), GetAnimSpeed());
		it->second.blur_thinkess = ImLerp(it->second.blur_thinkess, allow_zoom ? 1.f : 0.f, GetAnimSpeed() * 2);
		it->second.highlight_offset = ImLerp(it->second.highlight_offset, allow_zoom ? total_bb.GetSize().x + 52.f : -112.f, GetAnimSpeed() / 3);
		it->second.highlight_alpha = 0.8f;
		it->second.bg_alpha = ImLerp(it->second.bg_alpha, allow_zoom ? 1.f : 0.4f, GetAnimSpeed());
		if (!allow_zoom) it->second.highlight_offset = -112.f;

		it->second.draw_list->AddRectFilled(total_bb.Min, total_bb.Max, utils::GetColorWithAlpha(c::window_bg_color, it->second.bg_alpha), c::elements::rounding);
		shaderrt_v2::Draw_v2(it->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it->second.shader_alpha, ImShaderTex_WindowBg_v2);
		it->second.draw_list->AddShadowRect(total_bb.Min, total_bb.Max, ImColor(0.f, 0.f, 0.f, it->second.blur_thinkess), 70.f, ImVec2(0, 0), ImDrawFlags_ShadowCutOutShapeBackground, c::elements::rounding);
		crr::Push(it->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding);
		it->second.draw_list->AddImage(c::highlight_image, total_bb.Min + ImVec2(it->second.highlight_offset - 32 * it->second.size_scale, -32 * it->second.size_scale), total_bb.Min + ImVec2(it->second.highlight_offset + total_bb.GetSize().y + 32 * it->second.size_scale, total_bb.GetSize().y + 32 * it->second.size_scale), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, it->second.highlight_alpha));
		crr::Pop(it->second.draw_list);
		ItemBackground(id, total_bb, hovered, it->second.draw_list);

		it->second.draw_list->AddText(GetDefaultFont(), GetDefaultFont()->FontSize * it->second.size_scale, ImVec2(total_bb.Min.x + 14.f * it->second.size_scale, total_bb.GetCenter().y - CalcTextSize(label).y), GetColorU32(it->second.text_color), label);
		ImRect desc_bb(ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y), ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y) + CalcTextSize(description));
		ItemDescription(description, id, ImRect(ImVec2(color_bb.Min.x - 25, desc_bb.Min.y), ImVec2(color_bb.Min.x - 5, desc_bb.Max.y)), hovered, ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y), it->second.description_col, it->second.draw_list, font::description_font, font::description_font->FontSize * it->second.size_scale);

		it->second.draw_list->AddRectFilled(check_bb.Min, check_bb.Max, c::second_color, c::elements::rounding);
		it->second.draw_list->AddRectFilled(check_bb.Min + ImVec2(1, 1), check_bb.Max - ImVec2(1, 1), GetColorU32(it->second.rect_color), c::elements::rounding);
		it->second.draw_list->AddRect(check_bb.Min, check_bb.Max, c::stroke_color, c::elements::rounding);
		it->second.draw_list->AddShadowCircle(check_bb.GetCenter(), 10.f, utils::GetColorWithAlpha(it->second.rect_color, it->second.checkbox_alpha * 0.75f), 95.f, ImVec2(0, 0));
		RenderCheckMark(it->second.draw_list, check_bb.GetCenter() - ImVec2(7, 7), utils::GetColorWithAlpha(c::window_bg_color, it->second.checkbox_alpha), 14.f);

		it->second.draw_list->AddRectFilled(color_bb.Min, color_bb.Max, ImColor(col_v4), c::elements::rounding);
		RenderColorRectWithAlphaCheckerboard(it->second.draw_list, color_bb.Min, color_bb.Max, ImColor(col_v4), ImMin(36, 29) / 2.99f, ImVec2(0.f, 0.f), c::elements::rounding);

		SetCursorScreenPos(ImVec2(total_bb.Min.x, total_bb.Max.y));
		PopID();
		EndGroup();
		return checkbox_pressed;
	}

	struct checkbox_combo_color_state {
		float checkbox_alpha = 0.f;
		ImVec4 rect_color = ImVec4(0, 0, 0, 0);
		ImVec4 text_color = ImVec4(0, 0, 0, 0);
		ImVec4 description_col = ImVec4(0, 0, 0, 0);
		float shader_alpha = 0.f;
		float size_scale = 1.f;
		ImVec2 frame_offset = ImVec2(0, 0);
		float blur_thinkess = 0.f;
		float highlight_offset = -112.f;
		float highlight_alpha = 0.8f;
		float bg_alpha = 0.4f;
		bool picker_is_open = false;
		float picker_scale = 0.f;
		bool combo_open = false;
		unsigned int opened_combo_frame = 0;
		float arrow_roll = 1.f;
		ImDrawList* draw_list = nullptr;
	};

	bool VisualBox(const char* label, const char* description, bool* v, int* combo_current, const char* const combo_items[], int combo_count, float col[4], ImGuiColorEditFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems || combo_count <= 0) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		BeginGroup();
		PushID(label);
		const ImGuiID id = window->GetID(label);
		const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
		if (set_current_color_edit_id)
			g.ColorEditCurrentID = window->IDStack.back();

		const float w = GetContentRegionAvail().x;
		const ImVec2 pos = window->DC.CursorPos;
		const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
		const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);

		static std::map<ImGuiID, checkbox_combo_color_state> anim;
		auto it = anim.emplace(id, checkbox_combo_color_state()).first;

		const ImRect frame_bb(pos, pos + ImVec2(w, 55));
		const ImRect total_bb(pos - it->second.frame_offset, pos + ImVec2(w, 55) + it->second.frame_offset);

		ItemSize(frame_bb, 0.f);
		if (!ItemAdd(frame_bb, id)) {
			if (set_current_color_edit_id) g.ColorEditCurrentID = 0;
			PopID();
			EndGroup();
			return false;
		}

		const float square_sz = 10.f;
		ImVec2 check_offset(15.f * it->second.size_scale, 15.f * it->second.size_scale);
		ImRect check_rect(total_bb.Max - ImVec2(total_bb.GetSize().y, total_bb.GetSize().y), total_bb.Max);
		ImRect check_bb(check_rect.GetCenter() - check_offset, check_rect.GetCenter() + check_offset);

		const float color_square_sz = total_bb.GetSize().y - 32.f;
		const float color_y = total_bb.GetCenter().y - color_square_sz * 0.5f;
		ImRect color_bb(ImVec2(check_bb.Min.x - color_square_sz - 8.f, color_y), ImVec2(check_bb.Min.x - 8.f, color_y + color_square_sz));

		const float combo_w = ImMin(ImMin(w * 0.4f, 180.f), (color_bb.Min.x - total_bb.Min.x) - 12.f);
		ImRect combo_bb(ImVec2(color_bb.Min.x - combo_w - 8.f, total_bb.Min.y), ImVec2(color_bb.Min.x - 8.f, total_bb.Max.y));
		// Only the right strip (preview + arrow) opens combo; rest of row toggles checkbox (no border, no big hit area)
		const float combo_click_w = 90.f;
		ImRect combo_click_bb(ImVec2(combo_bb.Max.x - combo_click_w, combo_bb.Min.y), combo_bb.Max);
		ImRect left_bb(total_bb.Min, ImVec2(combo_click_bb.Min.x - 4.f, total_bb.Max.y));

		bool checkbox_pressed = false;
		bool combo_hovered = ImGui::IsMouseHoveringRect(combo_click_bb.Min, combo_click_bb.Max, false) && !IsAnyComboOrPickerOpen();
		bool color_hovered = ImGui::IsMouseHoveringRect(color_bb.Min, color_bb.Max, false) && !IsAnyComboOrPickerOpen();
		bool left_or_check_hovered = (ImGui::IsMouseHoveringRect(left_bb.Min, left_bb.Max, false) || ImGui::IsMouseHoveringRect(check_bb.Min, check_bb.Max, false)) && !IsAnyComboOrPickerOpen();
		bool hovered = left_or_check_hovered || combo_hovered || color_hovered;

		if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			ImVec2 mousePos = ImGui::GetMousePos();
			if (color_bb.Contains(mousePos) && !(flags & ImGuiColorEditFlags_NoPicker)) {
				g.ColorPickerRef = col_v4;
				it->second.picker_is_open = true;
				SetNextWindowPos(ImVec2(color_bb.Max.x + 4.f, color_bb.Min.y), ImGuiCond_FirstUseEver);
				SetNextWindowSize(ImVec2(280, 380), ImGuiCond_FirstUseEver);
			}
			else if (combo_click_bb.Contains(mousePos)) {
				it->second.combo_open = !it->second.combo_open;
				if (it->second.combo_open) it->second.opened_combo_frame = g.FrameCount;
			}
			else {
				*v = !(*v);
				checkbox_pressed = true;
			}
		}

		char popup_id[64];
		ImFormatString(popup_id, IM_ARRAYSIZE(popup_id), "VisualBoxPopup_%08X", id);
		UpdateAnyComboOrPickerOpen(it->second.combo_open || it->second.picker_is_open);
		it->second.picker_scale = ImLerp(it->second.picker_scale, it->second.picker_is_open ? 1.f : 0.f, GetIO().DeltaTime * 5.f);
		it->second.arrow_roll = ImLerp(it->second.arrow_roll, it->second.combo_open ? -1.f : 1.f, g.IO.DeltaTime * 6.f);

		if (it->second.combo_open) {
			ImVec2 viewport_size = GetMainViewport()->Size;
			ImVec2 viewport_pos = GetMainViewport()->Pos;
			float popup_h = (combo_count * 32) + 22.f;
			ImVec2 window_size(combo_bb.GetWidth(), popup_h);
			ImVec2 window_pos(combo_bb.Min.x, combo_bb.Max.y + 2.f);
			if (window_pos.x + window_size.x > viewport_pos.x + viewport_size.x) window_pos.x = viewport_pos.x + viewport_size.x - window_size.x;
			if (window_pos.y + window_size.y > viewport_pos.y + viewport_size.y) window_pos.y = viewport_pos.y + viewport_size.y - window_size.y;
			if (window_pos.y < viewport_pos.y) window_pos.y = viewport_pos.y;
			SetNextWindowPos(window_pos, ImGuiCond_Always);
			SetNextWindowSize(window_size, ImGuiCond_Always);
			PushStyleColor(ImGuiCol_WindowBg, utils::ImColorToImVec4(c::window_bg_color));
			PushStyleVar(ImGuiStyleVar_WindowRounding, c::elements::rounding);
			PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
			PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
			if (Begin(popup_id, NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {
				GetForegroundDrawList()->AddRectFilled(GetWindowPos(), GetWindowPos() + GetWindowSize(), utils::GetColorWithAlpha(c::window_bg_color, style.Alpha / 2), c::elements::rounding * 2);
				if (!IsWindowHovered() && g.IO.MouseClicked[0] && g.FrameCount != it->second.opened_combo_frame)
					it->second.combo_open = false;
				for (int i = 0; i < combo_count; i++) {
					PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 15));
					bool selected = (*combo_current == i);
					if (custom::Selectable(combo_items[i], selected)) {
						*combo_current = i;
						it->second.combo_open = false;
					}
					if (selected) SetItemDefaultFocus();
					PopStyleVar(1);
				}
				GetForegroundDrawList()->AddRect(GetWindowPos(), GetWindowPos() + GetWindowSize(), GetColorU32(c::child::stroke), c::elements::rounding * 2);
				End();
			}
			PopStyleVar(3);
			PopStyleColor(1);
		}

		if (it->second.picker_scale > 0.05f) {
			char picker_id[64];
			ImFormatString(picker_id, IM_ARRAYSIZE(picker_id), "##cbcolor_%u", id);
			PushStyleVar(ImGuiStyleVar_PopupRounding, c::child::rounding);
			PushStyleVar(ImGuiStyleVar_Alpha, it->second.picker_scale);
			if (Begin(picker_id, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
				ImRect bb = GetCurrentWindow()->Rect();
				if (it->second.picker_scale > 0.55f && !bb.Contains(GetMousePos()) && IsMouseClicked(ImGuiMouseButton_Left))
					it->second.picker_is_open = false;
				PushClipRect(ImVec2(0, 0), GetMainViewport()->Size, false);
				GetForegroundDrawList()->AddRectFilled(bb.Min, bb.Max, utils::GetColorWithAlpha(c::window_bg_color, style.Alpha / 3), c::elements::rounding * 2);
				GetForegroundDrawList()->AddRect(bb.Min, bb.Max, ImColor(1.f, 1.f, 1.f, style.Alpha * 0.05f), c::elements::rounding * 2);
				PopClipRect();
				if (g.CurrentWindow->BeginCount == 1) {
					ImGuiColorEditFlags picker_flags = (flags & (ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar)) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
					SetNextItemWidth(260.f);
					ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
				}
				End();
			}
			PopStyleVar(2);
		}
		if (set_current_color_edit_id)
			g.ColorEditCurrentID = 0;

		it->second.shader_alpha = ImLerp(it->second.shader_alpha, *v ? 0.65f : 0.f, GetAnimSpeed());
		it->second.checkbox_alpha = ImLerp(it->second.checkbox_alpha, *v ? 1.f : 0.f, GetAnimSpeed());
		it->second.rect_color = ImLerp(it->second.rect_color, *v ? c::anim::active : utils::GetColorWithAlpha(c::anim::active, 0.f), GetAnimSpeed());
		it->second.text_color = ImLerp(it->second.text_color, *v ? c::anim::active : hovered ? c::text::label::hovered : c::text::label::regular, GetAnimSpeed());
		it->second.description_col = ImLerp(it->second.description_col, *v ? c::text::description::active : hovered ? c::text::description::hovered : c::text::description::regular, GetAnimSpeed());
		bool allow_zoom = (hovered || it->second.picker_is_open || it->second.combo_open) && !IsAnyComboOrPickerOpen();
		it->second.draw_list = (it->second.size_scale > 1.01f && !it->second.picker_is_open) ? GetForegroundDrawList() : GetWindowDrawList();
		it->second.size_scale = ImLerp(it->second.size_scale, allow_zoom ? 1.05f : 1.f, GetAnimSpeed());
		it->second.frame_offset = ImLerp(it->second.frame_offset, allow_zoom ? ImVec2(50 * it->second.size_scale, 12.5f * it->second.size_scale) : ImVec2(0, 0), GetAnimSpeed());
		it->second.blur_thinkess = ImLerp(it->second.blur_thinkess, allow_zoom ? 1.f : 0.f, GetAnimSpeed() * 2);
		it->second.highlight_offset = ImLerp(it->second.highlight_offset, allow_zoom ? total_bb.GetSize().x + 52.f : -112.f, GetAnimSpeed() / 3);
		it->second.bg_alpha = ImLerp(it->second.bg_alpha, allow_zoom ? 1.f : 0.4f, GetAnimSpeed());
		if (!allow_zoom) it->second.highlight_offset = -112.f;

		it->second.draw_list->AddRectFilled(total_bb.Min, total_bb.Max, utils::GetColorWithAlpha(c::window_bg_color, it->second.bg_alpha), c::elements::rounding);
		shaderrt_v2::Draw_v2(it->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding, it->second.shader_alpha, ImShaderTex_WindowBg_v2);
		it->second.draw_list->AddShadowRect(total_bb.Min, total_bb.Max, ImColor(0.f, 0.f, 0.f, it->second.blur_thinkess), 70.f, ImVec2(0, 0), ImDrawFlags_ShadowCutOutShapeBackground, c::elements::rounding);
		crr::Push(it->second.draw_list, total_bb.Min, total_bb.Max, c::elements::rounding);
		it->second.draw_list->AddImage(c::highlight_image, total_bb.Min + ImVec2(it->second.highlight_offset - 32, -32), total_bb.Min + ImVec2(it->second.highlight_offset + 55 + 32, 55 + 32), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, it->second.highlight_alpha));
		crr::Pop(it->second.draw_list);
		ItemBackground(id, total_bb, hovered, it->second.draw_list);

		it->second.draw_list->AddText(GetDefaultFont(), GetDefaultFont()->FontSize * it->second.size_scale, ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y - CalcTextSize(label).y), ImGui::GetColorU32(ImVec4(it->second.text_color)), label);
		ItemDescription(description, id, ImRect(ImVec2(combo_bb.Min.x - 25, total_bb.GetCenter().y), ImVec2(combo_bb.Min.x - 5, total_bb.GetCenter().y + CalcTextSize(description).y)), hovered, ImVec2(total_bb.Min.x + 14.f, total_bb.GetCenter().y), it->second.description_col, it->second.draw_list, font::description_font, font::description_font->FontSize * it->second.size_scale);

		it->second.draw_list->AddRectFilled(check_bb.Min, check_bb.Max, c::second_color, c::elements::rounding);
		it->second.draw_list->AddRectFilled(check_bb.Min + ImVec2(1, 1), check_bb.Max - ImVec2(1, 1), ImGui::GetColorU32(ImVec4(it->second.rect_color)), c::elements::rounding);
		it->second.draw_list->AddRect(check_bb.Min, check_bb.Max, c::stroke_color, c::elements::rounding);
		it->second.draw_list->AddShadowCircle(check_bb.GetCenter(), 10.f, utils::GetColorWithAlpha(it->second.rect_color, it->second.checkbox_alpha * 0.75f), 95.f, ImVec2(0, 0));
		RenderCheckMark(it->second.draw_list, check_bb.GetCenter() - ImVec2(7, 7), utils::GetColorWithAlpha(c::window_bg_color, it->second.checkbox_alpha), 14.f);

		it->second.draw_list->AddRectFilled(color_bb.Min, color_bb.Max, ImColor(col_v4), c::elements::rounding);
		RenderColorRectWithAlphaCheckerboard(it->second.draw_list, color_bb.Min, color_bb.Max, ImColor(col_v4), ImMin(36, 29) / 2.99f, ImVec2(0.f, 0.f), c::elements::rounding);

		const char* combo_preview = (*combo_current >= 0 && *combo_current < combo_count) ? combo_items[*combo_current] : "?";
		PushClipRect(combo_bb.Min, combo_bb.Max, true);
		it->second.draw_list->AddText(GetDefaultFont(), GetDefaultFont()->FontSize * it->second.size_scale, ImVec2(combo_bb.Max.x - 34.f - CalcTextSize(combo_preview).x, combo_bb.GetCenter().y - CalcTextSize(combo_preview).y * 0.5f), utils::GetColorWithAlpha(c::label::active, style.Alpha), combo_preview);
		PopClipRect();
		ImRotateStart();
		it->second.draw_list->AddText(GetDefaultFont(), GetDefaultFont()->FontSize * it->second.size_scale, ImVec2(combo_bb.Max.x - 21.f - CalcTextSize(ICON_DOWN_SMALL_LINE).x / 2, combo_bb.GetCenter().y - CalcTextSize(ICON_DOWN_SMALL_LINE).y / 2), c::text::label::regular, ICON_DOWN_SMALL_LINE);
		ImRotateEnd(1.57f * it->second.arrow_roll);

		SetCursorScreenPos(ImVec2(total_bb.Min.x, total_bb.Max.y));
		PopID();
		EndGroup();
		return checkbox_pressed;
	}

	bool InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		// Use InputTextEx directly to support size parameter (both width and height)
		// InputTextEx handles both size.x (width) and size.y (height) automatically
		// For multiline input, set ImGuiInputTextFlags_Multiline flag and specify size.y > 0
		// InputTextEx is accessible via imgui_internal.h which is included in custom_widgets.hpp
		return InputTextEx(label, hint, buf, (int)buf_size, size, flags, callback, user_data);
	}
}


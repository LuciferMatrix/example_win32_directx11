#define IMGUI_DEFINE_MATH_OPERATORS


#include <string>
#include <wtypes.h>

#include <iostream>
#include <Windows.h>

#include <imgui_internal.h>

#include "notifications.h"	
#include "imstb_textedit.h"	

#include <cstdlib>
#include <imgui.h>

#include <map>

using namespace ImGui;

namespace custom
{
	void				ItemBackground(ImGuiID id, ImRect bb, bool hovered, ImDrawList* draw_list = ImGui::GetWindowDrawList());

	bool				Tab(const char* label, const char* icon, int* v, int number);;
	bool				SubTab(const char* label, int* v, int number, ImColor color);

	bool				ThemeButton(const char* id_theme, bool dark, const ImVec2& size_arg);
	bool				Button(const char* label, const ImVec2& size_arg);

	bool				ChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool cap = false, ImGuiWindowFlags flags = 0);
	bool				Child(const char* str_id, const ImVec2& size = ImVec2(0, 0), bool cap = false, ImGuiWindowFlags flags = 0);
	bool				ChildID(ImGuiID id, const ImVec2& size = ImVec2(0, 0), bool cap = false, ImGuiWindowFlags flags = 0);
	void				EndChild();

	void				BeginGroup();
	void				EndGroup();

	bool				Checkbox(const char* label, const char* description, bool* v);
	bool				CheckboxClicked(const char* label, bool* v);


	bool			    Selectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
	bool				Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));

	bool				BeginCombo(const char* label, const char* description, const char* preview_value, int val = 0, bool multi = false, ImGuiComboFlags flags = 0);
	void				EndCombo();
	void				MultiCombo(const char* label, bool variable[], const char* labels[], int count);
	bool				Combo(const char* label, const char* description, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
	bool				Combo(const char* label, const char* description, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);
	bool				Combo(const char* label, const char* description, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int popup_max_height_in_items = -1);

	bool				ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
	bool				ColorEdit4(const char* label, const char* description, float col[4], ImGuiColorEditFlags flags = 0);
	bool			    ColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);

	bool				KnobScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags = 0);
	bool				KnobFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags = 0);
	bool				KnobInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags = 0);

	bool				SliderScalar(const char* label, const char* description, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags = 0);
	bool				SliderFloat(const char* label, const char* description, float* v, float v_min, float v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	bool				SliderInt(const char* label, const char* description, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);

	bool				Keybind(const char* label, const char* description, int* key, int* mode, bool allow_mouse = true);
	bool				MiniBind(const char* label, int* key, int* mode, bool allow_mouse = true);
	const char*			KeyName(int vk);

	void				Separator_line();

	void				SeparatorEx(ImGuiSeparatorFlags flags, float thickness);
	void				Separator();

	bool				ColorBox(const char* label, const char* description, bool* v, float col[4], ImGuiColorEditFlags flags = 0);
	bool				VisualBox(const char* label, const char* description, bool* v, int* combo_current, const char* const combo_items[], int combo_count, float col[4], ImGuiColorEditFlags flags = 0);

	bool				InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

}
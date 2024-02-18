#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>

#include "utils/overlay/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "utils/overlay/imgui/imgui_internal.h"

extern ImFont* tab_icons;

namespace elements {
    bool tab(const char* icon, bool boolean);
    bool subtab(const char* name, bool boolean);
}


#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>

#include "utils/overlay/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "utils/overlay/imgui/imgui_internal.h"

extern ImFont* bold;

using namespace ImGui;

namespace e_elements {
    bool begin_child_ex(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags flags);
    bool begin_child(const char* str_id, const ImVec2& size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0);
    bool begin_child(ImGuiID id, const ImVec2& size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0);
    void end_child();
}
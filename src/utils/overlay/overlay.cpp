#define IMGUI_DEFINE_MATH_OPERATORS

#include "overlay.hpp"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <dwmapi.h>

#include "../../roblox/esp/esp.hpp"
#include "../../roblox/globals/globals.hpp"

// lua env
#include "../../lua_env/lua_overlay.hpp"
#include "../../lua_env/LuaVM.hpp"

#include "ckeybind/keybind.hpp"
#include "../skcrypt/skStr.hpp"
#include "../xorstr/xorstr.hpp"

#include "../configs/configs.hpp"
#include "../json/json.hpp"

#include <filesystem>
#include "../../icons.h"
#include "../../animation.h"
#include "../../icon.h"
#include "../../font_raw.h"
#include "../../nav_elements.h"
static int tabs = 1;

float box_thickness = 1;

//keyauth info removed -> change globals::show_auth to false
//auto name = skCrypt("Ethic");
//auto ownerid = skCrypt("SWhiNSeijg");
//auto secret = skCrypt("280fb1e3ffc1407fe17207b1da53758b8a969ff7b47313f947b0f1c4a3e7c01d");
//auto version = skCrypt("1.0");
//auto url = skCrypt("https://keyauth.win/api/1.2/");

//KeyAuth::api KeyAuthApp(name.decrypt(), ownerid.decrypt(), secret.decrypt(), version.decrypt(), url.decrypt());

ID3D11Device* ethic::utils::overlay::d3d11_device = nullptr;

ID3D11DeviceContext* ethic::utils::overlay::d3d11_device_context = nullptr;

ID3D11ShaderResourceView* Logo = nullptr;

IDXGISwapChain* ethic::utils::overlay::dxgi_swap_chain = nullptr;

ID3D11RenderTargetView* ethic::utils::overlay::d3d11_render_target_view = nullptr;
ImVec2 pos;


ImFont* medium;
ImFont* bold;
ImFont* tab_icons;
ImFont* logo;
ImFont* tab_title;
ImFont* tab_title_icon;
ImFont* subtab_title;
ImFont* combo_arrow;


static const char* combo_items_4[4] = { ("Head"), ("UpperTorso"), ("HumanoidRootPart"), ("LowerTorso") };
static const char* combo_items_2[2] = { ("Mouse"), ("Camera"), };
static const char* box_items_3[3] = { ("2D Box"), ("2D Corner Box"), ("Box Filled")};

bool Keybind(CKeybind* keybind, const ImVec2& size_arg = ImVec2(0, 0), bool clicked = false, ImGuiButtonFlags flags = 0)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;
	// SetCursorPosX(window->Size.x - 14 - size_arg.x);
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(keybind->get_name().c_str());
	const ImVec2 label_size = ImGui::CalcTextSize(keybind->get_name().c_str(), NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) &&
		style.FramePadding.y <
		window->DC.CurrLineTextBaseOffset)  // Try to vertically align buttons that are smaller/have no padding so that
		// text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = ImGui::CalcItemSize(
		size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	bool value_changed = false;
	int key = keybind->key;

	auto io = ImGui::GetIO();

	std::string name = keybind->get_key_name();

	if (keybind->waiting_for_input)
		name = "waiting";

	if (ImGui::GetIO().MouseClicked[0] && hovered)
	{
		if (g.ActiveId == id)
		{
			keybind->waiting_for_input = true;
		}
	}
	else if (ImGui::GetIO().MouseClicked[1] && hovered)
	{
		ImGui::OpenPopup(keybind->get_name().c_str());
	}
	else if (ImGui::GetIO().MouseClicked[0] && !hovered)
	{
		if (g.ActiveId == id)
			ImGui::ClearActiveID();
	}

	if (keybind->waiting_for_input)
	{
		if (ImGui::GetIO().MouseClicked[0] && !hovered)
		{
			keybind->key = VK_LBUTTON;

			ImGui::ClearActiveID();
			keybind->waiting_for_input = false;
		}
		else
		{
			if (keybind->set_key())
			{
				ImGui::ClearActiveID();
				keybind->waiting_for_input = false;
			}
		}
	}

	// Render
	ImVec4 textcolor = ImLerp(ImVec4(201 / 255.f, 204 / 255.f, 210 / 255.f, 1.f), ImVec4(1.0f, 1.0f, 1.0f, 1.f), 1.f);

	window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(33 / 255.0f, 33 / 255.0f, 33 / 255.0f, 1.f), 2.f);
	//window->DrawList->AddRect( bb.Min, bb.Max, ImColor( 0.f, 0.f, 0.f, 1.f ) );

	window->DrawList->AddText(
		bb.Min +
		ImVec2(
			size_arg.x / 2 - ImGui::CalcTextSize(name.c_str()).x / 2,
			size_arg.y / 2 - ImGui::CalcTextSize(name.c_str()).y / 2),
		ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)),
		name.c_str());

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar;
	//ImGui::SetNextWindowPos( pos + ImVec2( 0, size_arg.y - 1 ) );
	//ImGui::SetNextWindowSize( ImVec2( size_arg.x, 47 * 1.f ) );

	{
		if (ImGui::BeginPopup(keybind->get_name().c_str(), 0))
		{

			{
				{
					ImGui::BeginGroup();
					{
						if (ImGui::Selectable("Hold", keybind->type == CKeybind::HOLD))
							keybind->type = CKeybind::HOLD;
						if (ImGui::Selectable("Toggle", keybind->type == CKeybind::TOGGLE))
							keybind->type = CKeybind::TOGGLE;
					}
					ImGui::EndGroup();
				}
			}

			ImGui::EndPopup();
		}
	}

	return pressed;
}

bool ethic::utils::overlay::fullsc(HWND windowHandle)
{
	MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
	if (GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
	{
		RECT windowRect;
		if (GetWindowRect(windowHandle, &windowRect))
		{
			return windowRect.left == monitorInfo.rcMonitor.left
				&& windowRect.right == monitorInfo.rcMonitor.right
				&& windowRect.top == monitorInfo.rcMonitor.top
				&& windowRect.bottom == monitorInfo.rcMonitor.bottom;
		}
	}
}

std::string GetConfigFolderPath()
{
	std::string configFolderPath = ethic::utils::appdata_path() + "\\ethic\\configs";

	if (!std::filesystem::exists(configFolderPath))
	{
		std::filesystem::create_directory(configFolderPath);
	}

	return configFolderPath;
}

//void DisplayInstanceTree(ethic::roblox::instance_t instance)
//{
//	for (auto& child : instance.children())
//	{
//		std::string childName = child.name();
//
//		ImGui::PushID(childName.c_str());
//		bool isNodeOpen = ImGui::TreeNodeEx(childName.c_str(), ImGuiTreeNodeFlags_OpenOnArrow);
//
//		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1))
//		{
//			ImGui::OpenPopup("Context Menu");
//		}
//
//		if (ImGui::BeginPopup("Context Menu"))
//		{
//			if (ImGui::MenuItem("Copy Text"))
//			{
//				ImGui::SetClipboardText(childName.c_str());
//			}
//			ImGui::EndPopup();
//		}
//
//		if (isNodeOpen)
//		{
//			DisplayInstanceTree(child);
//			ImGui::TreePop();
//		}
//
//		ImGui::PopID();
//	}
//}

static char text[999] = "";
char configname[100];

bool ethic::utils::overlay::init = false;

bool if_first = false;

bool ButtonCenteredOnLine(const char* label, float alignment = 0.5f)
{
	ImGuiStyle& style = ImGui::GetStyle();

	float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
	float avail = ImGui::GetContentRegionAvail().x;

	float off = (avail - size) * alignment;
	if (off > 0.0f)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

	return ImGui::Button(label);
}


enum heads {
	rage, antiaim, visuals, settings, skins, configz, scripts
};

enum sub_heads {
	general, accuracy, exploits, _general, advanced
};




inline ImFont* LexendRegular;
inline ImFont* LexendLight;

template<class T, class U>
inline static T clamp(const T& in, const U& low, const U& high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}

inline void DrawBox(float X, float Y, float W, float H, const ImU32& color, int thickness)
{
	ImGui::GetForegroundDrawList()->AddRect(ImVec2(X, Y), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
}

inline bool ImGui::Renderingtab(const char* icon, bool selected)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(icon);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = { 35, 35 };

	const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

	ImGui::ItemSize(size, 0);

	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);

	int current_tab;

	if (hovered || held)
		ImGui::SetMouseCursor(9);

	float deltatime = 1.5f * ImGui::GetIO().DeltaTime;
	static std::map<ImGuiID, float> hover_animation;
	auto it_hover = hover_animation.find(id)
		;
	if (it_hover == hover_animation.end())
	{
		hover_animation.insert({ id, 0.f });
		it_hover = hover_animation.find(id);
	}

	it_hover->second = clamp(it_hover->second + (1.15f * ImGui::GetIO().DeltaTime * (hovered ? 1.f : -1.f)), 0.0f, 0.0f);
	it_hover->second *= ImGui::GetStyle().Alpha;

	static std::map<ImGuiID, float> filled_animation;
	auto it_filled = filled_animation.find(id);

	if (it_filled == filled_animation.end())
	{
		filled_animation.insert({ id, 0.f });
		it_filled = filled_animation.find(id);
	}

	it_filled->second = clamp(it_filled->second + (2.15f * ImGui::GetIO().DeltaTime * (selected ? 1.f : -1.5f)), it_hover->second, 1.f);
	it_filled->second *= ImGui::GetStyle().Alpha;

	ImGui::GetWindowDrawList()->AddText(ImVec2(bb.Min.x + 20, bb.Min.y + 4), ImColor(64, 64, 64, int(255 * ImGui::GetStyle().Alpha)), icon);

	if (selected)
	{
		ImGui::GetWindowDrawList()->AddText(ImVec2(bb.Min.x + 20, bb.Min.y + 4), ImColor(183, 183, 183, int(255 * it_filled->second)), icon);
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(bb.Min.x - 11, bb.Min.y + 8.75f), ImVec2(bb.Min.x - 7 * it_filled->second, bb.Max.y - 8.75f), ImColor(201, 135, 52, int(255 * it_filled->second)), 10.f, ImDrawCornerFlags_Right);
	}

	return pressed;
}

inline bool ImGui::Renderingsubtab(const char* icon, bool selected)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(std::string(icon + std::string(icon)).c_str());
	const ImVec2 label_size = ImGui::CalcTextSize(icon);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = { 35, 35 };

	const ImRect bb(pos, ImVec2(pos.x + 35, pos.y + 35));
	ImGui::ItemSize(size, 50);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);

	if (hovered || held);

	float t = selected ? 1.0f : 0.0f;
	float deltatime = 1.5f * ImGui::GetIO().DeltaTime;
	static std::map<ImGuiID, float> hover_animation;
	auto it_hover = hover_animation.find(id);
	if (it_hover == hover_animation.end())
	{
		hover_animation.insert({ id, 0.f });
		it_hover = hover_animation.find(id);
	}
	it_hover->second = ImClamp(it_hover->second + (0.2f * ImGui::GetIO().DeltaTime * (hovered || ImGui::IsItemActive() ? 1.f : -1.f)), 0.0f, 0.15f);
	it_hover->second *= min(ImGui::GetStyle().Alpha * 1.2, 1.f);

	static std::map<ImGuiID, float> filled_animation;
	auto it_filled = filled_animation.find(id);
	if (it_filled == filled_animation.end())
	{
		filled_animation.insert({ id, 0.f });
		it_filled = filled_animation.find(id);
	}
	it_filled->second = ImClamp(it_filled->second + (2.55f * ImGui::GetIO().DeltaTime * (selected ? 1.f : -1.0f)), it_hover->second, 1.f);
	it_filled->second *= min(ImGui::GetStyle().Alpha * 1.2, 1.f);

	static std::map<ImGuiID, float> fill_animation;
	auto it_fill = fill_animation.find(id);
	if (it_fill == fill_animation.end())
	{
		fill_animation.insert({ id, 0.f });
		it_fill = fill_animation.find(id);
	}
	it_fill->second = ImClamp(it_filled->second + (1.75f * ImGui::GetIO().DeltaTime * (selected ? 1.f : -1.0f)), it_hover->second, 1.f);
	it_fill->second *= min(ImGui::GetStyle().Alpha * 1.2, 1.f);

	ImVec4 text = ImLerp(ImVec4{ 128 / 255.f, 128 / 255.f, 128 / 255.f, ImGui::GetStyle().Alpha }, ImVec4{ 255 / 255.f, 255 / 255.f, 255 / 255.f, ImGui::GetStyle().Alpha }, it_filled->second);
	ImVec4 text2 = ImLerp(ImVec4{ 128 / 255.f, 128 / 255.f, 128 / 255.f, ImGui::GetStyle().Alpha }, ImVec4{ 255 / 255.f, 255 / 255.f, 255 / 255.f, ImGui::GetStyle().Alpha }, it_filled->second);

	ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(bb.Min.x + 12, bb.Max.y - it_fill->second * 5), ImVec2(bb.Min.x + 35, bb.Max.y - 1), ImColor(201, 135, 52, int(255 * it_filled->second)), 3, ImDrawCornerFlags_Top);

	ImGui::GetForegroundDrawList()->AddText(ImVec2(bb.Min.x, bb.Min.y + 8), ImColor(text), icon);

	return pressed;
}

inline int MenuTab;
inline static float switch_alpha[3], open_alpha = 0;


bool ethic::utils::overlay::render()
{

	//name.clear(); ownerid.clear(); secret.clear(); version.clear(); url.clear();

	//if (if_first)
	//{
	//	KeyAuthApp.init();
	//	if_first = false;
	//}
	// //this is where it checks auth in the external itself since atg downloaded it from a imgui laopder they had 2 checks need to maek 1 :nonman
	//nlohmann::json json;

	//if (globals::show_auth)
	//{
	//	std::string filePath = appdata_path() + XorStr("\\ethic\\license.json");
	//	if (std::filesystem::exists(filePath))
	//	{
	//		std::ifstream file(filePath);
	//		if (file.is_open())
	//		{
	//			file >> json;
	//			file.close();

	//			std::string stored_license = json["license"];
	//			KeyAuthApp.license(stored_license);

	//			if (KeyAuthApp.data.success)
	//			{
	//				globals::show_auth = false;
	//			}
	//			else
	//			{
	//				//exit(0);
	//				globals::show_auth = false;

	//			}
	//		}
	//	}
	//}

		ImGui_ImplWin32_EnableDpiAwareness();

		WNDCLASSEX wc;
		wc.cbClsExtra = NULL;
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.cbWndExtra = NULL;
		wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hInstance = GetModuleHandle(nullptr);
		wc.lpfnWndProc = window_proc;
		wc.lpszClassName = TEXT("ethic");
		wc.lpszMenuName = nullptr;
		wc.style = CS_VREDRAW | CS_HREDRAW;

		RegisterClassEx(&wc);
		const HWND hw = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, TEXT("ethic-workspace"),
			WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr);

		SetLayeredWindowAttributes(hw, 0, 255, LWA_ALPHA);
		const MARGINS margin = { -1 };
		DwmExtendFrameIntoClientArea(hw, &margin);

		if (!create_device_d3d(hw))
		{
			cleanup_device_d3d();
			UnregisterClass(wc.lpszClassName, wc.hInstance);
			return false;
		}

		ShowWindow(hw, SW_SHOW);
		UpdateWindow(hw);

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
		ImFontConfig icons_config;

		ImFontConfig CustomFont;
		CustomFont.FontDataOwnedByAtlas = false;

		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.OversampleH = 2.5;
		icons_config.OversampleV = 2.5;
		ImFontConfig font_config;
		font_config.OversampleH = 1;
		font_config.OversampleV = 1;
		font_config.PixelSnapH = 1;


		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF,
			0x0400, 0x044F,
			0,
		};


		medium = io.Fonts->AddFontFromMemoryTTF(PTRootUIMedium, sizeof(PTRootUIMedium), 15.0f, &font_config, ranges);
		bold = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold, sizeof(PTRootUIBold), 15.0f, &font_config, ranges);

		tab_icons = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 15.0f, &font_config, ranges);
		logo = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 21.0f, &font_config, ranges);

		tab_title = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold, sizeof(PTRootUIBold), 19.0f, &font_config, ranges);
		tab_title_icon = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 18.0f, &font_config, ranges);

		subtab_title = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold, sizeof(PTRootUIBold), 15.0f, &font_config, ranges);

		combo_arrow = io.Fonts->AddFontFromMemoryTTF(combo, sizeof(combo), 9.0f, &font_config, ranges);
		io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges);
		io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAR, 16.0f, &icons_config, icons_ranges);

		ImGui::StyleColorsDark();
		ImGui::GetIO().IniFilename = nullptr;

		ImGui_ImplWin32_Init(hw);
		ImGui_ImplDX11_Init(d3d11_device, d3d11_device_context);

		const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		init = true;

		bool draw = false;
		bool done = false;
		int tab = 0;
		// lua env - mogus
		ethic::lua_overlay::init();
		globals::luaVM.init();
		while (!done)
		{
			MSG msg;
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
				{
					done = true;
				}
			}

			if (done)
				break;

			move_window(hw);

			if (GetAsyncKeyState(VK_INSERT) & 1)
				draw = !draw;

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			{
				if (GetForegroundWindow() == FindWindowA(0, XorStr("Roblox")) || GetForegroundWindow() == hw)
				{
					ImGui::Begin(XorStr("overlay"), nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
					{
						ethic::roblox::hook_esp();
						ImGui::End();
					}

					static int selected = 0;
					static int sub_selected = 0;
					if (draw)
					{

						ImGui::SetNextWindowSize({ 565, 395 });
						ImGui::Begin(("Ethic"), nullptr, ImGuiWindowFlags_NoDecoration);
						auto draw = ImGui::GetWindowDrawList();
						ImVec2 pos = ImGui::GetWindowPos();
						ImVec2 size = ImGui::GetWindowSize();

						draw->AddRectFilled(pos, ImVec2(pos.x + 100, pos.y + size.y), ImColor(20, 20, 20, 255));
						draw->AddRectFilled(ImVec2(pos.x + 100, pos.y), ImVec2(pos.x + size.x, pos.y + 35), ImColor(20, 20, 20, 255));

						draw->AddLine(ImVec2(pos.x + 0, pos.y + 35), ImVec2(pos.x + size.x, pos.y + 35), ImColor(255, 255, 255, 15));
						draw->AddLine(ImVec2(pos.x + 100, pos.y), ImVec2(pos.x + 100, pos.y + size.y), ImColor(255, 255, 255, 15));

						// Load Fonts
						ImFontConfig font_config;
						font_config.PixelSnapH = false;
						font_config.OversampleH = 5;
						font_config.OversampleV = 5;
						font_config.RasterizerMultiply = 1.2f;

						static const ImWchar ranges[] =
						{
							0x0020, 0x00FF, // Basic Latin + Latin Supplement
							0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
							0x2DE0, 0x2DFF, // Cyrillic Extended-A
							0xA640, 0xA69F, // Cyrillic Extended-B
							0xE000, 0xE226, // icons
							0,
						};

						font_config.GlyphRanges = ranges;
						ImGuiIO& io = ImGui::GetIO(); (void)io;

						io.Fonts->AddFontFromFileTTF(("C:\\Windows\\Fonts\\verdana.ttf"), 14.0f, &font_config, ranges);

						ImGui::SetCursorPos(ImVec2(30, 13));
						ImGui::Text(("Ethic"));
						ImGui::SetCursorPos(ImVec2(15, 40));
						ImGui::BeginGroup();
						ImGui::PushFont(medium);
						{
							if (ImGui::Renderingtab(("Aimbot"), 0 == selected)) {
								selected = 0; sub_selected = 0;
							}
							if (ImGui::Renderingtab(("Visual"), 1 == selected)) {
								selected = 1; sub_selected = 2;
							}
							if (ImGui::Renderingtab(("Misc"), 2 == selected)) {
								selected = 2; sub_selected = 5;
							}
						}
						ImGui::PopFont();
						ImGui::EndGroup();

						if (selected == 0) {
							ImGui::BeginGroup();
							ImGui::SetCursorPos(ImVec2(110, 1));
							if (ImGui::Renderingsubtab((" Aimbot"), 0 == sub_selected)) {
								sub_selected = 0;
							}
							ImGui::EndGroup();


							if (sub_selected == 0) {
								ImGui::SetCursorPos(ImVec2(110, 44));
								ImGui::BeginChild(("Aim"), ImVec2(219, 342));
								{
										ImGui::Spacing();
										ImGui::Checkbox(XorStr("Enabled"), &globals::aimbot);
										if (globals::aimbot) {

											ImGui::Combo(XorStr("Type"), &globals::aimmethod, combo_items_2, 2);

										}
										ImGui::Checkbox(XorStr("Draw FOV"), &globals::fov_on);
										ImGui::Checkbox(XorStr("Sticky Aim"), &globals::sticky_aim);
										ImGui::Checkbox(XorStr("Triggerbot"), &globals::triggerbot);
								}
								ImGui::EndChild();
								ImGui::SetCursorPos(ImVec2(291 + 45, 44));
								ImGui::BeginChild(("Settings"), ImVec2(219, 342));
								{
									ImGui::Spacing();
										ImGui::Text((("Aim Key: "))); ImGui::SameLine(); Keybind(&globals::aimbotkey, ImVec2(60, 20));

											ImGui::PushItemWidth(150);

											ImGui::Combo(XorStr("Part"), &globals::aimpart, combo_items_4, 4);
											ImGui::PopItemWidth();

											ImGui::PushItemWidth(350);
											ImGui::SliderInt(XorStr("FOV"), &globals::fov, 1, 1000);
											ImGui::SliderFloat(XorStr("Smooth X"), &globals::smoothness_x, 1, 10); 								
											ImGui::SliderFloat(XorStr("Smooth Y"), &globals::smoothness_y, 1, 10); 		
											ImGui::Separator();
											ImGui::SliderFloat(XorStr("Pred X"), &globals::prediction_x, 1, 10);
											ImGui::SliderFloat(XorStr("Pred Y"), &globals::prediction_y, 1, 10);

											//ImGui::SliderFloat(XorStr("	Smooth Y"), &globals::smoothness_y, 1, 1);									        ImGui::SameLine(); ImGui::Text(std::to_string(globals::smoothness_y).c_str());
											 
											ImGui::PopItemWidth();
								}
								//ImGui::SliderInt(("Aim Delay:"), &aimdelay, 0, 10);
								//ImGui::SameLine(); ImGui::Text(std::to_string(aimdelay).c_str());
							}
							ImGui::EndChild();
						}
					}

					if (selected == 1) {
						ImGui::BeginGroup();
						ImGui::SetCursorPos(ImVec2(110, 1));
						if (ImGui::Renderingsubtab((" Players"), 2 == sub_selected)) {
							sub_selected = 2;
						}
						ImGui::SameLine();
						if (ImGui::Renderingsubtab(("   Misc"), 4 == sub_selected)) {
							sub_selected = 4;
						}
						ImGui::EndGroup();


						if (sub_selected == 2) {
							ImGui::SetCursorPos(ImVec2(110, 44));
							ImGui::BeginChild(("Visuals"), ImVec2(219, 342));
							{
								ImGui::Spacing();
								ImGui::Checkbox(("Enable Draw VEH"), &globals::esp);
								if (globals::esp)
								{
									ImGui::Checkbox(XorStr("Draw Box"), &globals::box);
									ImGui::Combo(XorStr("Box Type"), &globals::boxtype, box_items_3, 3);
									ImGui::Checkbox(XorStr("Draw Snaplines"), &globals::tracers);
									ImGui::Checkbox(XorStr("Draw Username"), &globals::name_esp);
									ImGui::Checkbox(XorStr("Draw Health"), &globals::healthinfo);
								}
					
		
							}
							ImGui::EndChild();

							ImGui::SetCursorPos(ImVec2(291 + 45, 44));
							ImGui::BeginChild(("Preview"), ImVec2(219, 342), ImGuiWindowFlags_NoScrollbar);
							{
								//ImGui::PushFont(proggy_clean_game);
								//if (draw_held_weapon) {
								//	ImGui::SetCursorPos(ImVec2(57, 31));
								//	ImGui::Text(("Harvesting Tool"));
								//}

								if (globals::name_esp) {
									ImGui::SetCursorPos(ImVec2(57 + 27, 48));
									ImGui::Text(("0day"));
								}

								
								ImGui::SetCursorPos(ImVec2(57 + 30, 243));
								ImGui::Text(("1337m"));
								

								//if (draw_platform) {
								//	ImGui::SetCursorPos(ImVec2(57 + 35, 14));
								//	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 136, 255, 255));
								//	ImGui::Text(("WIN"));
								//	ImGui::PopStyleColor();
								//}

								if (globals::box)
								{
									ImDrawList* draw_list = ImGui::GetForegroundDrawList();
									ImGui::SetCursorPos(ImVec2(57, 65));
									ImVec2 pos1 = ImGui::GetCursorScreenPos();
									DrawBox(pos1.x, pos1.y, 100, 175, ImColor(178, 0, 255, 51), box_thickness);

								}
								//ImGui::PopFont();
							}
							ImGui::EndChild();
						}

						else if (sub_selected == 4) {
							ImGui::SetCursorPos(ImVec2(110, 44));

							ImGui::BeginChild(("Misc"), ImVec2(219 + 226, 342));
							{
								ImGui::Spacing();
							}
							ImGui::EndChild();
						}


					}

					if (selected == 2) {
						ImGui::BeginGroup();
						ImGui::SetCursorPos(ImVec2(110, 1));
						if (ImGui::Renderingsubtab(("   Misc"), 5 == sub_selected)) {
							sub_selected = 5;
						}
						ImGui::EndGroup();

						if (sub_selected == 5) {
							ImGui::SetCursorPos(ImVec2(110, 44));
							ImGui::BeginChild(("Misc"), ImVec2(219, 342));
							{
								ImGui::Spacing();
								ImGui::Checkbox(XorStr("Healthcheck"), &globals::healthcheck);
								ImGui::Checkbox(XorStr("Knockcheck"), &globals::knock_check);
								ImGui::Checkbox(XorStr("Teamcheck"), &globals::team_check_esp);
								ImGui::Checkbox(XorStr("Streamproof"), &globals::streamproof);
								ImGui::Checkbox(XorStr("V-Sync"), &globals::vsync);


								std::vector<std::string> configFiles;
								std::string configFolderPath = GetConfigFolderPath();
								for (auto file : std::filesystem::directory_iterator(configFolderPath)) {
									std::filesystem::path filePath = file;
									std::string extension = filePath.extension().string();

									if (extension == ".cfg") {
										if (!std::filesystem::is_directory(file.path())) {
											auto path2 = file.path().string().substr(configFolderPath.length() + 1);
											configFiles.push_back(path2.c_str());
										}
									}
								}

								static std::string current_item = configFiles.empty() ? "" : configFiles[0];

								for (int n = 0; n < configFiles.size(); n++) {
									if (configFiles[n] == "")
										continue;

									bool is_selected = (current_item == configFiles[n]);
									if (ImGui::Selectable(configFiles[n].c_str(), is_selected)) {
										current_item = configFiles[n];

										size_t extensionPos = current_item.find(".cfg");
										if (extensionPos != std::string::npos) {
											current_item.erase(extensionPos);
										}

										strncpy(configname, current_item.c_str(), IM_ARRAYSIZE(configname));
									}
								}

								ImGui::InputText(XorStr("Config Name"), configname, IM_ARRAYSIZE(configname));
								if (ImGui::Button(XorStr("Load"))) {
									ethic::utils::configs::load(configname);
								}

								ImGui::SameLine();

								if (ImGui::Button(XorStr("Save"))) {
									ethic::utils::configs::save(configname);
								}

								ImGui::SameLine();

								if (ImGui::Button(XorStr("Open Folder Location")))
								{
									ShellExecute(NULL, "open", "explorer.exe", GetConfigFolderPath().c_str(), NULL, SW_SHOWNORMAL);
								}


							}
							ImGui::EndChild();

							ImGui::SetCursorPos(ImVec2(291 + 45, 44));
							ImGui::BeginChild(("Account"), ImVec2(219, 342));
							{
								ImGui::Spacing();
								ImGui::Text((("License duration: ")));
								ImGui::Text((("Status:")));
								ImGui::SameLine();
								ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
								ImGui::Text(("undetected"));
								ImGui::PopStyleColor();
								//ImGui::Text(("Menu Key -> [F1]"));
								//ImGui::Text(("Panic Key -> [F4]"));
							}
							ImGui::EndChild();
						}
					
					ImGui::End();

				}


			
	



					//////








					if (globals::streamproof)
					{
						SetWindowDisplayAffinity(hw, WDA_EXCLUDEFROMCAPTURE);
					}
					else
					{
						SetWindowDisplayAffinity(hw, WDA_NONE);
					}

					//if (globals::show_auth)
					if (draw)
					{
						SetWindowLong(hw, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
					}
					else
					{
						SetWindowLong(hw, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
					}

					ImGui::EndFrame();
					ImGui::Render();

					const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
					d3d11_device_context->OMSetRenderTargets(1, &d3d11_render_target_view, nullptr);
					d3d11_device_context->ClearRenderTargetView(d3d11_render_target_view, clear_color_with_alpha);
					ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

					if (globals::vsync)
					{
						dxgi_swap_chain->Present(1, 0);
					}
					else
					{
						dxgi_swap_chain->Present(0, 0);

					}
				}
			}
		}


	
	
	init = false;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	cleanup_device_d3d();
	DestroyWindow(hw);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
}


void ethic::utils::overlay::move_window(HWND hw)
{
	HWND target = FindWindowA(0, XorStr("Roblox"));
	HWND foregroundWindow = GetForegroundWindow();

	if (target != foregroundWindow && hw != foregroundWindow)
	{
		MoveWindow(hw, 0, 0, 0, 0, true);
	}
	else
	{
		RECT rect;
		GetWindowRect(target, &rect);

		int rsize_x = rect.right - rect.left;
		int rsize_y = rect.bottom - rect.top;

		if (fullsc(target))
		{
			rsize_x += 16;
			rsize_y -= 24;
		}
		else
		{
			rsize_y -= 63;
			rect.left += 8;
			rect.top += 31;
		}

		MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE);
	}
}

bool ethic::utils::overlay::create_device_d3d(HWND hw)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hw;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	const UINT create_device_flags = 0;
	D3D_FEATURE_LEVEL d3d_feature_level;
	const D3D_FEATURE_LEVEL feature_level_arr[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags, feature_level_arr, 2, D3D11_SDK_VERSION, &sd, &dxgi_swap_chain, &d3d11_device, &d3d_feature_level, &d3d11_device_context);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, create_device_flags, feature_level_arr, 2, D3D11_SDK_VERSION, &sd, &dxgi_swap_chain, &d3d11_device, &d3d_feature_level, &d3d11_device_context);
	if (res != S_OK)
		return false;

	create_render_target();
	return true;
}

void ethic::utils::overlay::cleanup_device_d3d()
{
	cleanup_render_target();

	if (dxgi_swap_chain)
	{
		dxgi_swap_chain->Release();
		dxgi_swap_chain = nullptr;
	}

	if (d3d11_device_context)
	{
		d3d11_device_context->Release();
		d3d11_device_context = nullptr;
	}

	if (d3d11_device)
	{
		d3d11_device->Release();
		d3d11_device = nullptr;
	}
}

void ethic::utils::overlay::create_render_target()
{
	ID3D11Texture2D* d3d11_back_buffer;
	dxgi_swap_chain->GetBuffer(0, IID_PPV_ARGS(&d3d11_back_buffer));
	if (d3d11_back_buffer != nullptr)
	{
		d3d11_device->CreateRenderTargetView(d3d11_back_buffer, nullptr, &d3d11_render_target_view);
		d3d11_back_buffer->Release();
	}
}

void ethic::utils::overlay::cleanup_render_target()
{
	if (d3d11_render_target_view)
	{
		d3d11_render_target_view->Release();
		d3d11_render_target_view = nullptr;
	}
}

LRESULT __stdcall ethic::utils::overlay::window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (d3d11_device != nullptr && wParam != SIZE_MINIMIZED)
		{
			cleanup_render_target();
			dxgi_swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			create_render_target();
		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
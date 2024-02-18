#include "globals.hpp"
ethic::roblox::instance_t globals::datamodel{};
ethic::roblox::instance_t globals::players{};
ethic::roblox::instance_t globals::visualengine{};
ethic::roblox::instance_t globals::placeid{};
//ethic::roblox::instance_t globals::camera{};
//ethic::roblox::instance_t globals::pf{};

bool globals::sticky_aim = true;

bool globals::esp = false;
bool globals::box = false;
bool globals::name_esp = false;

bool globals::tracers = false;
bool globals::healthinfo = false;
bool globals::healthbar = false;
bool globals::chams = false;

float globals::box_color[3] = { 5, 102, 10 };
float globals::name_color[3] = { 255, 255, 255 };
float globals::tracers_color[3] = { 255, 255, 255 };

bool globals::prediction = false;
bool globals::vsync = false;
bool globals::streamproof = false;
bool globals::aimbot = false;
bool globals::autologin = false;
bool globals::shake = false;
bool globals::triggerbot = false;
bool globals::head_circle = false;

std::string globals::game = "Universal"; 

int globals::boxtype = 0;
int globals::aimmethod = 0;
int globals::aimpart = 0;

int globals::x_offset = 0;
int globals::y_offset = 0;

int globals::max_dist = 10000;

float globals::smoothness_x = 2;
float globals::smoothness_y = 1;

float globals::prediction_x = 3;
float globals::prediction_y = 3;

float globals::shake_value = 100;

float globals::range = 0;
float globals::range_mult = 0;

float globals::sensitivity = 0.5f;
int globals::fov = 100;
bool globals::fov_on = false;
bool globals::disable_outside_fov = false;
bool globals::show_auth = false;

bool globals::healthcheck = false;
bool globals::knock_check = false;
bool globals::team_check = false;
bool globals::team_check_esp = false;

CKeybind globals::aimbotkey{ "aimkey" };
TextEditor globals::lua_editor = TextEditor();
ethic::LuaVM globals::luaVM = ethic::LuaVM();
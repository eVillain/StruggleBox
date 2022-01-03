#pragma once

#include <string>

const std::string KEY_ESCAPE = "Escape";
const std::string KEY_RETURN = "Return";
const std::string KEY_DELETE = "Delete";
const std::string KEY_BACKSPACE = "BackSpace";
const std::string KEY_UP = "Up";
const std::string KEY_DOWN = "Down";
const std::string KEY_LEFT = "Left";
const std::string KEY_RIGHT = "Right";
const std::string KEY_SPACE = "Space";
const std::string KEY_W = "W";
const std::string KEY_A = "A";
const std::string KEY_S = "S";
const std::string KEY_D = "D";
const std::string KEY_P = "P";
const std::string KEY_C = "C";
const std::string KEY_E = "E";
const std::string KEY_M = "M";
const std::string KEY_R = "R";
const std::string KEY_I = "I";
const std::string KEY_G = "G";
const std::string KEY_1 = "1";
const std::string KEY_2 = "2";
const std::string KEY_3 = "3";
const std::string KEY_4 = "4";
const std::string KEY_5 = "5";
const std::string KEY_6 = "6";
const std::string KEY_7 = "7";
const std::string KEY_8 = "8";
const std::string KEY_9 = "9";
const std::string KEY_0 = "0";
const std::string KEY_TILDE = "~";
const std::string KEY_TAB = "Tab";
const std::string KEY_LEFT_SHIFT = "Left Shift";
const std::string KEY_LEFT_ALT = "Left Alt";
const std::string KEY_RIGHT_SHIFT = "Left Option";
const std::string MOUSE_BUTTON_1 = "MouseButton1";
const std::string MOUSE_BUTTON_2 = "MouseButton2";
const std::string MOUSE_BUTTON_3 = "MouseButton3";
const std::string MOUSE_WHEEL_Y = "MouseWheelY";
const std::string MOUSE_WHEEL_X = "MouseWheelX";

enum class InputEvent {
	Back,
	Start,
	Pause,
	Console,
	Stats,
	Erase_Right,
	Erase_Left,
	Look_Up,
	Look_Down,
	Look_Left,
	Look_Right,
	Move_Forward,
	Move_Backward,
	Move_Left,
	Move_Right,
	Jump,
	Run,
	Sneak,
	Grab,
	Inventory,
	Shoot,
	Aim,
	Scroll_Y,
	Edit_Blocks_Replace,
	Edit_Mode_Blocks,
	Edit_Mode_Object,
	Edit_Grab_Cursor
};

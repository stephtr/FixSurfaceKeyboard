#pragma once

#include "stdafx.h"

enum KeyboardType {
	KT_UNKNOWN,
	KT_SURFACE_TYPECOVER_3,
	KT_SURFACE_TYPECOVER_4,
	KT_SURFACE_TYPECOVER_2017,
	KT_SURFACEBOOK_1,
	KT_SURFACE_KEYBOARD // Surface Keyboard and Surface Ergonomic Keyboard
};

enum PressedKey {
	PK_LSHIFT = 1 << 0,
	PK_RSHIFT = 1 << 1,
	PK_LCTRL = 1 << 2,
	PK_RCTRL = 1 << 3,
	PK_LALT = 1 << 4,
	PK_RALT = 1 << 5,
	PK_LWIN = 1 << 6,
	PK_RWIN = 1 << 7,
};

int getModifierState() {
	int result = 0;
	if (GetKeyState(VK_LSHIFT) < 0) result |= PK_LSHIFT;
	if (GetKeyState(VK_RSHIFT) < 0) result |= PK_RSHIFT;
	if (GetKeyState(VK_LCONTROL) < 0) result |= PK_LCTRL;
	if (GetKeyState(VK_RCONTROL) < 0) result |= PK_RCTRL;
	if (GetKeyState(VK_LMENU) < 0) result |= PK_LALT;
	if (GetKeyState(VK_RMENU) < 0) result |= PK_RALT;
	if (GetKeyState(VK_LWIN) < 0) result |= PK_LWIN;
	if (GetKeyState(VK_RWIN) < 0) result |= PK_RWIN;
	return result;
}

typedef std::unordered_map<DWORD, DWORD> KeyboardMap;
typedef DWORD(*KeyboardCallback)(DWORD);

// problem: for example Type Cover 3: L_ALT + "F5" = L_SHIFT + "F6" = L_ALT + L_SHIFT + "F8"

std::unordered_map<KeyboardType, KeyboardMap> g_keymaps = {
	{ KT_SURFACE_TYPECOVER_3, {
		{ VK_VOLUME_MUTE, VK_F3 },
		{ VK_MEDIA_PLAY_PAUSE, VK_F4 },
		// L_SHIFT, L_WIN, F21 => F5
		// L_ALT, L_WIN, F21 => F6
		// L_CTRL, L_WIN, VK_SEPARATOR => F7
		// L_WIN, F21 => F8
} },
{ KT_SURFACE_TYPECOVER_4, {
	{ VK_MEDIA_PLAY_PAUSE, VK_F3 },
	{ VK_VOLUME_MUTE, VK_F4 },
	{ VK_VOLUME_DOWN, VK_F5 },
	{ VK_VOLUME_UP, VK_F6 }
} },
{ KT_SURFACE_TYPECOVER_2017, {
	{ VK_MEDIA_PLAY_PAUSE, VK_F3 },
	{ VK_VOLUME_MUTE, VK_F4 },
	{ VK_VOLUME_DOWN, VK_F5 },
	{ VK_VOLUME_UP, VK_F6 }
} },
{ KT_SURFACE_KEYBOARD, {
	{ VK_VOLUME_MUTE, VK_F1 },
	{ VK_VOLUME_DOWN, VK_F2 },
	{ VK_VOLUME_UP, VK_F3 },
	{ VK_MEDIA_PREV_TRACK, VK_F4 },
	{ VK_MEDIA_PLAY_PAUSE, VK_F5 },
	{ VK_MEDIA_NEXT_TRACK, VK_F6 }
	// L_WIN, L_SHIFT, F21 => F9
	// L_WIN, TAB => F10
	// L_WIN, R_CTRL, F21 => F11
	// L_WIN, F21 => F12
} },
{ KT_SURFACEBOOK_1, {
	{ VK_MEDIA_PLAY_PAUSE, VK_F3 },
	{ VK_VOLUME_MUTE, VK_F4 },
	{ VK_VOLUME_DOWN, VK_F5 },
	{ VK_VOLUME_UP, VK_F6 }
} }
};

bool handleKey_SurfaceKeyboard(DWORD vk, int modifierState, bool keydown);

void SendKey(WORD vk, bool keydown = true) {
	INPUT generated;
	generated.type = INPUT_KEYBOARD;
	generated.ki.wVk = vk;
	generated.ki.wScan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
	generated.ki.dwFlags = keydown ? 0 : KEYEVENTF_KEYUP;
	generated.ki.time = 0;
	generated.ki.dwExtraInfo = 0;
#ifdef _DEBUG
	std::cout << "generate keyboard event" << std::endl;
#endif
	SendInput(1, &generated, sizeof(INPUT));
}

bool handleKey_map(DWORD vk, KeyboardType type, PKBDLLHOOKSTRUCT data) {
	KeyboardMap::const_iterator got = g_keymaps[type].find(vk);
	int modifierState = getModifierState();
	bool keydown = !(data->flags & LLKHF_UP);

	if (modifierState == 0) return false;
	if (got != g_keymaps[type].end() && data->flags & LLKHF_EXTENDED) {
		SendKey((WORD)got->second, keydown);
		return true;
	}
	switch (type)
	{
	case KT_SURFACE_KEYBOARD: return handleKey_SurfaceKeyboard(vk, modifierState, keydown);
	}
	return false;
}

bool handleKey_SurfaceKeyboard(DWORD vk, int modifierState, bool keydown) {
	bool handleKey = false;

	// When switching to German keyboard layout,
	// pressing the action center key results in Shift + Ctrl + Alt + Win + F21,
	// which (obviously) doesn't open the action center.
	static bool blockActionCenter = false;
	if (keydown
		&& vk == VK_F21 && modifierState == (PK_RSHIFT | PK_LCTRL | PK_RALT | PK_LWIN)
		&& !blockActionCenter)
	{
		ShellExecute(0, 0, L"ms-actioncenter:", 0, 0, SW_HIDE);
		blockActionCenter = true;
		return true;
	}
	if (!keydown && vk == VK_LWIN)
		blockActionCenter = false;

	return handleKey;
}
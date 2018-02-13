#pragma once

#include "stdafx.h"
#include "KeyboardUtils.h"


std::unordered_map<HANDLE, KeyboardType> g_deviceTypes;
KeyboardType g_lastUsedKeyboard = KT_UNKNOWN;

KeyboardType identifyDevice(wchar_t* name) {
	size_t len = wcslen(name);
	if (wcsstr(name, L"VID_045E")) {
		if (wcsstr(name, L"PID_07DC")) return KT_SURFACE_TYPECOVER_3;
		if (wcsstr(name, L"PID_07E8")) return KT_SURFACE_TYPECOVER_4;
		if (wcsstr(name, L"PID_09C0")) return KT_SURFACE_TYPECOVER_2017;
		if (wcsstr(name, L"PID_07CD")) return KT_SURFACEBOOK_1;
	}
	if (wcsstr(name, L"_VID&02045E") || wcsstr(name, L"_VID&02045e")) {
		if (wcsstr(name, L"_PID&0817")) return KT_SURFACE_KEYBOARD; // Surface Ergonomic Keyboard
		if (wcsstr(name, L"_PID&0917")) return KT_SURFACE_KEYBOARD; // Surface Keyboard
	}
	return KT_UNKNOWN;
}

void handleRawInput(HRAWINPUT lParam)
{
	UINT cbSize = 0;
	if (GetRawInputData(lParam, RID_INPUT, NULL, &cbSize, sizeof(RAWINPUTHEADER)) != 0) return;
	LPBYTE lpb = new BYTE[cbSize];
	if (GetRawInputData(lParam, RID_INPUT, lpb, &cbSize, sizeof(RAWINPUTHEADER)) != cbSize) {
		delete[] lpb;
		return;
	}
	PRAWINPUT raw = (PRAWINPUT)lpb;
	if (raw->header.dwType == RIM_TYPEKEYBOARD) {
		auto got = g_deviceTypes.find(raw->header.hDevice);
		if(got != g_deviceTypes.end()) g_lastUsedKeyboard = got->second;
	}
	delete[] lpb;
	return;
}

void handleDeviceAdd(HANDLE hDevice)
{
	UINT cbSize = 0;
	if (GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, NULL, &cbSize) != 0) return;
#pragma warning( push )
#pragma warning( disable : 6001 )
	wchar_t *deviceName = new wchar_t[cbSize];
	if (GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, deviceName, &cbSize) != cbSize) {
		delete[] deviceName;
		return;
	}
#pragma warning( pop )
	KeyboardType type = identifyDevice(deviceName);
	if (type != KT_UNKNOWN) {
		g_deviceTypes[hDevice] = type;
	}
#ifdef _DEBUG
	std::wcout << (type == KT_UNKNOWN ? "unknown" : "known") << " keyboard added: " << deviceName << std::endl;
#endif
	delete[] deviceName;
}

void handleDeviceRemove(HANDLE hDevice) {
	g_deviceTypes.erase(hDevice);
#ifdef _DEBUG
	std::cout << "keyboard removed" << std::endl;
#endif
}

bool handleRawMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INPUT: {
		handleRawInput((HRAWINPUT)lParam);
		return true;
	}
	case WM_INPUT_DEVICE_CHANGE: {
		if (wParam == GIDC_ARRIVAL) handleDeviceAdd((HANDLE)lParam);
		if (wParam == GIDC_REMOVAL) handleDeviceRemove((HANDLE)lParam);
		return true;
	}
	}
	return true;
}
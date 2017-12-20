#include "stdafx.h"

#include "KeyboardUtils.h"
#include "RawInputHelper.h"
#include "Installer.h"

#include <ctime>

LRESULT CALLBACK keyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	bool removeKey = false;
	if (nCode == HC_ACTION && g_lastUsedKeyboard != KT_UNKNOWN) {
		PKBDLLHOOKSTRUCT data = (PKBDLLHOOKSTRUCT)lParam;
		removeKey = handleKey_map(data->vkCode, g_lastUsedKeyboard, data);
#ifdef _DEBUG
		if (!removeKey) {
			std::cout << (data->flags & LLKHF_UP ? "UP" : "DOWN") << " rFlags=" << (data->flags & ~LLKHF_UP & ~LLKHF_INJECTED) << " extraInfo=" << data->dwExtraInfo << " scanCode=" << data->scanCode << " VK=" << data->vkCode << std::endl;
		}
#endif
	}
	return removeKey ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(int argc, char* argv[])
{
	if (argc == 2) {
		if (strcmp(argv[1], "/Install") == 0 || strcmp(argv[1], "/install") == 0) {
			return scheduleTask(true, argv[0]);
		}
		if (strcmp(argv[1], "/Uninstall") == 0 || strcmp(argv[1], "/uninstall") == 0) {
			return scheduleTask(false);
		}
	}

	//logfile.open("C:\\Users\\steph\\Desktop\\FixSurfaceKeyboard.log", std::ios_base::app);
	//time_t time = std::time(0);
	//logfile << "Starting at " << time << std::endl;

	HWND hwnd = CreateWindow(L"STATIC", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

	RAWINPUTDEVICE* rid = new RAWINPUTDEVICE[1];
	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x06;
	rid[0].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
	rid[0].hwndTarget = hwnd;

	if (!RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE))) return -1;

	HHOOK hhkKeyboard = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHook, NULL, 0);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		if (!handleRawMessage(msg.message, msg.wParam, msg.lParam)) break;
	}
	UnhookWindowsHookEx(hhkKeyboard);
	return msg.wParam;
}
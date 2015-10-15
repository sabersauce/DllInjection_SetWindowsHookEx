//keyboard hook
//injector

//using SetWindowsHookEx()

#include <stdio.h>
#include <conio.h>
#include <Windows.h>

#define DLL_NAME "keyhook.dll"

typedef int(*pfnHook)();

int
main(int argc, char *argv[]) {
	if (argc != 3) {
		if (argc != 1) printf("Wrong parameters.\n\n");
		printf("Usage:injector.exe <ProcessName> <DataDestinationPath>\n");
		printf("Do not run as administrator unless you are going to inject the DLL into a system program.\n");
		return -1;
	}

	HANDLE file = INVALID_HANDLE_VALUE;
	file = CreateFileA(argv[2], GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file != INVALID_HANDLE_VALUE) {
		printf("Destination file already exists!\n");
		CloseHandle(file);
		return -1;
	}
	file = CreateFileA(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		printf("Destination unwritable!\n");
		printf("Please notice that you should use '\\\\' for '\\'.\n");
		return -1;
	}
	CloseHandle(file);
	file = INVALID_HANDLE_VALUE;

	HMODULE hookDll = LoadLibraryA(DLL_NAME);
	if (hookDll == NULL) {
		printf("Load keyhook.dll failed.\n");
		return -1;
	}
	pfnHook hookStart = (pfnHook)GetProcAddress(hookDll, "HookStart");
	pfnHook hookStop = (pfnHook)GetProcAddress(hookDll, "HookStop");

	char filePath[MAX_PATH] = { 0 };
	char parameters[MAX_PATH * 2 + 1] = { 0 };
	strcat(parameters, argv[1]);
	parameters[strlen(parameters)] = '|';
	strcat(parameters, argv[2]);

	GetTempPathA(MAX_PATH - 10, filePath);
	strcat(filePath, "dllinject");

	file = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		printf("Open file in temp path error.\n");
		return -1;
	}
	DWORD bytesWritten = 0;
	BOOL writeState = WriteFile(file, parameters, strlen(argv[1]) + strlen(argv[2]) + 2, &bytesWritten, NULL);
	if ((bytesWritten != strlen(argv[1]) + strlen(argv[2]) + 2) || writeState == FALSE) {
		printf("Write file in temp path error.\n");
		CloseHandle(file);
		return -1;
	}
	CloseHandle(file);

	if (hookStart() == 0) {
		printf("Start hooking.\n");
		printf("Press any key to stop.\n");
		getch();
		if (hookStop() == 0) {
			printf("Stop hooking.\n");
			FreeLibrary(hookDll);
			return 0;
		}
		else {
			printf("Stop hooking error.\n");
			FreeLibrary(hookDll);
			return -1;
		}
	}
	else {
		printf("Start hooking error.\n");
		return -1;
	}
}
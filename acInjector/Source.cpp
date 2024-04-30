#include <iostream>
#include <Windows.h>

DWORD GetProcId(const char* windowName)
{
	// must change properties to MULTIBYTE (advanced/character set/multibyte)
	HWND acWindow = FindWindow(nullptr, windowName);

	DWORD processId = {0};
	GetWindowThreadProcessId(acWindow, &processId);

	return processId;
}

int main()
{
	const char* dllPath = "change_this_to_your_dll_path";
	const char* windowName = "AssaultCube";

	DWORD processId = GetProcId(windowName);

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, NULL, processId);

	void* spaceForDllPath = VirtualAllocEx(hProc, nullptr, strlen(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	
	WriteProcessMemory(hProc, spaceForDllPath, dllPath, strlen(dllPath), nullptr);

	HANDLE hThread = CreateRemoteThread(hProc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryA, spaceForDllPath, NULL, nullptr);

	CloseHandle(hThread);
	CloseHandle(hProc);
}
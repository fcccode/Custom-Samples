// dllmain.cpp : 定义 DLL 应用程序的入口点。
//#include "stdafx.h"
#include<windows.h>
#include<commctrl.h>
#include<winuser.h>

#pragma data_seg("Shared")
HWND Icons = 0;
HWND WorkerW = 0; 
HWND Recv = 0;
HINSTANCE Me = 0;
HHOOK hHook = 0;
UINT MsgCode = 0;
#pragma data_seg()

LRESULT CALLBACK MouseProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	
	//according to MSDN, when nCode<0, do nothing
	if (nCode < 0)return CallNextHookEx(hHook, nCode, wParam, lParam);
	
	//when middle button is released, hide the desktop.
	if (wParam == WM_MBUTTONUP) {
		ShowWindow(WorkerW, SW_HIDE);
		return 1;
	}

	//passing mouse actions: it can not handle properly when you drag.
	if ((wParam == WM_LBUTTONDOWN)|| (wParam == WM_LBUTTONUP)|| (wParam == WM_LBUTTONDBLCLK)||(wParam==WM_MOUSEMOVE)) {
		MOUSEHOOKSTRUCT *in = reinterpret_cast<MOUSEHOOKSTRUCT*>(lParam);
		if(in->pt.x)
		PostMessage(Recv, MsgCode, wParam, MAKELPARAM(in->pt.x, in->pt.y));
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

BOOL FindWindows(_Out_ HWND *hDesktop, _Out_ HWND *hWorkerW, _Out_ HWND *hIcons) {

	//Finding the desktop
	*hDesktop = FindWindowA("Progman", "Program Manager");

	//Separating
	SendMessageA(*hDesktop, WM_USER + 300, 0, 0);

	//Looking for Icons.
	HWND loop = FindWindowExA(nullptr, nullptr, "WorkerW", nullptr);
	while (loop) {
		HWND tmp = FindWindowExA(loop, nullptr, "SHELLDLL_DefView", nullptr);
		if (tmp == nullptr) {
			//Simply unnecessary work.
			ShowWindow(loop, SW_HIDE);
		}
		else {
			//Found!
			*hWorkerW = loop;
			*hIcons = tmp;
		}
		loop = FindWindowExA(nullptr, loop, "WorkerW", nullptr);
	}

	//Looking for errors.
	if (*hDesktop == 0)return FALSE;
	if (*hWorkerW == 0)return FALSE;
	if (*hIcons == 0)return FALSE;

	return TRUE;
}

BOOL StartHook(_In_ HWND pIcons, _In_ HWND pWorkerW, _In_ HWND pRecv) {
	
	//Hook into the window with icons.
	DWORD tid = GetWindowThreadProcessId(pIcons, NULL);
	if (tid == 0)return FALSE;
	hHook = SetWindowsHookEx(WH_MOUSE, MouseProc, Me, tid);
	if (hHook == 0)return FALSE;

	//make sure the icons are shown.
	ShowWindow(WorkerW, SW_SHOW);

	//Retrieve necessary data.
	Icons = pIcons;
	WorkerW = pWorkerW;
	Recv = pRecv;
	
	return TRUE;
}

void StopHook(){
	
	//clear data up.
	Icons = 0;
	WorkerW = 0;
	Recv = 0;
	
	//make sure that the icons are shown
	ShowWindow(WorkerW, SW_SHOW);

	//release the hooks
	UnhookWindowsHookEx(hHook);
	hHook = 0;
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL,_In_ DWORD fdwReason,_In_ LPVOID lpvReserved) {
	Me = hinstDLL;
	MsgCode = RegisterWindowMessageA("CustomDesktopBackground");
	if (MsgCode == 0)return FALSE;
	return TRUE;
}

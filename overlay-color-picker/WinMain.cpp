#include <stdio.h>

#include "GUI/GUI.h"
#include "Window/Window.h"
#include "TrayIcon/TrayIcon.h"

bool IsAltHolding = false;
bool IsLCtrlHolding = false;
bool IsCloseCalled = false;

extern bool IsExitCalled;

DWORD WINAPI create_overlay_thread(LPVOID lpParameter)
{
	while (true)
	{
		SHORT ALTKEY = GetAsyncKeyState(VK_MENU);

		if (ALTKEY < 0)
			IsAltHolding = true;
		else
			IsAltHolding = false;

		SHORT L_CTRL_KEY = GetAsyncKeyState(VK_LCONTROL);

		if (L_CTRL_KEY < 0)
			IsLCtrlHolding = true;
		else
			IsLCtrlHolding = false;

		Sleep(1);
	}

	return 0;
}

bool ForceToForeground(HWND hWnd)
{
	//https://stackoverflow.com/questions/22094330/setforgroundwindow-fails-after-program-was-updated

	HWND hForeground = GetForegroundWindow();

	int curThread = GetCurrentThreadId();
	int remoteThread = GetWindowThreadProcessId(hForeground, 0);

	AttachThreadInput(curThread, remoteThread, TRUE);
	SetForegroundWindow(hWnd);
	AttachThreadInput(curThread, remoteThread, FALSE);

	return GetForegroundWindow() == hWnd;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
//int main()
{
	HANDLE hThread = CreateThread(NULL, 0, create_overlay_thread, NULL, 0, NULL);

	//Create GUI Window
	Gui.OnCreateWindow(160, 230);

	//Main loop
	while (!IsExitCalled)
	{
		//reduce cpu usage
		Sleep(1);

		//Procedure mensage
		Win.OnUpdateMsg();

		if (IsAltHolding && IsLCtrlHolding)
		{
			// get the height and width of the screen
			int _width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
			int _height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

			//Create window
			Win.OnCreateWindow(_width, _height);

			//Show the gui
			ShowWindow(Gui.GetWindow(), SW_SHOW);

			//Create main loop
			while (IsAltHolding && IsLCtrlHolding)
			{
				//Set Gui to foreground
				if (GetForegroundWindow() != Gui.GetWindow())
				{
					//SetForegroundWindow the function sometimes fails
					ForceToForeground(Win.GetWindow());
					ForceToForeground(Gui.GetWindow());
				}

				//Render ImGui interface
				Gui.Render();

				//Procedure mensage
				Win.OnUpdateMsg();
			}

			//Hide the gui
			ShowWindow(Gui.GetWindow(), SW_HIDE);

			//Release
			Win.Release();
		}
	}

	//Release the Gui
	Gui.Release();

	return 0;
}
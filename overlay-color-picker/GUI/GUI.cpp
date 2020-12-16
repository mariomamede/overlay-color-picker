#include "GUI.h"

CInterface Gui;

#define SWM_EXIT	WM_APP + 1

bool IsExitCalled = false;
extern bool IsColorPicker;

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;
static ID3D11BlendState*		g_pBlendState = NULL;

const char* color_array_name[6] = { "color_0", "color_1", "color_2", "color_3", "color_4" };

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI GuiProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_APP:
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU: 
			Gui.ShowContextMenu(hWnd);
		}
		break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case SWM_EXIT:
			IsExitCalled = true;
			break;
		}
	}
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			Gui.CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			Gui.CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_LBUTTONDOWN:
	{
		if (Gui.IsMouseOnTitleBar())
		{
			ReleaseCapture();
			SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		}
		break;
	}
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

void CInterface::InitTrayIcon()
{
	NOTIFYICONDATA niData;

	ZeroMemory(&niData, sizeof(NOTIFYICONDATA));

	//NOTIFYICONDATA_V2_SIZE
	niData.cbSize = sizeof(NOTIFYICONDATA);

	// the ID number can be anything you choose
	niData.uID = 1;

	// state which structure members are valid
	niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	// load the icon
	niData.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));;

	niData.hWnd = hWnd;

	niData.uCallbackMessage = WM_APP;

	// tooltip message
	lstrcpyn(niData.szTip, "OCP - Color Picker", sizeof(niData.szTip) / sizeof(TCHAR));

	Shell_NotifyIcon(NIM_ADD, &niData);

	// free icon handle
	if (niData.hIcon && DestroyIcon(niData.hIcon))
		niData.hIcon = NULL;
}

void CInterface::ShowContextMenu(HWND hWnd)
{
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, "Exit");

		SetForegroundWindow(hWnd);

		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);

		DestroyMenu(hMenu);
	}
}


bool CInterface::OnCreateWindow(int width, int height)
{
	// Create application window
	wc =
	{
		sizeof(WNDCLASSEX),
		CS_CLASSDC,
		GuiProc,
		0L, 0L,
		GetModuleHandle(NULL),
		NULL,
		NULL,
		NULL,
		NULL,
		GUI_WINDOW_CLASS,
		NULL
	};

	if (!::RegisterClassEx(&wc))
		return false;

	hWnd = ::CreateWindowEx(WS_EX_TOPMOST, wc.lpszClassName, GUI_WINDOW_TEXT, WS_POPUP, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL);

	if (!hWnd)
		return false;

	// Initialize Direct3D
	if (!OnCreateDevice(hWnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return false;
	}

	// Show the window
	::ShowWindow(hWnd, SW_HIDE); //SW_SHOWDEFAULT
	::UpdateWindow(hWnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	OnSetupStyle();

#ifdef _DEBUG
	
#else
	OnSetupFont();
#endif // DEBUG

	InitTrayIcon();

	SetupWindowPos(0, 0);

	SetupWindowSize(width, height);

	color = ImColor(0xad1457);
	color_array = { 0xad1457, 0x6a1b9a, 0x4527a0, 0xf283593, 0x1565c0 };

	return true;
}

bool CInterface::OnCreateDevice(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CInterface::Render()
{
	// Our state
	ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	OnBuildInterface();

	//Get asyncrono key state
	static bool once = false;

	if (!IsColorPicker)
		once = false;

	if (IsColorPicker && !once)
	{
		once = true;

		if (!IsMouseOnGui())
		{
			ImColor Color = Misc.GetColorOnMousePosition();

			if (Color != color_array.at(0))
			{
				UpdateColor(Color);

				Misc.CopyToClipboard(Color);

				color_array.insert(color_array.begin(), Color);
			}
		}
	}

	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	g_pSwapChain->Present(1, 0); // Present with vsync
}

void CInterface::Release()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hWnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void CInterface::CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CInterface::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CInterface::CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void CInterface::OnSetupStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	//style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 0.0f;
	style->WindowBorderSize = 0.0f;
	style->PopupBorderSize = 0.0f;

	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 10.0f;
	style->ItemSpacing = ImVec2(5, 8);
	style->ItemInnerSpacing = ImVec2(0, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

void CInterface::OnSetupFont()
{
	ImGuiIO& io = ImGui::GetIO();

	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;

	// Load Fonts
	io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 14.0f);
	ImFont* FontAwesome = io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 14.0f, &icons_config, icons_ranges); // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid
}

void CInterface::OnBuildInterface()
{
	ImGui::SetNextWindowSize(window_size);
	ImGui::SetNextWindowPos(window_pos);

	ImGui::Begin(" ", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
	{
		ImGuiStyle* style = &ImGui::GetStyle();

		style->FrameRounding = 0.f;
		ImGui::ColorButton("##preview", color, ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoTooltip, ImVec2(window_size.x - (ImGui::GetStyle().WindowPadding.x * 2), 140));
		style->FrameRounding = 20.f;

		char text_formart[20];
		sprintf_s(text_formart, 20, "#%06X  %s", Misc.ImColorToHex(color), ICON_FA_COPY);

		if (ImGui::Button(text_formart, ImVec2(-1, 25)))
			Misc.CopyToClipboard(color);

		ImGui::Separator();

		if (color_array.size() > 5)
			color_array.erase(color_array.begin() + 5, color_array.end());

		//Color history array
		for (size_t i = 0; i < 5; i++)
		{
			if (ImGui::ColorButton(color_array_name[i], color_array.at(i), ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoTooltip))
				UpdateColor(color_array.at(i)), Misc.CopyToClipboard(color_array.at(i));

			ImGui::SameLine();
		}

	} ImGui::End();
}

bool CInterface::IsMouseOnGui()
{
	ImVec2 mouse_pos = ImGui::GetMousePos();

	if (mouse_pos.x >= window_pos.x && mouse_pos.x <= window_size.x &&
		mouse_pos.y >= window_pos.y && mouse_pos.y <= window_size.y)
		return true;

	return false;
}

bool CInterface::IsMouseOnTitleBar()
{
	ImVec2 mouse_pos = ImGui::GetMousePos();

	if (mouse_pos.x >= window_pos.x && mouse_pos.x <= window_size.x &&
		mouse_pos.y >= window_pos.y && mouse_pos.y <= window_pos.y + 160.f)
		return true;

	return false;
}
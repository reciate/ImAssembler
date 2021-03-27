#include <Windows.h>
#include <iostream>
#include <d3d11.h>

#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

#pragma comment (lib, "d3d11.lib")

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

IDXGISwapChain* swap_chain{};
ID3D11Device* device{};
ID3D11DeviceContext* device_context{};
ID3D11RenderTargetView* back_buffer{};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);

	switch (uMsg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void initialize_dx(HWND hwnd) {
	DXGI_SWAP_CHAIN_DESC swap_chain_desc { 
		{
			0,
			0,
			{},
			DXGI_FORMAT_R8G8B8A8_UNORM
		},
		{4},
		DXGI_USAGE_RENDER_TARGET_OUTPUT,
		1,
		hwnd,
		true
	};

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, NULL, &device_context);

	ID3D11Texture2D* p_back_buffer{};

	swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&p_back_buffer));

	device->CreateRenderTargetView(p_back_buffer, NULL, &back_buffer);

	p_back_buffer->Release();

	device_context->OMSetRenderTargets(1, &back_buffer, NULL);
}

void cleanup_dx() {
	device_context->Release();
	device->Release();
	swap_chain->Release();
	back_buffer->Release();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSA window_class{ 0, WindowProc, 0, 0, hInstance, 0, 0, 0, 0, "WindowClass" };

	RegisterClassA(&window_class);

	HWND hwnd{ CreateWindowExA(0, "WindowClass", "ImAssembler", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL) };

	if (!hwnd) return 0;

	ShowWindow(hwnd, nCmdShow);

	initialize_dx(hwnd);

	ImGui::CreateContext();
	ImGuiIO& io{ ImGui::GetIO() };
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = NULL;

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device, device_context);

	MSG message{};

	while (GetMessageA(&message, NULL, 0, 0)) {

		float colour[]{ 0.0f, 0.0f, 0.0f, 255.0f };
		device_context->ClearRenderTargetView(back_buffer, colour);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swap_chain->Present(0, 0);

		TranslateMessage(&message);
		DispatchMessageA(&message);
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	cleanup_dx();

	return 0;
}
#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <Zydis/Zydis.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_dx11.h"

#pragma comment (lib, "d3d11.lib")

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

IDXGISwapChain* swap_chain{};
ID3D11Device* device{};
ID3D11DeviceContext* device_context{};

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
}

void cleanup_dx() {
	device_context->Release();
	device->Release();
	swap_chain->Release();
}

void dis(std::string& buffer) {
	ZydisDecoder decoder{};
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
	ZydisFormatter formatter{};
	ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

	std::istringstream string_stream{ buffer };
	std::string instruction_string{};
	std::vector<char> instructions{};

	while (getline(string_stream, instruction_string, ',')) {
		char b{ static_cast<char>(std::stoi(instruction_string, 0, 16)) };
		instructions.push_back(b);
	}

	buffer.clear();

	int offset{};
	ZydisDecodedInstruction instruction{};

	while (offset < instructions.size() * sizeof(char)) {
		ZydisDecoderDecodeBuffer(&decoder, instructions.data() + offset, 16, &instruction);

		char buf[256]{};
		ZydisFormatterFormatInstruction(&formatter, &instruction, buf, sizeof(buf), ZYDIS_RUNTIME_ADDRESS_NONE);

		std::cout << buffer << '\n';

		offset += instruction.length;

		buffer += buf;
		buffer += '\n';
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSA window_class{ 0, WindowProc, 0, 0, hInstance, 0, 0, 0, 0, "WindowClass" };

	RegisterClassA(&window_class);

	HWND hwnd{ CreateWindowExA(0, "WindowClass", "ImAssembler", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, hInstance, NULL) };

	if (!hwnd) return 0;

	ShowWindow(hwnd, 0);

	initialize_dx(hwnd);

	ImGui::CreateContext();
	ImGuiIO& io{ ImGui::GetIO() };
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = NULL;

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device, device_context);

	MSG message{};
	bool is_open{ true };

	std::string buffer{};

	while (message.message != WM_QUIT) {
		if (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessageA(&message);
			continue;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		RECT window_rect{};
		GetWindowRect(hwnd, &window_rect);

		ImGui::SetNextWindowSize({ 800, 600 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_FirstUseEver);

		ImGui::Begin("ImAssembler", &is_open, ImGuiWindowFlags_NoResize);

		ImGui::InputTextMultiline("Input", &buffer);
		if (ImGui::Button("Disassemble")) dis(buffer);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		swap_chain->Present(1, 0);

		if (!is_open) PostMessageA(hwnd, WM_QUIT, NULL, NULL);

		Sleep(1);
	}

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	cleanup_dx();

	return 0;
}
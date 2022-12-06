#pragma once

#include <windows.h>
#include <d2d1_3.h>
#include <wincodec.h>
#include <winrt/base.h>
#include <d3d11_4.h>

class Clock {
public:
    Clock();

    void Initialize(HINSTANCE instance, INT cmd_show);
    void RunMessageLoop();

private:
    // Window handle.
    HWND hwnd;

    // Direct3D objects.
    winrt::com_ptr<ID3D11Device5> d3d_device;
    winrt::com_ptr<ID3D11DeviceContext4> d3d_context;
    winrt::com_ptr<IDXGISwapChain4> swap_chain;

    // Direct2D objects.
    winrt::com_ptr<ID2D1Factory7> d2d_factory;
    winrt::com_ptr<ID2D1Device6> d2d_device;
    winrt::com_ptr<ID2D1DeviceContext6> d2d_context;
    winrt::com_ptr<ID2D1Bitmap1> d2d_target_bitmap;

    winrt::com_ptr<IWICImagingFactory2> imaging_factory;

    static constexpr D2D1_COLOR_F background_color = { .r = 0.62f, .g = 0.38f, .b = 0.62f, .a = 1.0f };

    void CreateDeviceDependentResources();
    void CreateDeviceIndependentResources();
    void CreateWindowSizeDependentResources();
    void HandleDeviceLost();

    void OnRender();

    void OnResize(UINT width, UINT height);

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
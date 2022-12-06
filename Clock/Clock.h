#pragma once

#include "BitmapDefinition.h"
#include <windows.h>
#include <d2d1_3.h>
#include <wincodec.h>
#include <winrt/base.h>
#include <d3d11_4.h>

class Clock {
public:
    Clock();

    void InitializeWindow(HINSTANCE instance, INT cmd_show);
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
    BitmapDefinition clock_bitmap_def;
    BitmapDefinition digits_bitmap_def;

    D2D1_MATRIX_3X2_F transformation;
    D2D1_POINT_2F center;

    D2D1_RECT_F clock_dest_rect;
    D2D1_RECT_F dots_src_rect = D2D1::RectF(
        1080.0f,
        0.0f,
        1180.0f,
        192.0f
    );
    D2D1_RECT_F dots_dest_rect;

    bool show_dots = true;

    int hour = 13;
    int minute = 47;
    int second = 18;

    static constexpr D2D1_COLOR_F background_color = { .r = 0.62f, .g = 0.38f, .b = 0.62f, .a = 1.0f };

    void CreateDeviceDependentResources();
    void CreateDeviceIndependentResources();
    void CreateWindowSizeDependentResources();
    void HandleDeviceLost();

    void OnRender();

    void OnResize(UINT width, UINT height);

    void RenderTime();
    void RenderDigit(int digit, const D2D1_RECT_F& dest_rect);

    void IncreaseTime();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
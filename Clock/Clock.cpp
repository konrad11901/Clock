#include "Clock.h"
#include "RandomGenerator.h"
#include <dxgi1_6.h>

Clock::Clock() : hwnd(nullptr), clock_bitmap_def(L"Assets\\Clock.png"), digits_bitmap_def(L"Assets\\Digits.png"),
    transformation(), center(), clock_dest_rect(), dots_dest_rect() {
    RandomGenerator random_generator;
    hour = random_generator.GetRandomNumber(0, 23);
    minute = random_generator.GetRandomNumber(0, 59);
    second = random_generator.GetRandomNumber(0, 59);
}

void Clock::InitializeWindow(HINSTANCE instance, INT cmd_show) {
    CreateDeviceIndependentResources();

    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Clock::WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = instance;
    wcex.hCursor = LoadCursor(nullptr, IDI_APPLICATION);
    wcex.lpszClassName = L"D2DClock";

    winrt::check_bool(RegisterClassEx(&wcex));

    hwnd = CreateWindowEx(
        0,                      // Optional window styles
        L"D2DClock",            // Window class
        L"Direct2D Clock",      // Window text
        WS_OVERLAPPEDWINDOW,    // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        nullptr,    // Parent window    
        nullptr,    // Menu
        instance,   // Instance handle
        this        // Additional application data
    );

    winrt::check_pointer(hwnd);

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();

    ShowWindow(hwnd, cmd_show);
}

void Clock::RunMessageLoop() {
    SetTimer(hwnd, 1, 200, nullptr);

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Clock::CreateDeviceIndependentResources() {
    D2D1_FACTORY_OPTIONS options{};

    // Initialize the Direct2D Factory.
    winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, d2d_factory.put()));

    winrt::check_hresult(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

    // Initialize the Windows Imaging Component (WIC) Factory.
    winrt::check_hresult(CoCreateInstance(
        CLSID_WICImagingFactory2,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(imaging_factory.put())
    ));

    clock_bitmap_def.CreateDeviceIndependentResources(imaging_factory.get());
    digits_bitmap_def.CreateDeviceIndependentResources(imaging_factory.get());
}


void Clock::CreateDeviceDependentResources() {
    // This flag adds support for surfaces with a different color channel ordering
    // than the API default. It is required for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    // This array defines the set of DirectX hardware feature levels this app will support.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    winrt::com_ptr<ID3D11Device> device;
    winrt::com_ptr<ID3D11DeviceContext> context;

    auto hr = D3D11CreateDevice(
        nullptr,                    // Specify nullptr to use the default adapter.
        D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
        0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
        creationFlags,              // Set Direct2D compatibility flag.
        featureLevels,              // List of feature levels this app can support.
        ARRAYSIZE(featureLevels),   // Size of the list above.
        D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Runtime apps.
        device.put(),               // Returns the Direct3D device created.
        nullptr,                    // Don't store feature level of device created.
        context.put()               // Returns the device immediate context.
    );

    if (FAILED(hr))
    {
        // If the initialization fails, fall back to the WARP device.
        // For more information on WARP, see: 
        // http://go.microsoft.com/fwlink/?LinkId=286690
        winrt::check_hresult(D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
            0,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            device.put(),
            nullptr,
            context.put()
        ));
    }

    // Store pointers to the Direct3D 11.1 API device and immediate context.
    device.as(d3d_device);
    context.as(d3d_context);

    // Create the Direct2D device object and a corresponding context.
    winrt::com_ptr<IDXGIDevice4> dxgi_device;
    d3d_device.as(dxgi_device);

    winrt::check_hresult(d2d_factory->CreateDevice(dxgi_device.get(), d2d_device.put()));
    winrt::check_hresult(d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2d_context.put()));

    clock_bitmap_def.CreateDeviceDependentResources(d2d_context.get());
    digits_bitmap_def.CreateDeviceDependentResources(d2d_context.get());
}

void Clock::CreateWindowSizeDependentResources() {
    d2d_context->SetTarget(nullptr);
    d2d_target_bitmap = nullptr;

    if (swap_chain) {
        // If the swap chain already exists, resize it.
        auto hr = swap_chain->ResizeBuffers(
            2,
            0,
            0,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            0
        );
        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            HandleDeviceLost();

            // Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else {
            winrt::check_hresult(hr);
        }
    }
    else {
        winrt::com_ptr<IDXGIDevice4> dxgi_device;
        d3d_device.as(dxgi_device);

        winrt::com_ptr<IDXGIAdapter> dxgi_adapter;
        winrt::check_hresult(dxgi_device->GetAdapter(dxgi_adapter.put()));

        winrt::com_ptr<IDXGIFactory7> dxgi_factory;
        winrt::check_hresult(dxgi_adapter->GetParent(IID_PPV_ARGS(dxgi_factory.put())));

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

        swapChainDesc.Width = 0;
        swapChainDesc.Height = 0;
        swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;

        winrt::com_ptr<IDXGISwapChain1> swap_chain1;
        dxgi_factory->CreateSwapChainForHwnd(d3d_device.get(), hwnd, &swapChainDesc, nullptr, nullptr, swap_chain1.put());
        swap_chain1.as(swap_chain);
    }

    winrt::com_ptr<IDXGISurface2> dxgi_back_buffer;
    winrt::check_hresult(swap_chain->GetBuffer(0, IID_PPV_ARGS(dxgi_back_buffer.put())));

    // Get screen DPI
    FLOAT dpiX, dpiY;
    dpiX = (FLOAT)GetDpiForWindow(GetDesktopWindow());
    dpiY = dpiX;

    // Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpiX, dpiY);

    d2d_context->CreateBitmapFromDxgiSurface(dxgi_back_buffer.get(), &bitmapProperties, d2d_target_bitmap.put());

    d2d_context->SetTarget(d2d_target_bitmap.get());

    center = D2D1::Point2F(d2d_context->GetSize().width / 2.0f, d2d_context->GetSize().height / 2.0f);
    dots_dest_rect = D2D1::RectF(
        center.x - 50.0f,
        center.y - 96.0f,
        center.x + 50.0f,
        center.y + 96.0f
    );
    auto bitmap_size = clock_bitmap_def.GetBitmap()->GetSize();
    clock_dest_rect = D2D1::RectF(
        center.x - bitmap_size.width / 2.0f,
        center.y - bitmap_size.height / 2.0f,
        center.x + bitmap_size.width / 2.0f,
        center.y + bitmap_size.height / 2.0f
    );
    transformation = D2D1::Matrix3x2F::Rotation(-5.0f, center);
}

void Clock::HandleDeviceLost() {
    swap_chain = nullptr;

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}

void Clock::OnRender() {
    d2d_context->BeginDraw();
    d2d_context->Clear(background_color);

    d2d_context->SetTransform(transformation);

    d2d_context->DrawBitmap(clock_bitmap_def.GetBitmap(), clock_dest_rect);

    RenderTime();

    winrt::check_hresult(d2d_context->EndDraw());

    DXGI_PRESENT_PARAMETERS parameters = { 0 };
    parameters.DirtyRectsCount = 0;
    parameters.pDirtyRects = nullptr;
    parameters.pScrollRect = nullptr;
    parameters.pScrollOffset = nullptr;

    auto hr = swap_chain->Present1(1, 0, &parameters);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        // If the device was removed for any reason, a new device and swap chain will need to be created.
        HandleDeviceLost();
    }
    else {
        winrt::check_hresult(hr);
    }
}

void Clock::OnResize(UINT width, UINT height) {
    CreateWindowSizeDependentResources();
}

LRESULT Clock::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        Clock* clock = (Clock*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(clock)
        );

        result = 1;
    }
    else {
        Clock* clock = reinterpret_cast<Clock*>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )));

        bool wasHandled = false;

        if (clock) {
            switch (message) {
            case WM_SIZE:
            {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                clock->OnResize(width, height);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_DISPLAYCHANGE:
            {
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_TIMER:
            {
                clock->show_dots = !clock->show_dots;
                clock->IncreaseTime();
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_PAINT:
            {
                clock->OnRender();
                ValidateRect(hwnd, nullptr);
            }
            result = 0;
            wasHandled = true;
            break;

            case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            result = 1;
            wasHandled = true;
            break;
            }
        }

        if (!wasHandled) {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}

void Clock::RenderTime() {
    if (show_dots) {
        d2d_context->DrawBitmap(digits_bitmap_def.GetBitmap(), dots_dest_rect, 0.75f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, dots_src_rect);
    }

    int digits[] = { hour / 10, hour % 10, minute / 10, minute % 10 };
    auto dest_rect = D2D1::RectF(
        center.x - 241.0f,
        center.y - 96.0f,
        center.x - 133.0f,
        center.y + 96.0f
    );

    for (int i = 0; i < 4; i++) {
        RenderDigit(digits[i], dest_rect);
        dest_rect.left += 108.0f;
        dest_rect.right += 108.0f;
        if (i == 1) {
            dest_rect.left += 50.0f;
            dest_rect.right += 50.0f;
        }
    }
}

void Clock::RenderDigit(int digit, const D2D1_RECT_F& dest_rect) {
    auto src_rect = D2D1::RectF(
        digit * 108.0f,
        0.0f,
        (digit + 1) * 108.0f,
        192.0f
    );

    d2d_context->DrawBitmap(digits_bitmap_def.GetBitmap(), dest_rect, 0.75f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src_rect);
}

void Clock::IncreaseTime() {
    second++;
    minute += second / 60;
    hour += minute / 60;

    second %= 60;
    minute %= 60;
    hour &= 0;
}


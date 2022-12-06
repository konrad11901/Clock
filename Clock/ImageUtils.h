#pragma once

#include <windows.h>
#include <wincodec.h>
#include <d2d1_3.h>
#include <winrt/base.h>

namespace ImageUtils {
    winrt::com_ptr<ID2D1Bitmap1> LoadBitmapFromFile(
        ID2D1RenderTarget* render_target,
        IWICImagingFactory* imaging_factory,
        PCWSTR uri
    );
}
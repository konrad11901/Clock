#pragma once

#include <windows.h>
#include <wincodec.h>
#include <d2d1_3.h>
#include <winrt/base.h>

class BitmapDefinition {
public:
    BitmapDefinition(PCWSTR uri);
    void CreateDeviceIndependentResources(IWICImagingFactory* imaging_factory);
    void CreateDeviceDependentResources(ID2D1DeviceContext6* device_context);
    ID2D1Bitmap1* GetBitmap();
private:
    PCWSTR uri;
    winrt::com_ptr<IWICFormatConverter> converter;
    winrt::com_ptr<ID2D1Bitmap1> bitmap;
};
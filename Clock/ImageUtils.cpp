#include "ImageUtils.h"
#include <winrt/base.h>

winrt::com_ptr<ID2D1Bitmap1> ImageUtils::LoadBitmapFromFile(
    ID2D1RenderTarget* render_target,
    IWICImagingFactory* imaging_factory,
    PCWSTR uri) {
    winrt::com_ptr<IWICBitmapDecoder> decoder;
    winrt::com_ptr<IWICBitmapFrameDecode> source;
    winrt::com_ptr<IWICStream> stream;
    winrt::com_ptr<IWICFormatConverter> converter;
    winrt::com_ptr<IWICBitmapScaler> scaler;

    winrt::check_hresult(imaging_factory->CreateDecoderFromFilename(
        uri,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        decoder.put()
    ));
    
    winrt::check_hresult(decoder->GetFrame(0, source.put()));

    winrt::check_hresult(imaging_factory->CreateFormatConverter(converter.put()));

    winrt::check_hresult(converter->Initialize(
        source.get(),
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.f,
        WICBitmapPaletteTypeMedianCut
    ));

    winrt::com_ptr<ID2D1Bitmap1> bitmap;

    //winrt::check_hresult(render_target->CreateBitmapFromWicBitmap(
    //    converter.get(),
    //    nullptr,
    //    bitmap.put()
    //));

    return bitmap;
}

#include "Image.h"
#include "FlameUI.h"
using namespace FlameUI;
ID2D1Bitmap* Image::GetBitmap()
{
	return rawFrame;
}

void Image::Draw()
{
	auto ctx = BeginDraw(D2D1::ColorF::ColorF(0, 0));
	if (totalFrame == 1)
	{
		if (rawFrame)
			ctx->DrawBitmap(rawFrame, { 0,0,rect.right - rect.left,rect.bottom - rect.top });
	}
	else
	{
		ID2D1Bitmap* pFrameToRender = NULL;
		D2D1_RECT_F drawRect;
		auto hr = CalculateDrawRectangle(drawRect);

		if (SUCCEEDED(hr))
		{
			// Get the bitmap to draw on the hwnd render target
			hr = frameComposeRT->GetBitmap(&pFrameToRender);
		}

		if (SUCCEEDED(hr))
		{
			// Draw the bitmap onto the calculated rectangle

			ctx->DrawBitmap(pFrameToRender, drawRect);
		}

	}
	EndDraw();
}
LRESULT Image::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
Image::Image(View* parent, IWICBitmap* source) :Image(parent)
{
	Content(source);
}
Image::Image(View* parent) : View(parent)
{
	mouseable = false;
	render.container = true;
	layout.sizeMode = { SIZE_MODE_CONTENT,SIZE_MODE_CONTENT };
	render.direct = true;
}
void Image::Content(IWICBitmap* source)
{
	if (rawFrame) rawFrame->Release();
	auto hr = render.context->CreateBitmapFromWicBitmap(source, &rawFrame);
	if (layout.sizeMode.x == SIZE_MODE_CONTENT) size.width = rawFrame->GetSize().width;
	if (layout.sizeMode.y == SIZE_MODE_CONTENT) size.height = rawFrame->GetSize().height;
	UpdateTransform();

}
void Image::Content(const wchar_t* localPath)
{
	if (decoder) decoder->Release();
	auto hr = gWICFactory->CreateDecoderFromFilename(localPath, 0, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	decoder->GetFrameCount(&totalFrame);
	if (totalFrame == 1)
	{
		IWICBitmapFrameDecode* bdf;
		hr = decoder->GetFrame(0, &bdf);
		IWICFormatConverter* cvt;
		gWICFactory->CreateFormatConverter(&cvt);
		hr = cvt->Initialize(bdf, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, 0, 0, WICBitmapPaletteTypeCustom);
		IWICBitmap* bmp;
		hr = gWICFactory->CreateBitmapFromSource(cvt, WICBitmapNoCache, &bmp);
		Content(bmp);
	}
	else
	{
		GetGlobalMetadata();
		SafeRelease(frameComposeRT);
		hr = render.context->CreateCompatibleRenderTarget(
			D2D1::SizeF(
				static_cast<FLOAT>(cxGifImage),
				static_cast<FLOAT>(cyGifImage)),
			&frameComposeRT);
		if (layout.sizeMode.x == SIZE_MODE_CONTENT) size.width = cxGifImage;
		if (layout.sizeMode.y == SIZE_MODE_CONTENT) size.height = cyGifImage;
		timer = Animate(1, 0, 0);
	}
}
void Image::Content(void* ptr, size_t len)
{
	hGMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, len);
	auto p = GlobalLock(hGMem);
	RtlMoveMemory(p, ptr, len);
	GlobalUnlock(p);
	auto hr = CreateStreamOnHGlobal(hGMem, true, &memStream);
	if (decoder) decoder->Release();
	hr = gWICFactory->CreateDecoderFromStream(memStream, 0, WICDecodeMetadataCacheOnDemand, &decoder);
	decoder->GetFrameCount(&totalFrame);
	if (totalFrame == 1)
	{
		IWICBitmapFrameDecode* bdf;
		hr = decoder->GetFrame(0, &bdf);
		IWICFormatConverter* cvt;
		gWICFactory->CreateFormatConverter(&cvt);
		hr = cvt->Initialize(bdf, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, 0, 0, WICBitmapPaletteTypeCustom);
		IWICBitmap* bmp;
		hr = gWICFactory->CreateBitmapFromSource(cvt, WICBitmapNoCache, &bmp);
		Content(bmp);
	}
	else
	{
		GetGlobalMetadata();
		SafeRelease(frameComposeRT);
		hr = render.context->CreateCompatibleRenderTarget(
			D2D1::SizeF(
				static_cast<FLOAT>(cxGifImage),
				static_cast<FLOAT>(cyGifImage)),
			&frameComposeRT);
		if (layout.sizeMode.x == SIZE_MODE_CONTENT) size.width = cxGifImage;
		if (layout.sizeMode.y == SIZE_MODE_CONTENT) size.height = cyGifImage;
		timer = Animate(1, 0, 0);
	}

}

void Image::Animation(float progress, int p1, int p2)
{
	if (progress == 1)
	{
		auto hr = ComposeNextFrame();
		UpdateView();
	}
}

void Image::Size(D2D1_SIZE_F s)
{
	layout.sizeMode = { SIZE_MODE_NONE,SIZE_MODE_NONE };
	View::Size(s);
	UpdateTransform();
}

HRESULT Image::GetBackgroundColor(
	IWICMetadataQueryReader* pMetadataQueryReader)
{
	DWORD dwBGColor;
	BYTE backgroundIndex = 0;
	WICColor rgColors[256];
	UINT cColorsCopied = 0;
	PROPVARIANT propVariant;
	PropVariantInit(&propVariant);
	IWICPalette* pWicPalette = NULL;

	// If we have a global palette, get the palette and background color
	HRESULT hr = pMetadataQueryReader->GetMetadataByName(
		L"/logscrdesc/GlobalColorTableFlag",
		&propVariant);
	if (SUCCEEDED(hr))
	{
		hr = (propVariant.vt != VT_BOOL || !propVariant.boolVal) ? E_FAIL : S_OK;
		PropVariantClear(&propVariant);
	}

	if (SUCCEEDED(hr))
	{
		// Background color index
		hr = pMetadataQueryReader->GetMetadataByName(
			L"/logscrdesc/BackgroundColorIndex",
			&propVariant);
		if (SUCCEEDED(hr))
		{
			hr = (propVariant.vt != VT_UI1) ? E_FAIL : S_OK;
			if (SUCCEEDED(hr))
			{
				backgroundIndex = propVariant.bVal;
			}
			PropVariantClear(&propVariant);
		}
	}

	// Get the color from the palette
	if (SUCCEEDED(hr))
	{
		hr = gWICFactory->CreatePalette(&pWicPalette);
	}

	if (SUCCEEDED(hr))
	{
		// Get the global palette
		hr = decoder->CopyPalette(pWicPalette);
	}

	if (SUCCEEDED(hr))
	{
		hr = pWicPalette->GetColors(
			ARRAYSIZE(rgColors),
			rgColors,
			&cColorsCopied);
	}

	if (SUCCEEDED(hr))
	{
		// Check whether background color is outside range 
		hr = (backgroundIndex >= cColorsCopied) ? E_FAIL : S_OK;
	}

	if (SUCCEEDED(hr))
	{
		// Get the color in ARGB format
		dwBGColor = rgColors[backgroundIndex];

		// The background color is in ARGB format, and we want to 
		// extract the alpha value and convert it in FLOAT
		FLOAT alpha = (dwBGColor >> 24) / 255.f;
		backgroundColor = D2D1::ColorF(dwBGColor, alpha);
	}

	SafeRelease(pWicPalette);
	return hr;
}
HRESULT Image::GetGlobalMetadata()
{
	PROPVARIANT propValue;
	PropVariantInit(&propValue);
	IWICMetadataQueryReader* pMetadataQueryReader = NULL;

	auto hr = decoder->GetMetadataQueryReader(
		&pMetadataQueryReader);


	if (SUCCEEDED(hr))
	{
		// Get background color
		if (FAILED(GetBackgroundColor(pMetadataQueryReader)))
		{
			// Default to transparent if failed to get the color
			backgroundColor = D2D1::ColorF(0, 0.f);
		}
	}

	// Get global frame size
	if (SUCCEEDED(hr))
	{
		// Get width
		hr = pMetadataQueryReader->GetMetadataByName(
			L"/logscrdesc/Width",
			&propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
			{
				cxGifImage = propValue.uiVal;
			}
			PropVariantClear(&propValue);
		}
	}

	if (SUCCEEDED(hr))
	{
		// Get height
		hr = pMetadataQueryReader->GetMetadataByName(
			L"/logscrdesc/Height",
			&propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
			{
				cyGifImage = propValue.uiVal;
			}
			PropVariantClear(&propValue);
		}
	}

	if (SUCCEEDED(hr))
	{
		// Get pixel aspect ratio
		hr = pMetadataQueryReader->GetMetadataByName(
			L"/logscrdesc/PixelAspectRatio",
			&propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI1 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
			{
				UINT uPixelAspRatio = propValue.bVal;

				if (uPixelAspRatio != 0)
				{
					// Need to calculate the ratio. The value in uPixelAspRatio 
					// allows specifying widest pixel 4:1 to the tallest pixel of 
					// 1:4 in increments of 1/64th
					FLOAT pixelAspRatio = (uPixelAspRatio + 15.f) / 64.f;

					// Calculate the image width and height in pixel based on the
					// pixel aspect ratio. Only shrink the image.
					if (pixelAspRatio > 1.f)
					{
						cxGifImagePixel = cxGifImage;
						cyGifImagePixel = static_cast<UINT>(cyGifImage / pixelAspRatio);
					}
					else
					{
						cxGifImagePixel = static_cast<UINT>(cxGifImage * pixelAspRatio);
						cyGifImagePixel = cyGifImage;
					}
				}
				else
				{
					// The value is 0, so its ratio is 1
					cxGifImagePixel = cxGifImage;
					cyGifImagePixel = cyGifImage;
				}
			}
			PropVariantClear(&propValue);
		}
	}

	// Get looping information
	if (SUCCEEDED(hr))
	{
		// First check to see if the application block in the Application Extension
		// contains "NETSCAPE2.0" and "ANIMEXTS1.0", which indicates the gif animation
		// has looping information associated with it.
		// 
		// If we fail to get the looping information, loop the animation infinitely.
		if (SUCCEEDED(pMetadataQueryReader->GetMetadataByName(
			L"/appext/application",
			&propValue)) &&
			propValue.vt == (VT_UI1 | VT_VECTOR) &&
			propValue.caub.cElems == 11 &&  // Length of the application block
			(!memcmp(propValue.caub.pElems, "NETSCAPE2.0", propValue.caub.cElems) ||
				!memcmp(propValue.caub.pElems, "ANIMEXTS1.0", propValue.caub.cElems)))
		{
			PropVariantClear(&propValue);

			hr = pMetadataQueryReader->GetMetadataByName(L"/appext/data", &propValue);
			if (SUCCEEDED(hr))
			{
				//  The data is in the following format:
				//  byte 0: extsize (must be > 1)
				//  byte 1: loopType (1 == animated gif)
				//  byte 2: loop count (least significant byte)
				//  byte 3: loop count (most significant byte)
				//  byte 4: set to zero
				if (propValue.vt == (VT_UI1 | VT_VECTOR) &&
					propValue.caub.cElems >= 4 &&
					propValue.caub.pElems[0] > 0 &&
					propValue.caub.pElems[1] == 1)
				{
					totalLoop = MAKEWORD(propValue.caub.pElems[2],
						propValue.caub.pElems[3]);

					// If the total loop count is not zero, we then have a loop count
					// If it is 0, then we repeat infinitely
					if (totalLoop != 0)
					{
						hasLoop = TRUE;
					}
				}
			}
		}
	}

	PropVariantClear(&propValue);
	SafeRelease(pMetadataQueryReader);
	return hr;
}
HRESULT Image::GetRawFrame(UINT uFrameIndex)
{
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapFrameDecode* pWicFrame = NULL;
	IWICMetadataQueryReader* pFrameMetadataQueryReader = NULL;

	PROPVARIANT propValue;
	PropVariantInit(&propValue);

	// Retrieve the current frame
	HRESULT hr = decoder->GetFrame(uFrameIndex, &pWicFrame);
	if (SUCCEEDED(hr))
	{
		// Format convert to 32bppPBGRA which D2D expects
		hr = gWICFactory->CreateFormatConverter(&pConverter);
	}

	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pWicFrame,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeCustom);
	}

	if (SUCCEEDED(hr))
	{
		// Create a D2DBitmap from IWICBitmapSource
		SafeRelease(rawFrame);
		hr = render.context->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			&rawFrame);
	}

	if (SUCCEEDED(hr))
	{
		// Get Metadata Query Reader from the frame
		hr = pWicFrame->GetMetadataQueryReader(&pFrameMetadataQueryReader);
	}

	// Get the Metadata for the current frame
	if (SUCCEEDED(hr))
	{
		hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Left", &propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
			{
				framePosition.left = static_cast<FLOAT>(propValue.uiVal);
			}
			PropVariantClear(&propValue);
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Top", &propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
			{
				framePosition.top = static_cast<FLOAT>(propValue.uiVal);
			}
			PropVariantClear(&propValue);
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Width", &propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
			{
				framePosition.right = static_cast<FLOAT>(propValue.uiVal)
					+ framePosition.left;
			}
			PropVariantClear(&propValue);
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = pFrameMetadataQueryReader->GetMetadataByName(L"/imgdesc/Height", &propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
			{
				framePosition.bottom = static_cast<FLOAT>(propValue.uiVal)
					+ framePosition.top;
			}
			PropVariantClear(&propValue);
		}
	}

	if (SUCCEEDED(hr))
	{
		// Get delay from the optional Graphic Control Extension
		if (SUCCEEDED(pFrameMetadataQueryReader->GetMetadataByName(
			L"/grctlext/Delay",
			&propValue)))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
			{
				// Convert the delay retrieved in 10 ms units to a delay in 1 ms units
				hr = UIntMult(propValue.uiVal, 10, &frameDelay);
			}
			PropVariantClear(&propValue);
		}
		else
		{
			// Failed to get delay from graphic control extension. Possibly a
			// single frame image (non-animated gif)
			frameDelay = 0;
		}

		if (SUCCEEDED(hr))
		{
			// Insert an artificial delay to ensure rendering for gif with very small
			// or 0 delay.  This delay number is picked to match with most browsers' 
			// gif display speed.
			//
			// This will defeat the purpose of using zero delay intermediate frames in 
			// order to preserve compatibility. If this is removed, the zero delay 
			// intermediate frames will not be visible.
			if (frameDelay < 10)
			{
				frameDelay = 90;
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		if (SUCCEEDED(pFrameMetadataQueryReader->GetMetadataByName(
			L"/grctlext/Disposal",
			&propValue)))
		{
			hr = (propValue.vt == VT_UI1) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				frameDisposal = propValue.bVal;
			}
		}
		else
		{
			// Failed to get the disposal method, use default. Possibly a 
			// non-animated gif.
			frameDisposal = DM_UNDEFINED;
		}
	}

	PropVariantClear(&propValue);

	SafeRelease(pConverter);
	SafeRelease(pWicFrame);
	SafeRelease(pFrameMetadataQueryReader);

	return hr;
}
HRESULT Image::CalculateDrawRectangle(D2D1_RECT_F& drawRect)
{
	HRESULT hr = S_OK;

	// Top and left of the client rectangle are both 0
		// Calculate the area to display the image
		// Center the image if the client rectangle is larger
	drawRect.left = 0;//(static_cast<FLOAT>(rect.right) - cxGifImagePixel) / 2.f;
	drawRect.top = 0;//(static_cast<FLOAT>(rect.bottom) - cyGifImagePixel) / 2.f;
	drawRect.right = drawRect.left + cxGifImagePixel;
	drawRect.bottom = drawRect.top + cyGifImagePixel;

	// If the client area is resized to be smaller than the image size, scale
	// the image, and preserve the aspect ratio
	FLOAT aspectRatio = static_cast<FLOAT>(cxGifImagePixel) /
		static_cast<FLOAT>(cyGifImagePixel);

	if (drawRect.left < 0)
	{
		FLOAT newWidth = static_cast<FLOAT>(rect.right);
		FLOAT newHeight = newWidth / aspectRatio;
		drawRect.left = 0;
		drawRect.top = (static_cast<FLOAT>(rect.bottom) - newHeight) / 2.f;
		drawRect.right = newWidth;
		drawRect.bottom = drawRect.top + newHeight;
	}

	if (drawRect.top < 0)
	{
		FLOAT newHeight = static_cast<FLOAT>(rect.bottom);
		FLOAT newWidth = newHeight * aspectRatio;
		drawRect.left = (static_cast<FLOAT>(rect.right) - newWidth) / 2.f;
		drawRect.top = 0;
		drawRect.right = drawRect.left + newWidth;
		drawRect.bottom = newHeight;
	}


	return hr;
}
HRESULT Image::ComposeNextFrame()
{
	HRESULT hr = S_OK;

	// Compose one frame
	hr = DisposeCurrentFrame();
	if (SUCCEEDED(hr))
	{
		hr = OverlayNextFrame();
	}

	// Keep composing frames until we see a frame with delay greater than
	// 0 (0 delay frames are the invisible intermediate frames), or until
	// we have reached the very last frame.
	while (SUCCEEDED(hr) && frameDelay == 0 && !IsLastFrame())
	{
		hr = DisposeCurrentFrame();
		if (SUCCEEDED(hr))
		{
			hr = OverlayNextFrame();
		}
	}

	// If we have more frames to play, set the timer according to the delay.
	// Set the timer regardless of whether we succeeded in composing a frame
	// to try our best to continue displaying the animation.
	if (!EndOfAnimation() && totalFrame > 1)
	{
		// Set the timer according to the delay
		//SetTimer(m_hWnd, DELAY_TIMER_ID, m_uFrameDelay, NULL);
		Animate(frameDelay, 0, 0, timer - 16);
	}


	return hr;
}
HRESULT Image::DisposeCurrentFrame()
{
	HRESULT hr = S_OK;

	switch (frameDisposal)
	{
	case DM_UNDEFINED:
	case DM_NONE:
		// We simply draw on the previous frames. Do nothing here.
		break;
	case DM_BACKGROUND:
		// Dispose background
		// Clear the area covered by the current raw frame with background color
		hr = ClearCurrentFrameArea();
		break;
	case DM_PREVIOUS:
		// Dispose previous
		// We restore the previous composed frame first
		hr = RestoreSavedFrame();
		break;
	default:
		// Invalid disposal method
		hr = E_FAIL;
	}

	return hr;
}
HRESULT Image::SaveComposedFrame()
{
	HRESULT hr = S_OK;

	ID2D1Bitmap* pFrameToBeSaved = NULL;

	hr = frameComposeRT->GetBitmap(&pFrameToBeSaved);
	if (SUCCEEDED(hr))
	{
		// Create the temporary bitmap if it hasn't been created yet 
		if (savedFrame == NULL)
		{
			D2D1_SIZE_U bitmapSize = pFrameToBeSaved->GetPixelSize();
			D2D1_BITMAP_PROPERTIES bitmapProp;
			pFrameToBeSaved->GetDpi(&bitmapProp.dpiX, &bitmapProp.dpiY);
			bitmapProp.pixelFormat = pFrameToBeSaved->GetPixelFormat();

			hr = frameComposeRT->CreateBitmap(
				bitmapSize,
				bitmapProp,
				&savedFrame);
		}
	}

	if (SUCCEEDED(hr))
	{
		// Copy the whole bitmap
		hr = savedFrame->CopyFromBitmap(NULL, pFrameToBeSaved, NULL);
	}

	SafeRelease(pFrameToBeSaved);

	return hr;
}
HRESULT Image::OverlayNextFrame()
{
	// Get Frame information
	HRESULT hr = GetRawFrame(currentFrame);
	if (SUCCEEDED(hr))
	{
		// For disposal 3 method, we would want to save a copy of the current
		// composed frame
		if (frameDisposal == DM_PREVIOUS)
		{
			hr = SaveComposedFrame();
		}
	}

	if (SUCCEEDED(hr))
	{
		// Start producing the next bitmap
		frameComposeRT->BeginDraw();

		// If starting a new animation loop
		if (currentFrame == 0)
		{
			// Draw background and increase loop count
			frameComposeRT->Clear(backgroundColor);
			loopNumber++;
		}

		// Produce the next frame
		frameComposeRT->DrawBitmap(
			rawFrame,
			framePosition);

		hr = frameComposeRT->EndDraw();
	}

	// To improve performance and avoid decoding/composing this frame in the 
	// following animation loops, the composed frame can be cached here in system 
	// or video memory.

	if (SUCCEEDED(hr))
	{
		// Increase the frame index by 1
		currentFrame = (++currentFrame) % totalFrame;
	}

	return hr;
}
HRESULT Image::RestoreSavedFrame()
{
	HRESULT hr = S_OK;

	ID2D1Bitmap* pFrameToCopyTo = NULL;

	hr = savedFrame ? S_OK : E_FAIL;

	if (SUCCEEDED(hr))
	{
		hr = frameComposeRT->GetBitmap(&pFrameToCopyTo);
	}

	if (SUCCEEDED(hr))
	{
		// Copy the whole bitmap
		hr = pFrameToCopyTo->CopyFromBitmap(NULL, savedFrame, NULL);
	}

	SafeRelease(pFrameToCopyTo);

	return hr;
}
HRESULT Image::ClearCurrentFrameArea()
{
	frameComposeRT->BeginDraw();

	// Clip the render target to the size of the raw frame
	frameComposeRT->PushAxisAlignedClip(
		&framePosition,
		D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	frameComposeRT->Clear(backgroundColor);

	// Remove the clipping
	frameComposeRT->PopAxisAlignedClip();

	return frameComposeRT->EndDraw();
}


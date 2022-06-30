#pragma once
#include "View.h"
#include <iostream>
namespace FlameUI
{
	class Image :
		public View
	{
		enum DISPOSAL_METHODS
		{
			DM_UNDEFINED = 0,
			DM_NONE = 1,
			DM_BACKGROUND = 2,
			DM_PREVIOUS = 3
		};
	private:
		ID2D1Bitmap* rawFrame = nullptr;
		ID2D1Bitmap* savedFrame = nullptr;
		UINT totalLoop;
		UINT currentFrame;
		UINT totalFrame;
		BOOL hasLoop;
		D2D_RECT_F framePosition;
		IWICBitmapDecoder* decoder;
		animation_id timer;
		Color backgroundColor;
		UINT    cxGifImage;
		UINT    cyGifImage;
		UINT    cxGifImagePixel;  // Width of the displayed image in pixel calculated using pixel aspect ratio
		UINT    cyGifImagePixel;  // Height of the displayed image in pixel calculated using pixel aspect ratio
		UINT    frameDisposal;
		UINT    frameDelay;
		UINT    loopNumber;      // The current animation loop number (e.g. 1 when the animation is first played)
		ID2D1BitmapRenderTarget* frameComposeRT;
		HRESULT GetBackgroundColor(IWICMetadataQueryReader* pMetadataQueryReader);
		HRESULT GetRawFrame(UINT uFrameIndex);
		HRESULT GetGlobalMetadata();
		HRESULT ComposeNextFrame();
		HRESULT DisposeCurrentFrame();
		HRESULT OverlayNextFrame();

		HRESULT SaveComposedFrame();
		HRESULT RestoreSavedFrame();
		HRESULT ClearCurrentFrameArea();

		HGLOBAL hGMem;
		LPSTREAM memStream;

		BOOL IsLastFrame()
		{
			return (currentFrame == 0);
		}

		BOOL EndOfAnimation()
		{
			return hasLoop && IsLastFrame() && loopNumber == totalLoop + 1;
		}

		HRESULT CalculateDrawRectangle(D2D1_RECT_F& drawRect);

	protected:
		void Draw() override;
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Animation(float progress, int p1, int p2) override;
	public:
		Image(View* parent);
		Image(View* parent, IWICBitmap* source);
		void Content(IWICBitmap* src);
		void Content(void* ptr, size_t len);
		void Content(const wchar_t* localPath);
		void Size(D2D1_SIZE_F);
		ID2D1Bitmap* GetBitmap();
	};
}
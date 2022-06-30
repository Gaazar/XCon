#pragma once
#include "View.h"
namespace FlameUI
{
	class D3DViewPort :
		public View
	{
	private:
#ifdef _DX12
		ID3D12Resource* d12Tex;
#else

#endif
		ID3D11Resource* d11Tex;
		IDXGISurface* dxgiSurface;
		HANDLE shareHandle;
	protected:
		ID2D1Bitmap* d2dTex;
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;
	public:
		bool dontPaint = true;

		D3DViewPort(View* parent);
#ifdef _DX12
		void SetOutput(ID3D12Resource* res);
#endif
		void SetOutput(ID3D11Resource* res, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
		void* GetD3DResource();
		void* GetD3DDevice();
		void* GetDXGIDevice();
	};
}

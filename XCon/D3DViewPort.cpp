#include "D3DViewPort.h"
#include "FlameUI.h"

using namespace FlameUI;
using namespace D2D1;
D3DViewPort::D3DViewPort(View* parent) :View(parent), d2dTex(nullptr)
{

}
LRESULT D3DViewPort::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
#ifdef _DX12
void D3DViewPort::SetOutput(ID3D12Resource* res)
{
	auto hr = gD3D12Device->CreateSharedHandle(res, nullptr, GENERIC_ALL, nullptr, &shareHandle);
	hr = gD3D11Device->OpenSharedResource1(shareHandle, IID_PPV_ARGS(&d11Tex));
	SetOutput(d11Tex);
}
#endif
void D3DViewPort::SetOutput(ID3D11Resource* res, DXGI_FORMAT format)
{
	if (d2dTex) d2dTex->Release();
	auto hr = res->QueryInterface(&dxgiSurface);
	hr = render.context->CreateSharedBitmap(__uuidof(IDXGISurface), dxgiSurface, &BitmapProperties(
		D2D1::PixelFormat(format, D2D1_ALPHA_MODE_PREMULTIPLIED)), &d2dTex);
}


void D3DViewPort::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(ColorF::Black, 0));
	if (!dontPaint && d2dTex)
	{
		ctx->DrawBitmap(d2dTex, { 0,0,rect.width(),rect.height() });
	}
	EndDraw();

}

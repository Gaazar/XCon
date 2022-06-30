#include "VideoPlayer.h"
#include "FlameUI.h"
#include <string>

#include "yuv2rgbp.h"
#include "yuv2rgbv.h"

#include <iostream>
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
using namespace FlameUI;
using namespace D2D1;

VideoPlayer::VideoPlayer(View* parent) :View(parent)
{
	timer = Animate(14, 0, 0);

}

void VideoPlayer::Animation(float progress, int p1, int p2)
{
	if (progress == 1)
	{
		RenderFrame();
		DisplayFrame();
		UpdateView();
		Animate(2, 0, 0, timer);

	}
}
void VideoPlayer::RenderFrame()
{
	auto frame = NextFrame();

	ID3D11Texture2D* t_frame = (ID3D11Texture2D*)frame->data[0];
	int t_index = (int)frame->data[1];

	ID3D11Device* device;
	t_frame->GetDevice(&device);

	ID3D11DeviceContext* deviceCtx;
	device->GetImmediateContext(&deviceCtx);

	ID3D11Texture2D* videoTexture;
	auto hr = device->OpenSharedResource(sharedHandle, IID_PPV_ARGS(&videoTexture));

	deviceCtx->CopySubresourceRegion(videoTexture, 0, 0, 0, 0, t_frame, t_index, 0);
	deviceCtx->Flush();

	deviceCtx->Release();
	videoTexture->Release();
	av_frame_free(&frame);

}
void VideoPlayer::DisplayFrame() 
{
	IDXGISurface* dxgiSurface;
	auto hr = tex2d->QueryInterface(&dxgiSurface);
	if (img2d) img2d->Release();
	hr = render.context->CreateImageSourceFromDxgi(&dxgiSurface, 1, DXGI_COLOR_SPACE_YCBCR_FULL_G22_NONE_P709_X601, D2D1_IMAGE_SOURCE_FROM_DXGI_OPTIONS_NONE, &img2d);

	dxgiSurface->Release();

}
void VideoPlayer::PlayPause()
{

}
void VideoPlayer::Source(std::wstring url)
{
	avformat_open_input(&fmtCtx, ws2s(url).c_str(), nullptr, nullptr);
	avformat_find_stream_info(fmtCtx, nullptr);

	for (int i = 0; i < fmtCtx->nb_streams; i++) {
		AVStream* stream = fmtCtx->streams[i];
		if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
			videoStreamIndex = i;
			vcodecCtx = avcodec_alloc_context3(codec);
			avcodec_parameters_to_context(vcodecCtx, fmtCtx->streams[i]->codecpar);
			avcodec_open2(vcodecCtx, codec, NULL);
		}
	}

	AVBufferRef* hw_device_ctx = nullptr;
	av_hwdevice_ctx_create(&hw_device_ctx, AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA, NULL, NULL, NULL);
	vcodecCtx->hw_device_ctx = hw_device_ctx;
	width = vcodecCtx->width;
	height = vcodecCtx->height;

	D3D11_TEXTURE2D_DESC tdesc = {};
	tdesc.Format = DXGI_FORMAT_NV12;
	tdesc.Usage = D3D11_USAGE_DEFAULT;
	tdesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
	tdesc.ArraySize = 1;
	tdesc.MipLevels = 1;
	tdesc.SampleDesc = { 1, 0 };
	tdesc.Height = height;
	tdesc.Width = width;
	tdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (tex2d) tex2d->Release();
	gD3D11Device->CreateTexture2D(&tdesc, nullptr, &tex2d);

	IDXGIResource* dxgiShareTexture;
	tex2d->QueryInterface(IID_PPV_ARGS(&dxgiShareTexture));
	dxgiShareTexture->GetSharedHandle(&sharedHandle);

	/*tdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	tdesc.Usage = D3D11_USAGE_DEFAULT;
	tdesc.MiscFlags = 0;
	tdesc.ArraySize = 1;
	tdesc.MipLevels = 1;
	tdesc.SampleDesc = { 1, 0 };
	tdesc.Height = height;
	tdesc.Width = width;
	tdesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	if (tex2d) tex2d->Release();
	gD3D11Device->CreateTexture2D(&tdesc, nullptr, &texRt);*/

}

AVFrame* VideoPlayer::NextFrame()
{
	while (1) {
		AVPacket* packet = av_packet_alloc();
		int ret = av_read_frame(fmtCtx, packet);
		if (ret == 0 && packet->stream_index == videoStreamIndex) {
			ret = avcodec_send_packet(vcodecCtx, packet);
			if (ret == 0) {
				AVFrame* frame = av_frame_alloc();
				ret = avcodec_receive_frame(vcodecCtx, frame);
				if (ret == 0) {
					av_packet_unref(packet);
					return frame;
				}
				else if (ret == AVERROR(EAGAIN)) {
					av_frame_unref(frame);
				}
			}
		}

		av_packet_unref(packet);
	}

	return nullptr;
}

LRESULT VideoPlayer::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

void VideoPlayer::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(ColorF::Black, 0));
	if (img2d)
	{
		D2D1_MATRIX_3X2_F mat;
		ctx->GetTransform(&mat);
		ctx->SetTransform(Matrix3x2F::Scale({ rect.width() / width,rect.height() / height }) * mat);
		ctx->DrawImage(img2d);
		ctx->SetTransform(mat);

	}
	EndDraw();

}
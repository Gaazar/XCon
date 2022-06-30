#pragma once
#include "View.h"
#include "D3DViewPort.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
}

namespace FlameUI
{
	class VideoPlayer :
		public D3DViewPort
	{
	private:
		AVFormatContext* fmtCtx = nullptr;
		AVCodecContext* vcodecCtx = nullptr;
		int videoStreamIndex = 0;
		animation_id timer;
		int width;
		int height;
		ID3D11Texture2D* tex2d;
		HANDLE sharedHandle;

		AVFrame* NextFrame();
		void Animation(float progress, int p1, int p2) override;
		void RenderFrame();

	public:
		VideoPlayer(View* parent);
		void PlayPause();
		void Source(std::wstring);

	};
}

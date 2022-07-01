#pragma once
#include "View.h"
#include "D3DViewPort.h"
#include <thread>
#include <string>
#include <mutex>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
}

namespace FlameUI
{
	class VideoPlayer :
		public View
	{
	protected:
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Draw() override;

	private:
		AVFormatContext* fmtCtx = nullptr;
		AVCodecContext* vcodecCtx = nullptr;
		int videoStreamIndex = 0;
		animation_id timer;
		int width;
		int height;
		ID3D11Texture2D* tex2d;
		ID2D1ImageSource* img2d = nullptr;
		//ID3D11Texture2D* texRt;
		HANDLE sharedHandle;
		std::thread playThread;
		std::wstring url;
		std::mutex mtx;


		AVFrame* NextFrame();
		void Animation(float progress, int p1, int p2) override;
		void RenderFrame();
		void DisplayFrame();
		void PlayThread();

	public:
		int fillMode = 0;//0 autofit, 1 strech, 2 flat
		VideoPlayer(View* parent);
		void PlayPause();
		void Source(std::wstring);

	};
}

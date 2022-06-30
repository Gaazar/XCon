#pragma once
#include "View.h"
#include <string>

namespace FlameUI
{
	class Label :
		public View
	{
	private:
		wstring content;
		IDWriteTextFormat* format;
	protected:
		void Draw() override;
		LRESULT OnEvent(Message msg, WPARAM wParam, LPARAM lParam) override;
		void Refresh();
	public:
		Color frontColor;
		Label(View* parent);
		Label(View* parent, std::wstring content);
		void Font(IDWriteTextFormat* format);
		void Content(std::wstring content);
		std::wstring Content();

	};
}

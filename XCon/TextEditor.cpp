#include "TextEditor.h"

using namespace FlameUI;
using namespace D2D1;
TextEditor::TextEditor(View* parent) :View(parent)
{
	size = { 64,32 };
	scroller = new ScrollView(this);
	scroller->Position({ 1 + 2 * Theme::BorderWidth,1 + 2.f * Theme::BorderWidth });
	scroller->Size({ 1 + 2 * Theme::BorderWidth,1 + 2.f * Theme::BorderWidth });
	scroller->Coord(COORD_FILL, COORD_FILL);

	base = new TextEditBase(scroller->Content());
	base->Position({ 0,0 });
	base->Size({ 0,0 });
	base->Coord(COORD_FILL, COORD_FILL);
	//base->SizeMode(SIZE_MODE_CONTENT, SIZE_MODE_CONTENT);

	keyable = true;

}
TextEditor::~TextEditor()
{
	delete scroller;
}
LRESULT TextEditor::OnEvent(Message msg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}
void TextEditor::Draw()
{
	auto ctx = BeginDraw(ColorF::ColorF(0, 0));
	ID2D1SolidColorBrush* br;
	ctx->CreateSolidColorBrush(Theme::Color::Theme, &br);
	if (!focused)
		br->SetColor(AdaptColor(Theme::Color::Front));
	ctx->DrawRectangle({ 1.5f,1.5f,rect.width() - 1.5f,rect.height() - 1.5f }, br, Theme::BorderWidth);
	br->Release();
	EndDraw();
}

void TextEditor::Multiline(bool v)
{
	base->Multiline(v);
}
void TextEditor::Readonly(bool v)
{
	base->Readonly(v);
}
void TextEditor::Password(bool v)
{
	base->Password(v);
}
void TextEditor::Tabable(bool v)
{
	base->Tabable(v);
}

void TextEditor::Content(wstring c)
{
	base->Content(c);
}
wstring TextEditor::Content()
{
	return base->Content();
}

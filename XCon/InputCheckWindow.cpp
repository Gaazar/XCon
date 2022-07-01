#include "Frames.h"

#include "FlameUI.h"
#include "XInputCehcker.h"
#include "RadioButton.h"
#include "Label.h"
#include "global.h"

#include <iostream>

using namespace FlameUI;

Frame* frame = nullptr;
XInputCehcker* xic = nullptr;

void ShowInputCheckWindow()
{
	if (frame)
	{
		FlashWindow(frame->GetNative(), FALSE);
		return;
	}
	frame = new Frame({ 380,430 });
	frame->Title(L"ÊäÈëÉè±¸");
	frame->maximal = false;
	frame->sizable = false;
	RadioButton* rb = new RadioButton(frame);
	rb->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_NONE);
	rb->Position({ 10,40 });
	new Label(rb, L"1");
	rb->Checked(true);
	rb->AddEventListener([](Message, WPARAM, LPARAM) {xic->index = 0; }, FE_LBUTTONDOWN);

	rb = new RadioButton(frame);
	rb->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_NONE);
	rb->Position({ 10 + 90,40 });
	rb->AddEventListener([](Message, WPARAM, LPARAM) {xic->index = 1; }, FE_LBUTTONDOWN);
	new Label(rb, L"2");

	rb = new RadioButton(frame);
	rb->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_NONE);
	rb->Position({ 10 + 180,40 });
	new Label(rb, L"3");
	rb->AddEventListener([](Message, WPARAM, LPARAM) {xic->index = 2; }, FE_LBUTTONDOWN);

	rb = new RadioButton(frame);
	rb->SizeMode(SIZE_MODE_CHILDREN, SIZE_MODE_NONE);
	rb->Position({ 10 + 270,40 });
	new Label(rb, L"4");
	rb->AddEventListener([](Message, WPARAM, LPARAM) {xic->index = 3; }, FE_LBUTTONDOWN);

	xic = new XInputCehcker(frame);
	xic->Position({ 10,430 - 360 });

	frame->AddEventListener([](Message, WPARAM, LPARAM) {
		frame = nullptr;
		}, FE_DESTROY);

	frame->Show();

}
#pragma once
#include <string>
#include "basictypes.h"
#include "FlameUI.h"
#include <map>;
using namespace D2D1;

namespace FlameUI
{
	namespace ImGui
	{
		struct Content
		{
			float left, top, width, height;
		};
		typedef unsigned int ID;
		enum DRAW_COMMAND
		{
			DRAW_COMMAND_RECT,
			DRAW_COMMAND_ROUNDED_RECT,
			DRAW_COMMAND_TEXT,
			DRAW_COMMAND_ELLIPSE,
			DRAW_COMMAND_LINE,
			DRAW_COMMAND_IMAGE,
			DRAW_COMMAND_PUSHCLIP,
			DRAW_COMMAND_POPCLIP
		};
		struct DrawCommand
		{
			DRAW_COMMAND command;
			std::wstring _text;
			union
			{
				struct
				{
					Color color;
					float stroke;
					Vector2 start;
					Vector2 end;
				} line;
				struct
				{
					Color color;
					float stroke;
					Rect rect;
				} rect;
				struct
				{
					Color color;
					float stroke;
					Rect rect;
					Vector2 radius;
				} roundedRect;
				struct
				{
					Color color;
					float stroke;
					Vector2 center;
					Vector2 radius;
				} ellipse;
				struct _text
				{
					Color color;
					Rect rect;
					float scale;
				} text;
				struct
				{
					Rect rect;
					ID2D1Bitmap* bitmap;
				} image;
				struct
				{
					Rect clip;
				} clip;
			};
			DrawCommand(DRAW_COMMAND cmd, Vector2 s_c, Vector2 e_r, Color c, float stroke)
			{
				command = cmd;
				line.color = c;
				line.stroke = stroke;
				line.start = s_c;
				line.end = e_r;
			}
			DrawCommand(DRAW_COMMAND cmd, Rect rc, Color c, float stroke)
			{
				command = cmd;
				rect.color = c;
				rect.stroke = stroke;
				rect.rect = rc;

			}
			DrawCommand(DRAW_COMMAND cmd, Rect rc, Vector2 radius, Color c, float stroke)
			{
				command = cmd;
				roundedRect.color = c;
				roundedRect.stroke = stroke;
				roundedRect.rect = rc;
				roundedRect.radius = radius;
			}
			DrawCommand(std::wstring t, Rect rc, Color c, float scale)
			{
				command = DRAW_COMMAND_TEXT;

				_text = t;
				text.rect = rc;
				text.scale = scale;
				text.color = c;
			}
			DrawCommand(ID2D1Bitmap* bitmap, Rect rc)//DrawImage
			{
				command = DRAW_COMMAND_IMAGE;
				image.bitmap = bitmap;
				image.rect = rc;
			}
			DrawCommand(Rect rc) // PushClip
			{
				command = DRAW_COMMAND_PUSHCLIP;
				clip.clip = rc;
			}
			DrawCommand(DRAW_COMMAND cmd = DRAW_COMMAND_POPCLIP) // PushClip
			{
				command = cmd;
			}


		};
		class DefferedContext
		{
			vector<DrawCommand> commands;
		public:
			void ClearCommands() { commands.clear(); }
			vector<DrawCommand>& GetCommands() { return commands; }
			//to fill geometry ,set stroke = 0
			void DrawRect(Rect rc, Color c, float stroke = 0.f) { commands.push_back(DrawCommand(DRAW_COMMAND_RECT, rc, c, stroke)); }
			void DrawRoundedRect(Rect rc, Vector2 radius, Color c, float stroke = 0.f) { commands.push_back(DrawCommand(DRAW_COMMAND_ROUNDED_RECT, rc, radius, c, stroke)); }
			void DrawLine(Vector2 start, Vector2 end, Color c, float stroke = 1.f) { commands.push_back(DrawCommand(DRAW_COMMAND_LINE, start, end, c, stroke)); }
			void DrawEllipse(Vector2 center, Vector2 radius, Color c, float stroke = 0.f) { commands.push_back(DrawCommand(DRAW_COMMAND_ELLIPSE, center, radius, c, stroke)); }
			void DrawImage(ID2D1Bitmap* bitmap, Rect rc) { commands.push_back(DrawCommand(bitmap, rc)); }
			void DrawText(std::wstring text, Rect rc, Color c, float scale = 1.f) { commands.push_back(DrawCommand(text, rc, c, scale)); }
			void PushClipRect(Rect rc) { commands.push_back(DrawCommand(rc)); }
			void PopClipRect() { commands.push_back(DrawCommand()); }
			void Excute(ID2D1DeviceContext1* ctx, IDWriteTextFormat* tf)
			{
				ID2D1SolidColorBrush* color;
				ctx->CreateSolidColorBrush(ColorF::ColorF(ColorF::Black, 0), &color);
				for (auto i : commands)
				{
					switch (i.command)
					{
					case DRAW_COMMAND_LINE:
					{
						color->SetColor(i.line.color);
						ctx->DrawLine(i.line.start, i.line.end, color, i.line.stroke);
						break;
					}
					case DRAW_COMMAND_RECT:
					{
						color->SetColor(i.rect.color);
						if (i.rect.stroke == 0)
						{
							ctx->FillRectangle(i.rect.rect, color);
						}
						else
						{
							ctx->DrawRectangle(i.rect.rect, color, i.rect.stroke);
						}
						break;
					}
					case DRAW_COMMAND_ROUNDED_RECT:
					{
						color->SetColor(i.roundedRect.color);
						if (i.rect.stroke == 0)
						{
							ctx->FillRoundedRectangle(RoundedRect(i.roundedRect.rect, i.roundedRect.radius.x, i.roundedRect.radius.y), color);
						}
						else
						{
							ctx->DrawRoundedRectangle(RoundedRect(i.roundedRect.rect, i.roundedRect.radius.x, i.roundedRect.radius.y), color, i.roundedRect.stroke);
						}
						break; break;
					}
					case DRAW_COMMAND_ELLIPSE:
					{
						color->SetColor(i.ellipse.color);
						if (i.rect.stroke == 0)
						{
							ctx->FillEllipse(Ellipse(i.ellipse.center, i.ellipse.radius.x, i.ellipse.radius.y), color);
						}
						else
						{
							ctx->DrawEllipse(Ellipse(i.ellipse.center, i.ellipse.radius.x, i.ellipse.radius.y), color, i.ellipse.stroke);
						}break;
					}
					case DRAW_COMMAND_IMAGE:
					{
						break;
					}
					case DRAW_COMMAND_TEXT:
					{
						color->SetColor(i.text.color);
						ctx->SetTransform(Matrix3x2F::Scale({ i.text.scale,i.text.scale }, { i.text.rect.left,i.text.rect.top }));
						ctx->DrawTextW(i._text.c_str(), i._text.length(), tf,
							{
								i.text.rect.left,
								i.text.rect.top,
								i.text.rect.left + i.text.rect.width() / i.text.scale,
								i.text.rect.top + i.text.rect.height() / i.text.scale
							},
							color, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
						ctx->SetTransform(IdentityMatrix());
						break;
					}
					case DRAW_COMMAND_PUSHCLIP:
					{
						ctx->PushAxisAlignedClip(i.clip.clip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
						break;
					}
					case DRAW_COMMAND_POPCLIP:
					{
						ctx->PopAxisAlignedClip();
						break;
					}
					default:
						break;
					}
				}

			}

		};
		struct State
		{
			bool isFocus;
			bool isHover;
			bool isPressed;
			bool isEditing;

			union
			{
				void* p;
				int i;
				float f;
				bool b;
			};
		};
		struct Context
		{
			DefferedContext renderContext;
			Vector2 cursorPos;
			Vector2 dragDelta;
			Vector2 drawPos;
			Rect currentClipRect;
			bool lButtonDown;
			bool rButtonDown;
			bool mButtonDown;
			ID focus;

			float prevX;
			float prevY;

			Message event;
			ID seed;
			std::map<ID, State> states;
		};
		extern void* parentContent;
		extern void* colorTheme;
		extern void* colorBack;
		extern void* colorFront;
		extern Context* currentCotext;
		extern IDWriteTextFormat* defaultFont;

		ID HashString(String s);
		ID HashStringSL(String s);

		void SameLine(float x = 0, float spacing = 0);
		void Seperator();

		bool CollapsingHeader(String label); //▶

		void Text(String text, Color c = FlameUI::Theme::Color::Front, float scale = 1.f);
		void Text(String text, Size rc, Color c = FlameUI::Theme::Color::Front, float scale = 1.f);
		bool Button(String lable, Color mask = { 0,0,0,0 });
		bool Button(String lable, Size rc, Color mask = { 0,0,0,0 });
		bool CheckBox(String label, bool& checked);
		bool RadioButton(String label, int value, int& selection);
		bool Toggle(String label, bool& enabled);
		float FloatInput(float v);
		Vector2 Float2Input(Vector2* v);
		Vector3 Float3Input(Vector3* v);
		Vector4 Float4Input(Vector4* v);
		Color ColorSelector(Color* c);
		float FloatSlider(float* v, float min, float max);
		int IntSlider(int* value, int min, int max);

		int Combo(String label, String* items, int count, int* current);

		String TextInput(String text);

	};
}
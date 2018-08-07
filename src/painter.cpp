#include "painter.h"
#include "config.h"

#include "jcommon/jsystem.h"
#include "jgui/jfont.h"

#include <stdarg.h>

#define GAPX	16
#define GAPY	16

std::vector<jgui::Font *> Painter::_fonts;

Painter::Painter()
{
}

Painter::~Painter()
{
	// TODO:: delete fonts;
}

void Painter::Initialize()
{
	if (_fonts.size() > 0) {
		return;
	}

	std::string name = __C->GetTextParam("system.font");

	if (name.empty() == true) {
		name = "default";
	}

	jgui::jsize_t screen; // TODO:: = jgui::GFXHandler::GetInstance()->GetScreenSize();

	int d = 960;

	_fonts.push_back(new jgui::Font(name, jgui::JFA_NORMAL, (32*screen.height)/d));
	_fonts.push_back(new jgui::Font(name, jgui::JFA_NORMAL, (64*screen.height)/d));
	_fonts.push_back(new jgui::Font(name, jgui::JFA_NORMAL, (96*screen.height)/d));
	_fonts.push_back(new jgui::Font(name, jgui::JFA_NORMAL, (120*screen.height)/d));
	_fonts.push_back(new jgui::Font(name, jgui::JFA_BOLD, (18*screen.height)/d));
}

jgui::Font * Painter::GetFont(int font_index)
{
	return _fonts[font_index];
}

void Painter::DrawBorder(jgui::Graphics *g, jgui::Color color, int x, int y, int w, int h, int border_size)
{
	if (w < 0 || h < 0) {
		return;
	}

	g->SetColor(color);

	for (int i=0; i<border_size; i++) {
		g->DrawRectangle(x+i, y+i, w-2*i, h-2*i);
	}
}

void Painter::DrawBox(jgui::Graphics *g, jgui::Color color, int x, int y, int w, int h)
{
	if (w < 0 || h < 0) {
		return;
	}

	g->SetColor(color);
	g->FillRectangle(x, y, w, h);
}

void Painter::DrawString(jgui::Graphics *g, int font_index, int shadow_size, jgui::Color color, int x, int y, std::string fmt)
{
	DrawString(g, font_index, shadow_size, color, x, y, fmt.c_str());
}

void Painter::DrawString(jgui::Graphics *g, int font_index, int shadow_size, jgui::Color color, int x, int y, int w, int h, std::string fmt)
{
	DrawString(g, font_index, shadow_size, color, x, y, w, h, jgui::JHA_CENTER, jgui::JVA_CENTER, fmt.c_str());
}

void Painter::DrawString(jgui::Graphics *g, int font_index, int shadow_size, jgui::Color color, int x, int y, const char *fmt, ...)
{
	int ss = shadow_size/2;

	int tmp_size = 4096;
	char tmp[tmp_size];
	va_list va;

	va_start(va, fmt);
	vsnprintf(tmp, tmp_size-1, fmt, va); tmp[tmp_size] = 0;
	va_end(va);

	jgui::Font *font = _fonts[font_index];

	if (font == NULL) {
		return;
	}

	g->SetFont(font);

	if (ss > 0) {
		g->SetColor(0xff000000);
		g->DrawString(tmp, x+ss, y+ss);
	}

	g->SetColor(color);
	g->DrawString(tmp, x, y);
}

void Painter::DrawString(jgui::Graphics *g, int font_index, int shadow_size, jgui::Color color, int x, int y, int w, int h, jgui::jhorizontal_align_t halign, jgui::jvertical_align_t valign, const char *fmt, ...)
{
	int ss = shadow_size/2;
	int tmp_size = 4096;
	char tmp[tmp_size];
	va_list va;

	va_start(va, fmt);
	vsnprintf(tmp, tmp_size-1, fmt, va); tmp[tmp_size] = 0;
	va_end(va);

	jgui::Font *font = _fonts[font_index];

	if (font == NULL) {
		return;
	}

	g->SetFont(font);

	if (ss > 0) {
		g->SetColor(0xff000000);
		g->DrawString(font->TruncateString(tmp, "...", w), x+ss, y+ss, w, h, halign, valign);
	}

	g->SetColor(color);
	g->DrawString(font->TruncateString(tmp, "...", w), x, y, w, h, halign, valign);
}


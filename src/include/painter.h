#ifndef __PAINTER_PHOTOBOOTH_H
#define __PAINTER_PHOTOBOOTH_H

#include "jgraphics.h"

class Painter {

	private:
		static std::vector<jgui::Font *> _fonts;
	
	private:
		Painter();

	public:
		virtual ~Painter();

		static jgui::Font * GetFont(int font_index);

		static void Initialize();

		static void DrawBorder(jgui::Graphics *g, jgui::Color color, int x, int y, int w, int h, int border_size = 1);
		static void DrawBox(jgui::Graphics *g, jgui::Color color, int x, int y, int w, int h);
		
		static void DrawString(jgui::Graphics *g, int font_index, int shadow_size, jgui::Color color, int x, int y, std::string fmt);
		static void DrawString(jgui::Graphics *g, int font_index, int shadow_size, jgui::Color color, int x, int y, int w, int h, std::string fmt);

		static void DrawString(jgui::Graphics *g, int font_index, int shadow_size, jgui::Color color, int x, int y, const char *fmt, ...);
		static void DrawString(jgui::Graphics *g, int font_index, int shadow_size, jgui::Color color, int x, int y, int w, int h, jgui::jhorizontal_align_t halign, jgui::jvertical_align_t valign, const char *fmt, ...);

};

#endif

/***************************************************************************
 *   Copyright (C) 2005 by Jeff Ferr                                       *
 *   root@sat                                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "levelframe.h"
#include "painter.h"
#include "config.h"

LevelFrame::LevelFrame(jgui::Container *parent):
	jgui::Component()
{
	jgui::jregion_t t = parent->GetVisibleBounds();

	_level = 0;
	_timeout = 0;
  _is_running = false;

	SetBounds((t.width-t.width/2)/2, t.height-t.height/3, t.width/2, t.height/4);
	SetVisible(false);
}

LevelFrame::~LevelFrame()
{
}

void LevelFrame::Show(std::string id, int level)
{
	_timeout = __C->GetIntegerParam("frame.timeout");
	_id = id;
	_level = level;

	if (_is_running == false) {
		_thread = std::thread(&LevelFrame::Run, this);
	}
	
	SetVisible(true);
}
	
void LevelFrame::Hide()
{
	if (_is_running == false) {
		return;
	}
	
	_timeout = 0;

  _thread.join();

	SetVisible(false);
}
	
void LevelFrame::Paint(jgui::Graphics *g)
{
	// jgui::Component::Paint(g);

	jgui::jregion_t bounds = GetVisibleBounds();

	int gap = 8;
	int sw = bounds.width-2*gap,
			sh = bounds.height;
	int level = (_level < 0)?0:(_level > 100)?100:_level;

	jgui::Font *font = Painter::GetFont(1);

	Painter::DrawString(g, 1, 0, 0xfff0f0f0, 0, 16, sw, font->GetSize(), _id);
	Painter::DrawBox(g, 0xffcfcdc8, gap, 16+font->GetSize()+32, (sw*level)/100, 48);
	Painter::DrawBorder(g, 0xff000000, gap, 16+font->GetSize()+32, sw, 48);
}

void LevelFrame::Run()
{
	while (_timeout > 0) {
		sleep(1);

		_timeout = _timeout - 1;
	}

	SetVisible(false);
}

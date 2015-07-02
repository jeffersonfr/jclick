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

LevelFrame *LevelFrame::_instance = NULL;

LevelFrame::LevelFrame():
	jgui::Frame()
{
	_level = 0;
	_timeout = 0;

	SetBounds((_size.width-_size.width/2)/2, (_size.height-_size.height/3), _size.width/2, _size.height/4);
	SetUndecorated(true);
	SetInputEnabled(false);
}

LevelFrame::~LevelFrame()
{
}

LevelFrame * LevelFrame::GetInstance()
{
	if (_instance == NULL) {
		_instance = new LevelFrame();
	}

	return _instance;
}

void LevelFrame::Show(std::string id, int level)
{
	_id = id;
	_level = level;
	_timeout = __C->GetIntegerParam("frame.timeout");

	if (IsVisible() == false) {
		jgui::Frame::Show(false);

		Start();
	} else {
		Repaint();
	}
}
	
void LevelFrame::Paint(jgui::Graphics *g)
{
	jgui::Frame::Paint(g);

	jgui::jregion_t bounds = GetVisibleBounds();
	jgui::jinsets_t insets = GetInsets();

	int sw = bounds.width-insets.left-insets.right,
			sh = bounds.height-insets.top-insets.bottom;
	int level = (_level < 0)?0:(_level > 100)?100:_level;

	jgui::Font *font = Painter::GetFont(1);

	Painter::DrawString(g, 1, 0, 0xfff0f0f0, insets.left, 16, sw, font->GetSize(), _id);
	Painter::DrawBox(g, 0xffcfcdc8, insets.left, 16+font->GetSize()+32, (sw*level)/100, 48);
	Painter::DrawBorder(g, 0xff000000, insets.left, 16+font->GetSize()+32, sw, 48);
}

void LevelFrame::Run()
{
	while (_timeout > 0) {
		sleep(1);

		_timeout = _timeout - 1;
	}

	Hide();
}

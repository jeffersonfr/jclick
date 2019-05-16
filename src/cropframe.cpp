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
#include "cropframe.h"
#include "camerasettings.h"
#include "painter.h"
#include "mainframe.h"

#define CROP_LINE_SIZE 4

CropFrame::CropFrame(std::string title, jgui::jregion_t<int> region, jgui::jinsets_t insets):
	jgui::Container(/* title, */ region.x, region.y, region.width, region.height)
{
	_crop_region = region;
	_crop_insets = insets;
	_updated = false;
}

CropFrame::~CropFrame()
{
}

jgui::jinsets_t CropFrame::GetCrop()
{
	return _crop_insets;
}

bool CropFrame::KeyPressed(jevent::KeyEvent *event)
{
	if (jgui::Container::KeyPressed(event) == true) {
		return true;
	}

	bool exit = (event->GetSymbol() == jevent::JKS_ESCAPE || event->GetSymbol() == jevent::JKS_EXIT);

	if (exit == true || event->GetSymbol() == jevent::JKS_BACKSPACE) {
		// TODO:: Release()
		// Release();

		return true;
	}

	if (event->GetSymbol() == jevent::JKS_R || event->GetSymbol() == jevent::JKS_r) {
		_crop_insets.left = 0;
		_crop_insets.top = 0;
		_crop_insets.right = 0;
		_crop_insets.bottom = 0;
	} else if (event->GetSymbol() == jevent::JKS_CURSOR_LEFT) {
		_crop_insets.left = _crop_insets.left - 1;
	} else if (event->GetSymbol() == jevent::JKS_CURSOR_RIGHT) {
		_crop_insets.left = _crop_insets.left + 1;
	} else if (event->GetSymbol() == jevent::JKS_CURSOR_UP) {
		_crop_insets.top = _crop_insets.top - 1;
	} else if (event->GetSymbol() == jevent::JKS_CURSOR_DOWN) {
		_crop_insets.top = _crop_insets.top + 1;
	} else if (event->GetSymbol() == jevent::JKS_ENTER) {
		_updated = true;

		GetParams()->SetIntegerParam("left", _crop_insets.left);
		GetParams()->SetIntegerParam("top", _crop_insets.top);
		GetParams()->SetIntegerParam("right", _crop_insets.right);
		GetParams()->SetIntegerParam("bottom", _crop_insets.bottom);

		// TODO:: DispatchDataEvent(GetParams());
	}

	if (_crop_insets.left < 0) {
		_crop_insets.left = 0;
	}

	if (_crop_insets.left > 40) {
		_crop_insets.left = 40;
	}

	if (_crop_insets.top < 0) {
		_crop_insets.top = 0;
	}

	if (_crop_insets.top > 40) {
		_crop_insets.top = 40;
	}

	_crop_insets.right = _crop_insets.right;
	_crop_insets.bottom = _crop_insets.top;

	Repaint();

	return true;
}

void CropFrame::Paint(jgui::Graphics *g)
{
	jgui::Container::Paint(g);

	jgui::jregion_t<int> bounds = GetVisibleBounds();

	bounds.width = bounds.width-2*CROP_LINE_SIZE;
	bounds.height = bounds.height-2*CROP_LINE_SIZE;

	int tx = (_crop_insets.left*bounds.width)/100;
	int ty = (_crop_insets.top*bounds.height)/100;

	Painter::DrawBorder(g, 0xf0f00000, tx+CROP_LINE_SIZE, ty+CROP_LINE_SIZE, bounds.width-2*tx, bounds.height-2*ty, CROP_LINE_SIZE);
	
	int bw = 320,
			bh = 120,
			bx = (bounds.width-bw)/2,
			by = (bounds.height-bh)/2;
	char tmp[256];

	sprintf(tmp, "%d%%, %d%%", _crop_insets.left, _crop_insets.top);

	Painter::DrawBox(g, 0xffcfcdc8, bx, by, bw, bh);
		
	if (_updated == true) {
		Painter::DrawString(g, 0, 0, 0xf0f00000, bx, by, bw, bh, tmp);
	} else {
		Painter::DrawString(g, 0, 0, 0xf0000000, bx, by, bw, bh, tmp);
	}

	_updated = false;
}


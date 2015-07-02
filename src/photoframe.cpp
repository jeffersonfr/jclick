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
#include "photoframe.h"
#include "painter.h"
#include "config.h"
#include "jsystem.h"
#include "jfile.h"
#include "jioexception.h"

struct ascending_sort {
	bool operator()(std::string a, std::string b)
	{
		if (a < b) {
			return true;
		}

		return false;
	}
};

PhotoFrame::PhotoFrame(std::string path):
	jgui::Frame(__L->GetParam("photoframe.title"))
{
	_path = path;
	_index = -1;
	_message =  "Insert a device with photos";

	std::vector<std::string> files;

	jio::File file(path);

	file.ListFiles(&files);

	for (std::vector<std::string>::iterator i=files.begin(); i!=files.end(); i++) {
		std::string filepath = path + "/" + (*i);

		try {
			jio::File file(filepath);

			if (file.GetType() == jio::JFT_REGULAR) {
				if (filepath.size() > 4 && (
							strcasecmp((const char *)filepath.c_str()+filepath.size()-3, "png") == 0 || 
							strcasecmp((const char *)filepath.c_str()+filepath.size()-3, "jpg") == 0
							)) {

					_images.push_back(filepath);
				}
			}
		} catch (jio::IOException &e) {
		}
	}

	std::sort(_images.begin(), _images.end(), ascending_sort());

	if (_images.size() == 0) {
		_index = -1;
		_message = "No photos in device !";
	} else {
		_index = 0;
		_message = "";
	}
}

PhotoFrame::~PhotoFrame()
{
}

bool PhotoFrame::KeyPressed(jgui::KeyEvent *event)
{
	if (jgui::Frame::KeyPressed(event) == true) {
		return true;
	}

	int old = _index;

	if (_images.size() > 0) {
		if (event->GetSymbol() == jgui::JKS_CURSOR_LEFT) {
			if (--_index < 0) {
				_index = _images.size()-1;
			}
		} else if (event->GetSymbol() == jgui::JKS_CURSOR_RIGHT) {
			_index = (_index+1)%_images.size();
		}

		if (old != _index) {
			Repaint();
		}
	}

	return true;
}

void PhotoFrame::Paint(jgui::Graphics *g)
{
	jgui::Frame::Paint(g);

	jgui::jregion_t bounds = GetVisibleBounds();
	jgui::jinsets_t insets = GetInsets();

	int sw = bounds.width-insets.left-insets.right,
			sh = bounds.height-insets.top-insets.bottom;

	if (_images.size() == 0) {
		Painter::DrawBox(g, 0xffcfcdc8, insets.left, (2*sh)/4, sw, sh/5);
		Painter::DrawString(g, 1, 0, 0xf0000000, insets.left, (2*sh)/4, sw, sh/5, __L->GetParam("photoframe.no_photos"));

		return;
	}

	jgui::jsize_t isize = jgui::Image::GetImageSize(_images[_index]);

	jgui::jsize_t screen = jgui::GFXHandler::GetInstance()->GetScreenSize();

	float fw = isize.width,
				fh = isize.height,
				scale = 0.0;

	sh = sh - 2 * GetBorderSize();

	if (isize.width > sw) {
		fw = sw;
		scale = fw/isize.width;
		fh = isize.height*scale;

		if (fh > sh) {
			fh = sh;
			scale = fh/isize.height;
			fw = isize.width*scale;
		}
	} else if (isize.height > sh) {
		fh = sh;
		scale = fh/isize.height;
		fw = isize.width*scale;

		if (fw > sw) {
			fw = sw;
			scale = fw/isize.width;
			fh = isize.height*scale;
		}
	}

	jgui::Image *image = jgui::Image::CreateImage(_images[_index]);

	g->DrawImage(image, (int)((sw-fw)/2)+insets.left, (int)((sh-fh)/2)+insets.top+GetBorderSize(), (int)fw, (int)fh);
	
	delete image;

	jgui::Font *font = Painter::GetFont(0);

	int boxw = font->GetStringWidth("000 of 000")+16;
	int boxh = font->GetSize()+16;

	Painter::DrawBox(g, 0xffcfcdc8, insets.left, insets.top+8, boxw, boxh);
	Painter::DrawString(g, 0, 0, 0xf0000000, insets.left, insets.top+8, boxw, boxh, jgui::JHA_CENTER, jgui::JVA_CENTER, "%03d %s %03d", _index+1, __L->GetParam("of_label").c_str(), _images.size());

	g->SetVerticalSyncEnabled(true);
}

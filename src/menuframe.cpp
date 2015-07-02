/***************************************************************************
 *   Copyright (C) 2010 by Jeff Ferr                                       *
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
#include "menuframe.h"
#include "cropframe.h"
#include "networkframe.h"
#include "composeframe.h"
#include "photoframe.h"
#include "painter.h"
#include "mainframe.h"
#include "config.h"
#include "videocontrol.h"
#include "jgridlayout.h"
#include "jfilechooserdialogbox.h"
#include "jmessagedialogbox.h"

#include <iostream>
#include <iomanip>
#include <sstream>

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#define PushSubItem(id, value) { menu_options[id][menu_options[id].size()-1].elements.push_back(value); }

std::map<std::string, std::vector<struct options_t> > menu_options;

#define OPTIONS_COLS	3
#define OPTIONS_ROWS	2
#define OPTIONS_SIZE	3

std::string menu[][2] = {
	{"camera", "images/camera.png"},
	{"media", "images/media.png"},
	{"system", "images/system.png"},
};

#define GAPX		16
#define GAPY		16

#define TEXT_SIZE			72
#define TEXT_SPAN			(TEXT_SIZE+GAPY)

MenuFrame::MenuFrame(MainFrame *frame):
	jgui::Frame(__L->GetParam("menuframe.title"))
{
	SetDefaultExitEnabled(false);

	menu_options.clear();

	_current = NULL;
	_frame = frame;
	_index = 0;
	_state = "menu";

	for (int i=0; i<OPTIONS_SIZE; i++) {
		_images[menu[i][0]] = jgui::Image::CreateImage(__C->GetResourcesPath() + "/" + menu[i][1]);
	}

	_images["hspin"] = jgui::Image::CreateImage(__C->GetResourcesPath() + "/" + "images/hspin.png");
	_images["vspin"] = jgui::Image::CreateImage(__C->GetResourcesPath() + "/" + "images/vspin.png");

	SetBounds((_size.width-_size.width*0.60)/2, 64, _size.width*0.60, _size.height*0.70);

	Initialize();
}

MenuFrame::~MenuFrame()
{
	for (std::map<std::string, jgui::Image *>::iterator i=_images.begin(); i!=_images.end(); i++) {
		jgui::Image *image = i->second;

		delete image;
	}

	delete _current;
}

void MenuFrame::OnAction(std::string state, std::string id, int options_index)
{
	std::cout << "Action Event: " << state << " -> [" << menu_options[id][options_index].name << "]" << std::endl;

	if (_current != NULL) {
		delete _current;
		_current = NULL;
	}

	if (state == "menu.camera") {
		if (options_index == 0) {
			_image_index = 0;
			_state = "menu.camera.image";
		} else if (options_index == 1) {
			CropFrame *frame = new CropFrame(__L->GetParam("menuframe.camera.crop.title"), _frame->GetFrameBounds(), __C->GetSourceCrop());

			frame->GetParams()->SetTextParam("id", "crop");
			frame->RegisterDataListener(this);

			_current = frame;
		} else if (options_index == 2) {
			CropFrame *frame = new CropFrame(__L->GetParam("menuframe.camera.viewport.title"), _frame->GetViewportBounds(), __C->GetCameraViewport());

			frame->GetParams()->SetTextParam("id", "viewport");
			frame->RegisterDataListener(this);

			_current = frame;
		}
	} else if (state == "menu.camera.image") {
		if (options_index == 6) {
			_frame->ResetControlValues();
			
			menu_options[id][1].value = _frame->GetControlValue(GAMMA_CONTROL);
			menu_options[id][2].value = _frame->GetControlValue(BRIGHTNESS_CONTROL);
			menu_options[id][3].value = _frame->GetControlValue(CONTRAST_CONTROL);
			menu_options[id][4].value = _frame->GetControlValue(SATURATION_CONTROL);
			menu_options[id][5].value = _frame->GetControlValue(HUE_CONTROL);
		}
	} else if (state == "menu.media") {
		if (options_index == 0) {
			_current = new PhotoFrame(__C->GetPhotosPath());
		}
	} else if (state == "menu.system") {
		if (options_index == 2) {
			_current = new NetworkFrame(_frame);
		} else if (options_index == 4) {
			jgui::FileChooserDialogBox *frame = new jgui::FileChooserDialogBox(__L->GetParam("menuframe.system.load_configuration"), __C->GetResourcesPath());
			// jgui::FileChooserDialogBox *frame = new jgui::FileChooserDialogBox(__L->GetParam("menuframe.system.load_configuration"), __C->GetTextParam("config.directory"));
			
			frame->GetParams()->SetTextParam("id", "loadconf");
			frame->RegisterDataListener(this);

			_current = frame;
		}
	}

	if (_current != NULL) {
		_current->Show();
	}
}

void MenuFrame::DataChanged(jcommon::ParamMapper *params)
{
	jgui::jinsets_t insets;

	insets.left = params->GetIntegerParam("left");
	insets.top = params->GetIntegerParam("top");
	insets.right = params->GetIntegerParam("right");
	insets.bottom = params->GetIntegerParam("bottom");

	if (params->GetTextParam("id") == "crop") {
		__C->SetSourceCrop(insets);
	} else if (params->GetTextParam("id") == "viewport") {
		__C->SetCameraViewport(insets);
	} else if (params->GetTextParam("id") == "loadconf") {
		std::string directory = params->GetTextParam("directory");
		std::string filepath = params->GetTextParam("filepath");

		__C->SetTextParam("config.directory", directory);

		try {
			__C->LoadConfiguration(filepath);

			_frame->LoadResources();
		} catch (jcommon::RuntimeException &e) {
			jgui::MessageDialogBox msg(__L->GetParam("warning_label"), e.what());

			msg.Show();
		}
	}
	
	_frame->ReleaseFrame();
}

void MenuFrame::OnSelection(std::string state, std::string id, int options_index)
{
	std::cout << "Selection Event: " << state << " -> [" << menu_options[id][options_index].name << ":" << menu_options[id][options_index].value << "]" << std::endl;
		
	if (state == "menu.camera.image") {
		if (options_index == 0) {
			__C->SetCameraViewportAspect(menu_options[id][options_index].elements[menu_options[id][options_index].value]);

			_frame->InitializeRegions();
		} else if (options_index == 1) {
			_frame->SetControlValue(GAMMA_CONTROL, menu_options[id][options_index].value);
		} else if (options_index == 2) {
			_frame->SetControlValue(BRIGHTNESS_CONTROL, menu_options[id][options_index].value);
		} else if (options_index == 3) {
			_frame->SetControlValue(CONTRAST_CONTROL, menu_options[id][options_index].value);
		} else if (options_index == 4) {
			_frame->SetControlValue(SATURATION_CONTROL, menu_options[id][options_index].value);
		} else if (options_index == 5) {
			_frame->SetControlValue(HUE_CONTROL, menu_options[id][options_index].value);
		}
	} else if (state == "menu.system") {
		if (options_index == 0) {
			__C->SetSystemLanguage(menu_options[id][options_index].elements[menu_options[id][options_index].value]);

			Initialize();
		} else if (options_index == 1) {
			__C->SetImageFormat(menu_options[id][options_index].elements[menu_options[id][options_index].value]);
		}
	}
}

void MenuFrame::ProcessKeyDown(jgui::jkeyevent_symbol_t key, std::string state, std::string id, int &options_index)
{
	int value = menu_options[id][options_index].value,
			min = menu_options[id][options_index].min,
			max = menu_options[id][options_index].max;
	item_type_t type = menu_options[id][options_index].type;

	if (key == jgui::JKS_ENTER) {
		if (type == ACTION_ITEM || type == TIME_ITEM) {
			OnAction(state, id, options_index);
		}
	} else if (key == jgui::JKS_CURSOR_LEFT) {
		if (value > 0 && value > min) {
			menu_options[id][options_index].value = value - 1;

			OnSelection(state, id, options_index);
		}
	} else if (key == jgui::JKS_CURSOR_RIGHT) {
		if (value >= 0 && value < max) {
			menu_options[id][options_index].value = value + 1;

			OnSelection(state, id, options_index);
		}
	} else if (key == jgui::JKS_CURSOR_UP) {
		options_index = options_index - 1;
	} else if (key == jgui::JKS_CURSOR_DOWN) {
		options_index = options_index + 1;
	}

	if (options_index < 0) {
		options_index = menu_options[id].size() - 1;
	}

	if (options_index >= (int)menu_options[id].size()) {
		options_index = 0;
	}
}

void MenuFrame::DrawOptions(jgui::Graphics *g, std::string title, std::string id, int options_index)
{
	uint16_t cw, 
					 ch;

	cw = GetWidth();
	ch = GetHeight();

	int x = 0,
			y = 0,
			w = (cw - 2*x),
			h = (ch - 2*y);
	int tx = GAPX,
			ty = TEXT_SIZE+2*GAPY,
			tw = w-2*GAPX,
			th = TEXT_SIZE;
	int sw = 16*GAPX,
			sh = TEXT_SIZE-2*GAPY;

	std::string image_id;

	if (id == "camera") {
		image_id = "camera";
	} else if (id == "image") {
		image_id = "image";
	} else if (id == "media") {
		image_id = "media";
	} else if (id == "system") {
		image_id = "system";
	}

	for (int i=0; i<(int)menu_options[id].size(); i++) {
		jgui::Color color = jgui::Color::Gray;

		if (i == options_index) {
			color = jgui::Color::White;
		}

		Painter::DrawBorder(g, color, tx, ty+i*(TEXT_SIZE+GAPY), tw, th);
		Painter::DrawString(g, 0, 1, 0xfff0f0f0, tx+GAPX, ty+i*(TEXT_SIZE+GAPY), tw, th, jgui::JHA_LEFT, jgui::JVA_CENTER, menu_options[id][i].name.c_str());

		std::ostringstream o;
		int value = menu_options[id][i].value;
		item_type_t type = menu_options[id][i].type;

		if (type != LABEL_ITEM && type != ACTION_ITEM) {
			g->DrawImage(_images["hspin"], tx+tw-sw-GAPX, ty+i*(TEXT_SIZE+GAPY)+GAPY, sw, sh);
		}

		if (type == TIME_ITEM) {
			int h = value/3600,
					m = (value%3600)/60,
					s = (value%60);

			o << std::setw(2) << std::setfill('0') << h << ":" 
				<< std::setw(2) << std::setfill('0') << m << ":" 
				<< std::setw(2) << std::setfill('0') << s;
		} else if (type == LIST_ITEM) {
			o << menu_options[id][i].elements[value];
		} else if (type == LABEL_ITEM) {
			o << menu_options[id][i].elements[value];
		} else if (type == RANGE_ITEM) {
			o << value;
		}

		if (o.str().empty() == false) {
			if (type == LABEL_ITEM) {
				Painter::DrawString(g, 0, 1, 0xfff0f0f0, tx+tw-sw-GAPX, ty+i*(TEXT_SIZE+GAPY)+GAPY, sw, sh, jgui::JHA_RIGHT, jgui::JVA_CENTER, o.str().c_str());
			} else {
				Painter::DrawString(g, 0, 1, 0xfff0f0f0, tx+tw-sw-GAPX, ty+i*(TEXT_SIZE+GAPY)+GAPY, sw, sh, jgui::JHA_CENTER, jgui::JVA_CENTER, o.str().c_str());
			}
		}
	}
}

void MenuFrame::PushItem(std::string id, std::string name, int value, int min, int max, item_type_t type)
{
	struct options_t t;

	t.name = name;
	t.value = value;
	t.min = min;
	t.max = max;
	t.type = type;

	menu_options[id].push_back(t);
}

void MenuFrame::Initialize()
{
	menu_options.clear();

	// camera options
	PushItem("camera", __L->GetParam("menuframe.camera.image.title"), 0, 0, 0, ACTION_ITEM);
	PushItem("camera", __L->GetParam("menuframe.camera.crop.title"), 0, 0, 0, ACTION_ITEM);
	PushItem("camera", __L->GetParam("menuframe.camera.viewport.title"), 0, 0, 0, ACTION_ITEM);

	// image options
	PushItem("image", __L->GetParam("menuframe.camera.image.aspect"), 0, 0, 2-1, LIST_ITEM);
		PushSubItem("image", "keep");
		PushSubItem("image", "full");
	PushItem("image", __L->GetParam("menuframe.camera.image.gamma"), _frame->GetControlValue(GAMMA_CONTROL), 0, 100, RANGE_ITEM);
	PushItem("image", __L->GetParam("menuframe.camera.image.brightness"), _frame->GetControlValue(BRIGHTNESS_CONTROL), 0, 100, RANGE_ITEM);
	PushItem("image", __L->GetParam("menuframe.camera.image.contrast"), _frame->GetControlValue(CONTRAST_CONTROL), 0, 100, RANGE_ITEM);
	PushItem("image", __L->GetParam("menuframe.camera.image.saturation"), _frame->GetControlValue(SATURATION_CONTROL), 0, 100, RANGE_ITEM);
	PushItem("image", __L->GetParam("menuframe.camera.image.hue"), _frame->GetControlValue(HUE_CONTROL), 0, 100, RANGE_ITEM);
	PushItem("image", __L->GetParam("menuframe.camera.image.reset"), 0, 0, 0, ACTION_ITEM);

	// media options
	PushItem("media", __L->GetParam("menuframe.media.viewer.title"), 0, 0, 0, ACTION_ITEM);

	// system options
	PushItem("system", __L->GetParam("menuframe.system.language"), 0, 0, 2-1, LIST_ITEM);
		PushSubItem("system", "en-US");
		PushSubItem("system", "pt-BR");
	PushItem("system", __L->GetParam("menuframe.system.format"), 0, 0, 2-1, LIST_ITEM);
		PushSubItem("system", "jpg");
		PushSubItem("system", "png");
	PushItem("system", __L->GetParam("menuframe.system.network"), 0, 0, 0, ACTION_ITEM);
	PushItem("system", __L->GetParam("menuframe.system.version"), 0, 0, 1-1, LABEL_ITEM);
		PushSubItem("system", "1.0.0b");
	PushItem("system", __L->GetParam("menuframe.system.load_configuration"), 0, 0, 0, ACTION_ITEM);
	PushItem("system", __L->GetParam("menuframe.system.reset"), 0, 0, 0, ACTION_ITEM);
	
	// INFO:: adjust indexes 
	if (__C->GetCameraViewportAspect() == CVA_KEEP) {
		menu_options["image"][0].value = 0;
	} else if (__C->GetCameraViewportAspect() == CVA_FULL) {
		menu_options["image"][0].value = 1;
	}

	if (__C->GetSystemLanguage() == "en-US") {
		menu_options["system"][0].value = 0;
	} else if (__C->GetSystemLanguage() == "pt-BR") {
		menu_options["system"][0].value = 1;
	}

	if (__C->GetImageFormat() == "jpg") {
		menu_options["system"][1].value = 0;
	} else if (__C->GetImageFormat() == "png") {
		menu_options["system"][1].value = 1;
	}
}

bool MenuFrame::KeyPressed(jgui::KeyEvent *event)
{
	if (jgui::Frame::KeyPressed(event) == true) {
		return true;
	}

	bool exit = (event->GetSymbol() == jgui::JKS_ESCAPE || event->GetSymbol() == jgui::JKS_EXIT);

	if (exit == true || event->GetSymbol() == jgui::JKS_BACKSPACE) {
		if (_state == "menu") {
			Release();

			return true;
		}

		_state = _state.substr(0, _state.rfind("."));

		Repaint();
	} else if (_state == "main") {
		if (event->GetSymbol() == jgui::JKS_M || event->GetSymbol() == jgui::JKS_m) {
			_index = 0;
			_state = "menu";
		}
	} else if (_state == "menu") {
		if (event->GetSymbol() == jgui::JKS_ENTER) {
			if (_index == 0) {
				_camera_index = 0;
				_state = "menu.camera";
			} else if (_index == 1) {
				_media_index = 0;
				_state = "menu.media";
			} else if (_index == 2) {
				_system_index = 0;
				_state = "menu.system";
			}
		} else if (event->GetSymbol() == jgui::JKS_CURSOR_LEFT) {
			_index = _index - 1;
		} else if (event->GetSymbol() == jgui::JKS_CURSOR_RIGHT) {
			_index = _index + 1;
		} else if (event->GetSymbol() == jgui::JKS_CURSOR_UP) {
			if (_index >= 3) {
				_index = _index - 3;
			}
		} else if (event->GetSymbol() == jgui::JKS_CURSOR_DOWN) {
			if (_index < 3) {
				_index = _index + 3;
			}
		}

		if (_index < 0) {
			_index = OPTIONS_SIZE-1;
		}

		if (_index >= OPTIONS_SIZE) {
			_index = 0;
		}
	} else if (_state == "menu.camera") {
		ProcessKeyDown(event->GetSymbol(), "menu.camera", "camera", _camera_index);
	} else if (_state == "menu.camera.image") {
		ProcessKeyDown(event->GetSymbol(), "menu.camera.image", "image", _image_index);
	} else if (_state == "menu.media") {
		ProcessKeyDown(event->GetSymbol(), "menu.media", "media", _media_index);
	} else if (_state == "menu.system") {
		ProcessKeyDown(event->GetSymbol(), "menu.system", "system", _system_index);
	}

	if (_state == "menu") {
		SetTitle(__L->GetParam("menuframe.title"));
	} else if (_state == "menu.camera") {
		SetTitle(__L->GetParam("menuframe.camera.title"));
	} else if (_state == "menu.media") {
		SetTitle(__L->GetParam("menuframe.media.title"));
	} else if (_state == "menu.system") {
		SetTitle(__L->GetParam("menuframe.system.title"));
	}

	Repaint();

	return true;
}

void MenuFrame::DrawMenu(jgui::Graphics *g)
{
	uint16_t cw, 
					 ch;

	cw = GetWidth();
	ch = GetHeight();

	int x = 0,
			y = 0,
			w = (cw - 2*x),
			h = (ch - 2*y);
	int igx = 4*GAPX,
			igy = 2*GAPY,
			ix = igx,
			iy = TEXT_SIZE+igy,
			iw = (w-4*igx)/OPTIONS_COLS,
			ih = (h-TEXT_SIZE-4*igy)/OPTIONS_ROWS;

	for (int i=0; i<OPTIONS_SIZE; i++) {
		if (i != _index) {
			g->SetColor(jgui::Color::Gray);
		} else {
			g->SetColor(jgui::Color::White);
		}

		g->DrawRectangle(ix+(i%OPTIONS_COLS)*(iw+igx), iy+(i/OPTIONS_COLS)*(ih+igy), iw, ih);
		g->DrawImage(_images[menu[i][0]], ix+(i%OPTIONS_COLS)*(iw+igx), iy+(i/OPTIONS_COLS)*(ih+igy), iw, ih);
	}
}

void MenuFrame::Paint(jgui::Graphics *g)
{
	jgui::Frame::Paint(g);

	if (_state == "menu") {
		DrawMenu(g);
	} else if (_state == "menu.camera") {
		DrawOptions(g, "Camera Settings", "camera", _camera_index);
	} else if (_state == "menu.camera.image") {
		DrawOptions(g, "Image Settings", "image", _image_index);
	} else if (_state == "menu.media") {
		DrawOptions(g, "Media Tools", "media", _media_index);
	} else if (_state == "menu.system") {
		DrawOptions(g, "System Settings", "system", _system_index);
	}
}

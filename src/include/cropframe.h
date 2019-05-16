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
#ifndef __CROPFRAME_PHOTOBOOTH_H
#define __CROPFRAME_PHOTOBOOTH_H

#include "jgui/jcontainer.h"

#include <string>
#include <vector>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

class MainFrame;

class CropFrame : public jgui::Container {

	private:
		jgui::jregion_t<int> _crop_region;
		jgui::jinsets_t _crop_insets;
		bool _updated;

	public:
		CropFrame(std::string title, jgui::jregion_t<int> region, jgui::jinsets_t insets);
		virtual ~CropFrame();

		virtual jgui::jinsets_t GetCrop();
		virtual bool KeyPressed(jevent::KeyEvent *event);
		virtual void Paint(jgui::Graphics *g);

};

#endif

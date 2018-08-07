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
#ifndef __LEVELFRAME_LEVELBOOTH_H
#define __LEVELFRAME_LEVELBOOTH_H

#include "jgui/jcontainer.h"

#include <string>
#include <thread>
#include <vector>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

class LevelFrame : public jgui::Component {

	private:
    std::thread _thread;
		std::string _id;
		int _level;
		int _timeout;
    bool _is_running;

	public:
		LevelFrame(jgui::Container *parent);
		virtual ~LevelFrame();

		virtual void Show(std::string id, int level);
		virtual void Hide();
		virtual void Paint(jgui::Graphics *g);
		virtual void Run();

};

#endif

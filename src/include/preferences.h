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
#ifndef __PREFERENCES_PHOTOBOOTH_H
#define __PREFERENCES_PHOTOBOOTH_H

#include "jparammapper.h"

#include <iostream>
#include <map>
#include <string>

/**
 * \brief
 *
 * \author Jeff Ferr
 */
class Element : public jcommon::ParamMapper {

	private:
		/** \brief Comment */
		jcommon::ParamMapper _attributes;
		/** \brief Comment */
		std::string _name;

	public:
		/**
		 * \brief Construtor default.
		 *
		 */
		Element(std::string name);

		/**
		 * \brief Destrutor virtual.
		 *
		 */
		virtual ~Element();

		/**
		 * \brief 
		 *
		 */
		virtual std::string GetName();

		/**
		 * \brief 
		 *
		 */
		virtual jcommon::ParamMapper * GetAttributes();

};
/**
 * \brief
 *
 * \author Jeff Ferr
 */
class Document {

	friend class Preferences;

	private:
		/** \brief */
		std::vector<Element *> _elements;
		/** \brief Comment */
		std::string _name;

	public:
		/**
		 * \brief Construtor default.
		 *
		 */
		Document(std::string name);

		/**
		 * \brief Destrutor virtual.
		 *
		 */
		virtual ~Document();

		/**
		 * \brief 
		 *
		 */
		virtual std::string GetName();

		/**
		 * \brief
		 *
		 */
		virtual void AddElement(Element *e);

		/**
		 * \brief
		 *
		 */
		virtual void RemoveElementByID(std::string id);

		/**
		 * \brief
		 *
		 */
		virtual void RemoveElementByIndex(int index);
		/**
		 * \brief
		 *
		 */
		virtual void RemoveElements();

		/**
		 * \brief
		 *
		 */
		virtual Element * GetElementByID(std::string id);

		/**
		 * \brief
		 *
		 */
		virtual Element * GetElementByIndex(int index);

		/**
		 * \brief
		 *
		 */
		virtual std::vector<Element *> GetElements();

};

/**
 * \brief
 *
 * \author Jeff Ferr
 */
class Preferences {

	private:
		/**
		 * \brief Construtor default.
		 *
		 */
		Preferences();

	public:
		/**
		 * \brief Destrutor virtual.
		 *
		 */
		virtual ~Preferences();

		/**
		 * \brief 
		 *
		 */
		static Document * Create(std::string prefs);

		/**
		 * \brief
		 *
		 */
		static void Store(Document *document);

};

#endif


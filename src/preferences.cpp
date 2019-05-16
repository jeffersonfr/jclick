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
#include "preferences.h"
#include "config.h"

#include "jcommon/jxmlparser.h"
#include "jio/jfileoutputstream.h"
#include "jio/jfile.h"
#include "jcommon/jstringutils.h"
#include "jexception/jruntimeexception.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <fcntl.h>

Element::Element(std::string name)
{
	_name = name;
}

Element::~Element()
{
}

std::string Element::GetName()
{
	return _name;
}

jcommon::ParamMapper * Element::GetAttributes()
{
	return &_attributes;
}
	
Document::Document(std::string name)
{
	_name = name;
}

Document::~Document()
{
}

std::string Document::GetName()
{
	return _name;
}

void Document::AddElement(Element *e)
{
	if (std::find(_elements.begin(), _elements.end(), e) != _elements.end()) {
		return;
	}

	_elements.push_back(e);
}

void Document::RemoveElementByID(std::string id)
{
	for (std::vector<Element *>::iterator i=_elements.begin(); i!=_elements.end(); i++) {
		Element *element = (*i);

		if (element->GetAttributes()->GetTextParam("id") == id) {
			_elements.erase(i);

			delete element;

			break;
		}
	}
}

void Document::RemoveElementByIndex(int index)
{
	if (index < 0 || index >= (int)_elements.size()) {
		return;
	}

	Element *element = _elements[index];

	_elements.erase(_elements.begin()+index);

	delete element;
}

void Document::RemoveElements()
{
	_elements.clear();
}

Element * Document::GetElementByID(std::string id)
{
	for (std::vector<Element *>::iterator i=_elements.begin(); i!=_elements.end(); i++) {
		Element *element = (*i);

		if (element->GetAttributes()->GetTextParam("id") == id) {
			return element;
		}
	}

	return NULL;
}

Element * Document::GetElementByIndex(int index)
{
	if (index < 0 || index >= (int)_elements.size()) {
		return NULL;
	}

	return _elements[index];
}

std::vector<Element *> Document::GetElements()
{
	return _elements;
}
Preferences::Preferences()
{
}

Preferences::~Preferences()
{
}

Document * Preferences::Create(std::string prefs)
{
	Document *document = new Document(prefs);

	std::string path = PREFERENCES_PATH(prefs, "xml");

	jcommon::XMLDocument doc;

	if (!doc.LoadFile(path.c_str())) {
		return NULL;
	}

	jcommon::XMLElement *root = doc.RootElement()->FirstChildElement();
	jcommon::XMLElement *psg;
	const jcommon::XMLAttribute *attr;

	while (root != NULL) {
		Element *element = new Element(root->Value());

		attr = root->FirstAttribute();

		while (attr != NULL) {
			const char *key = attr->Name(),
				*value = attr->Value();

			if (key != NULL && value != NULL) {
				element->GetAttributes()->SetTextParam(key, value);
			}

			attr = attr->Next();
		}

		psg = root->FirstChildElement();

		while (psg != NULL) {
			if (strcmp(psg->Value(), "param") == 0) {
				const char *key = psg->Attribute("key"),
					*value = psg->Attribute("value");

				if (key != NULL && value != NULL) {
					element->SetTextParam(key, value);
				}
			}
		
			psg = psg->NextSiblingElement();
		}

		document->AddElement(element);

		root = root->NextSiblingElement();
	}

	return document;
}

void Preferences::Store(Document *document)
{
	if (document == NULL) {
		return;
	}

	std::ostringstream o;

	o << "<" << document->GetName() << ">" << std::endl;

	std::vector<Element *> elements = document->GetElements();

	for (std::vector<Element *>::iterator i=elements.begin(); i!=elements.end(); i++) {
		Element *element = (*i);

		o << "  <" << element->GetName();

		std::map<std::string, std::string> attributes = element->GetAttributes()->GetParameters();
		std::map<std::string, std::string> parameters = element->GetParameters();

		for (std::map<std::string, std::string>::iterator j=attributes.begin(); j!=attributes.end(); j++) {
			o << " " << j->first << "=\"" << j->second << "\"";
		}

		if (parameters.size() == 0) {
			o << " />" << std::endl;
		} else {
			o << ">" << std::endl;

			for (std::map<std::string, std::string>::iterator j=parameters.begin(); j!=parameters.end(); j++) {
				o << "    <param key=\"" << j->first << "\" value=\"" << j->second << "\" />" << std::endl;
			}

			o << "  </" << element->GetName() << ">" << std::endl;
		}
	}
	
	o << "</" << document->GetName() << ">" << std::endl;

	std::string path = PREFERENCES_PATH(document->GetName(), "xml");

	jio::File *file = jio::File::OpenDirectory(path);

	file->Remove();

	delete file;
	file = NULL;

	jio::FileOutputStream fos(path);

	fos.Write(o.str().c_str(), o.str().size());
	fos.Flush();
}

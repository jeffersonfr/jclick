#ifndef __LANGUAGE_PHOTOBOOTH_H
#define __LANGUAGE_PHOTOBOOTH_H

#include "preferences.h"

class Language {

	private:
		static Language *_instance;
	
	private:
		Document *_root;

	public:
		Language();

		virtual ~Language();

		static Language * GetInstance();

		virtual std::string GetParam(std::string key);

};

#endif

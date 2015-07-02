#include "language.h"
#include "camerasettings.h"
#include "stdio.h"

Language *Language::_instance = new Language();

Language::Language()
{
	_root = Preferences::Create("strings");
}

Language::~Language()
{
	delete _root;
}

Language * Language::GetInstance()
{
	return _instance;
}

std::string Language::GetParam(std::string key)
{
	if (_root == NULL || _root->GetElementByID(CameraSettings::GetInstance()->GetSystemLanguage()) == NULL) {
		return "";
	}

	std::string value = _root->GetElementByID(CameraSettings::GetInstance()->GetSystemLanguage())->GetTextParam(key);

	if (value.empty() == true) {
		value = _root->GetElementByID(_root->GetElementByID("default")->GetAttributes()->GetTextParam("value"))->GetTextParam(key);
	}

	return value;
}


#ifndef __CONFIG_PHOTOBOOTH_H
#define __CONFIG_PHOTOBOOTH_H

#include "camerasettings.h"
#include "language.h"

#define PREFERENCES_PATH(id, ext) (std::string("/etc/"__LOCAL_MODULE_NAME__"/") + id + "." + ext)

#define __C (CameraSettings::GetInstance())
#define __L (Language::GetInstance())

#endif

#pragma once

#include "Module.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class UserInterfaceModule : public Module
{
  public:
    bool Init() override;
};
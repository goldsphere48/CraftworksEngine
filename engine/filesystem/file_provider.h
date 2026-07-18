#pragma once

#include "provider.h"

namespace cw::fs
{
    Provider* CreateFileProvider(const char* root_utf8);
}

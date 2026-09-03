#pragma once

#include "provider.h"

namespace cw::fs
{
    Provider* CreateFileProvider(const char* rootUtf8);
}

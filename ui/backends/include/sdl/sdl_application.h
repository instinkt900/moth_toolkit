#pragma once

#include "iapplication.h"

namespace backend::sdl {
	std::unique_ptr<IApplication*> CreateApplication();
}

#pragma once

#include "iapplication.h"

namespace backend::vulkan {
	std::unique_ptr<IApplication> CreateApplication();
}

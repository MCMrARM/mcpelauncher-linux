#pragma once

#include <functional>

#ifdef __APPLE__
#include "function_dawin.h"
#else
namespace mcpe {
template<typename T>
using function = std::function<T>;
}
#endif
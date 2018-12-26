#pragma once

#include <string>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define SHADER_ROOT std::string("shader/")
#else
#define SHADER_ROOT std::string("/home/slim/source/graphene2/build/package/share/graphene/shaders/")
#endif

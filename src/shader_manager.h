#pragma once

#include <bgfx/bgfx.h>
#include <core/platform.h>
#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/memory/allocator.h"

#if CROWN_PLATFORM_LINUX
	#define EXE_PREFIX "./"
	#define EXE_SUFFIX ""
#elif CROWN_PLATFORM_WINDOWS
	#define EXE_PREFIX ""
	#define EXE_SUFFIX ".exe"
#else
	#error "Unknown platform"
#endif // CROWN_PLATFORM_LINUX

#define EXE_PATH(exe) EXE_PREFIX exe EXE_SUFFIX


namespace crown {

static const char* shaderc_paths[] =
{
	EXE_PATH("shaderc"),
#if CROWN_DEBUG
	EXE_PATH("shaderc-debug")
#elif CROWN_DEVELOPMENT
	EXE_PATH("shaderc-development")
#else
	EXE_PATH("shaderc-release")
#endif
};


struct ShaderManager {
    bgfx::ProgramHandle compile(char *vs_path, char *vr_path, char* fs_path);
    Buffer read(const char* path);

};

}

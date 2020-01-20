/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "core/filesystem/types.h"
#include "core/list.h"
#include "core/memory/allocator.h"
#include "core/memory/linear_allocator.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "device/display.h"
#include "device/input_types.h"
#include "device/window.h"
//#include "renderer.h"

/// @defgroup Device Device
namespace crown
{
struct BgfxAllocator;
struct BgfxCallback;
//void render();

/// This is the place where to look for accessing all of
/// the engine subsystems and related stuff.
///
/// @ingroup Device
struct Device
{
	LinearAllocator _allocator;

	Filesystem* _data_filesystem;
	File* _last_log;
	BgfxAllocator* _bgfx_allocator;
	BgfxCallback* _bgfx_callback;
	InputManager* _input_manager;
	Display* _display;
	Window* _window;

	u16 _width;
	u16 _height;

	bool _quit;
	bool _paused;

	bool process_events(bool vsync);

	///
	Device();
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;

	/// Runs the engine.
	void run();

	/// Quits the application.
	void quit();

	/// Pauses the engine.
	void pause();

	/// Unpauses the engine.
	void unpause();

	/// Returns the main window resolution.
	void resolution(u16& width, u16& height);

	/// Renders @a world using @a camera.
};

/// Runs the engine.
void run();

/// Returns the device.
Device* device();

} // namespace crown

/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/array.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/filesystem_apk.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/path.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/math/matrix4x4.h"
#include "core/math/vector3.h"
#include "core/memory/memory.h"
#include "core/memory/proxy_allocator.h"
#include "core/memory/temp_allocator.h"
#include "core/os.h"
#include "core/strings/string.h"
#include "core/strings/string_stream.h"
#include "core/time.h"
#include "core/types.h"
#include "device/device.h"
#include "device/device_event_queue.h"
#include "device/input_device.h"
#include "device/input_manager.h"
#include <bgfx/bgfx.h>
#include <bx/allocator.h>
#include "renderer.h"
#include "renderer_triangle.h"
#include "renderer_texture.h"
#include "renderer_coord_system.h"
#include "renderer_camera.h"
#include "renderer_colors.h"
#include "renderer_basiclighting.h"

#define MAX_SUBSYSTEMS_HEAP 8 * 1024 * 1024


namespace crown
{

extern bool next_event(OsEvent& ev);

struct BgfxCallback : public bgfx::CallbackI
{
	virtual void fatal(const char* _filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char* _str)
	{
		CE_ASSERT(false, "Fatal error: 0x%08x: %s", _code, _str);
		CE_UNUSED(_filePath);
		CE_UNUSED(_line);
		CE_UNUSED(_code);
		CE_UNUSED(_str);
	}

	virtual void traceVargs(const char* /*_filePath*/, u16 /*_line*/, const char* _format, va_list _argList)
	{
		char buf[2048];
		strncpy(buf, _format, sizeof(buf)-1);
		buf[strlen32(buf)-1] = '\0'; // Remove trailing newline
	}

	virtual void profilerBegin(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/)
	{
	}

	virtual void profilerBeginLiteral(const char* /*_name*/, uint32_t /*_abgr*/, const char* /*_filePath*/, uint16_t /*_line*/)
	{
	}

	virtual void profilerEnd()
	{
	}

	virtual u32 cacheReadSize(u64 /*_id*/)
	{
		return 0;
	}

	virtual bool cacheRead(u64 /*_id*/, void* /*_data*/, u32 /*_size*/)
	{
		return false;
	}

	virtual void cacheWrite(u64 /*_id*/, const void* /*_data*/, u32 /*_size*/)
	{
	}

	virtual void screenShot(const char* /*_filePath*/, u32 /*_width*/, u32 /*_height*/, u32 /*_pitch*/, const void* /*_data*/, u32 /*_size*/, bool /*_yflip*/)
	{
	}

	virtual void captureBegin(u32 /*_width*/, u32 /*_height*/, u32 /*_pitch*/, bgfx::TextureFormat::Enum /*_format*/, bool /*_yflip*/)
	{
	}

	virtual void captureEnd()
	{
	}

	virtual void captureFrame(const void* /*_data*/, u32 /*_size*/)
	{
	}
};

struct BgfxAllocator : public bx::AllocatorI
{
	ProxyAllocator _allocator;

	BgfxAllocator(Allocator& a)
		: _allocator(a, "bgfx")
	{
	}

	virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* /*_file*/, u32 /*_line*/)
	{
		if (!_ptr)
			return _allocator.allocate((u32)_size, (u32)_align == 0 ? 16 : (u32)_align);

		if (_size == 0)
		{
			_allocator.deallocate(_ptr);
			return NULL;
		}

		// Realloc
		void* p = _allocator.allocate((u32)_size, (u32)_align == 0 ? 16 : (u32)_align);
		_allocator.deallocate(_ptr);
		return p;
	}
};


Device::Device()
	: _allocator(default_allocator(), MAX_SUBSYSTEMS_HEAP)
	, _data_filesystem(NULL)
	, _bgfx_allocator(NULL)
	, _bgfx_callback(NULL)
	, _input_manager(NULL)
	, _display(NULL)
	, _window(NULL)
	, _width(0)
	, _height(0)
	, _quit(false)
	, _paused(false)
{
}

bool Device::process_events(bool vsync)
{

	bool exit = false;
	bool reset = false;

	OsEvent event;
	while (next_event(event))
	{
		if (event.type == OsEventType::NONE)
			continue;

		switch (event.type)
		{
		case OsEventType::BUTTON:
		case OsEventType::AXIS:
		case OsEventType::STATUS:
			_input_manager->read(event);
			break;

		case OsEventType::RESOLUTION:
			_width  = event.resolution.width;
			_height = event.resolution.height;
			reset   = true;
			break;

		case OsEventType::EXIT:
			exit = true;
			break;

		case OsEventType::PAUSE:
			pause();
			break;

		case OsEventType::RESUME:
			unpause();
			break;

		case OsEventType::TEXT:
			break;

		default:
			CE_FATAL("Unknown OS event");
			break;
		}
	}

	if (reset)
		bgfx::reset(_width, _height, (vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE));

	return exit;
}

void Device::run()
{

#if CROWN_PLATFORM_ANDROID
	_data_filesystem = CE_NEW(_allocator, FilesystemApk)(default_allocator(), const_cast<AAssetManager*>((AAssetManager*)_options._asset_manager));
#else
	_data_filesystem = CE_NEW(_allocator, FilesystemDisk)(default_allocator());
	{
        char cwd[1024];
		const char* data_dir =  os::getcwd(cwd, sizeof(cwd));
		((FilesystemDisk*)_data_filesystem)->set_prefix(data_dir);
	}

#endif // CROWN_PLATFORM_ANDROID


	// Init all remaining subsystems
	_bgfx_allocator = CE_NEW(_allocator, BgfxAllocator)(default_allocator());
	_bgfx_callback  = CE_NEW(_allocator, BgfxCallback)();

	_display = display::create(_allocator);

	_width  = CROWN_DEFAULT_WINDOW_WIDTH;
	_height = CROWN_DEFAULT_WINDOW_HEIGHT;

	_window = window::create(_allocator);
    u16 window_x = 0;
    u16 window_y = 0;
	_window->open(window_x
		, window_y
		, _width
		, _height
		, 0
		);
	_window->set_title("tutorial");
    _window->set_cursor_mode(CursorMode::DISABLED);
	//_window->set_fullscreen(_boot_config.fullscreen);
	_window->bgfx_setup();

	bgfx::Init init;
	init.type     = bgfx::RendererType::Count;
	init.vendorId = BGFX_PCI_ID_NONE;
	init.resolution.width  = _width;
	init.resolution.height = _height;
	init.resolution.reset  =  BGFX_RESET_VSYNC;
	init.callback  = _bgfx_callback;
	init.allocator = _bgfx_allocator;
	bgfx::init(init);

	_input_manager    = CE_NEW(_allocator, InputManager)(default_allocator());

	u16 old_width = _width;
	u16 old_height = _height;
	s64 time_last = time::now();

    rendererbasiclighting::init(_width, _height);

    bgfx::setViewRect(0, 0, 0, uint16_t(_width), uint16_t(_width));
    while (!process_events(true) && !_quit)
    {
        const s64 time = time::now();
        const f32 dt   = f32(time::seconds(time - time_last));
        time_last = time;


        if (_width != old_width || _height != old_height)
        {
            old_width = _width;
            old_height = _height;
        }

        if (!_paused)
        {
        }
        rendererbasiclighting::render(_width, _height, dt);

        _input_manager->update();

        //const bgfx::Stats* stats = bgfx::getStats();


        bgfx::frame();
    }


	bgfx::shutdown();
	_window->close();
	window::destroy(_allocator, *_window);
	display::destroy(_allocator, *_display);
	CE_DELETE(_allocator, _bgfx_callback);
	CE_DELETE(_allocator, _bgfx_allocator);

	CE_DELETE(_allocator, _data_filesystem);

	_allocator.clear();
}

void Device::quit()
{
	_quit = true;
}

void Device::pause()
{
	_paused = true;
}

void Device::unpause()
{
	_paused = false;
}

void Device::resolution(u16& width, u16& height)
{
	width = _width;
	height = _height;
}


char _buffer[sizeof(Device)];
Device* _device = NULL;

void run()
{
	CE_ASSERT(_device == NULL, "Crown already initialized");
	_device = new (_buffer) Device();
	_device->run();
	_device->~Device();
	_device = NULL;
}

Device* device()
{
	return crown::_device;
}

} // namespace crown

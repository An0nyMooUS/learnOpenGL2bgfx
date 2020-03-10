#include "shader_manager.h"
#include "core/memory/temp_allocator.inl"
#include "core/process.h"
#include "core/strings/string_stream.h"
#include "core/strings/string_stream.inl"
#include "core/types.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/reader_writer.h"
#include "core/filesystem/path.h"
#include "core/containers/array.inl"
#include <stdio.h>


namespace crown {

static s32 run_external_compiler(Process& pr
    , const char* shaderc
    , const char* infile
    , const char* outfile
    , const char* varying
    , const char* type
    , const char* platform
    ) {

    const char* argv[] =
    {
        shaderc,
        "-f",
        infile,
        "-o",
        outfile,
        "--varyingdef",
        varying,
        "--type",
        type,
        "--platform",
        platform,
        "-i",
        "../../../shaders/common.sh",
        NULL,
    };

    if (strcmp("windows", platform) == 0)
    {
        argv[11] = "--profile";
        argv[12] = ((strcmp(type, "vertex") == 0) ? "vs_4_0" : "ps_4_0");
    }

    return pr.spawn(argv, ProcessFlags::STDOUT_PIPE | ProcessFlags::STDERR_MERGE);
}


bgfx::ProgramHandle ShaderManager::compile(char *vs_path, char *vr_path, char* fs_path) {
    // Invoke shaderc
    Process pr_vert;
    Process pr_frag;
    s32 sc;

    sc = run_external_compiler(pr_vert
        , shaderc_paths[0]
        , vs_path
        , "vs.bin"
        , vr_path
        , "vertex"
        , CROWN_PLATFORM_NAME
        );

    sc = run_external_compiler(pr_frag
        , shaderc_paths[0]
        , fs_path
        , "fs.bin"
        , vr_path
        , "fragment"
        , CROWN_PLATFORM_NAME
        );

    // Check shaderc exit code
    s32 ec;
    TempAllocator4096 ta;
    StringStream output_vert(ta);
    StringStream output_frag(ta);

    // Read error messages if any
    {
        char err[512];
        while (pr_vert.fgets(err, sizeof(err)) != NULL)
            output_vert << err;
    }
    ec = pr_vert.wait();
    if (ec != 0)
    {
        printf("Failed to compile vertex shader:\n%s\n", string_stream::c_str(output_vert));
    }
    // Read error messages if any
    {
        char err[512];
        while (pr_frag.fgets(err, sizeof(err)) != NULL)
            output_frag << err;
    }
    ec = pr_frag.wait();
    if (ec != 0)
    {
        printf("Failed to compile fragment shader:\n%s\n", string_stream::c_str(output_frag));
    }
    Buffer vs_buf = read("vs.bin");
    Buffer fs_buf = read("fs.bin");
    const bgfx::Memory *mem = bgfx::copy(vs_buf._data, vs_buf._size);
    mem->data[mem->size-1] = '\0';
    bgfx::ShaderHandle vs_handle = bgfx::createShader(mem);
    mem = bgfx::copy(fs_buf._data, fs_buf._size);
    mem->data[mem->size-1] = '\0';
    bgfx::ShaderHandle fs_handle = bgfx::createShader(mem);
    bgfx::ProgramHandle program = bgfx::createProgram(vs_handle, fs_handle, true);

    return program;

}

Buffer ShaderManager::read(const char *path) {
    FilesystemDisk data_filesystem(default_allocator());
    File* file = data_filesystem.open(path, FileOpenMode::READ);
	u32 size = file->size();
	Buffer buf(default_allocator());
	array::resize(buf, size);
	file->read(array::begin(buf), size);
	data_filesystem.close(*file);
	return buf;
}


}

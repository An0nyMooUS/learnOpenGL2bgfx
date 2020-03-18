#include <stb_image.h>
#include "shader_manager.h"
#include "renderer_depthtest.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include "core/math/types.h"
#include "core/math/color4.inl"
#include "core/math/vector3.inl"
#include "core/math/matrix4x4.inl"
#include "core/math/quaternion.inl"
#include "core/math/constants.h"
#include "camera.h"
#include <stdio.h>

namespace crown {
namespace rendererdepthtest {

    static float vertices[] = {
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  -0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  -0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, 0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  -0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  -0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, 0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  -0.5f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, 0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  -0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  -0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  -0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, 0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, 0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  -0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  -0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, 0.5f,  0.0f, 1.0f
    };

    float plane_vertices[] = {
         5.0f, -0.5f, -5.0f, 2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f, 0.0f, 0.0f,
        -5.0f, -0.5f,  5.0f, 0.0f, 2.0f,

         5.0f, -0.5f, -5.0f, 2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f, 0.0f, 2.0f,
         5.0f, -0.5f,  5.0f, 2.0f, 2.0f
    };

    static bgfx::VertexBufferHandle vbh_cube;
    static bgfx::VertexBufferHandle vbh_plane;
    static bgfx::TextureHandle th1;
    static bgfx::TextureHandle th2;
    static bgfx::UniformHandle tex1;
    static bgfx::UniformHandle tex2;
    static bgfx::VertexLayout layout;
    static bgfx::ProgramHandle program;
    static bgfx::ProgramHandle program_outline;
    static Camera cam;

    bool load_texture(bgfx::TextureHandle &handle, const char *path) {
        int w, h, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(path , &w, &h, &nrChannels, 0);
        const bgfx::Memory *im = bgfx::copy(data, nrChannels*w*h);
        if (data) {
            handle = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::RGB8, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, im);
            stbi_image_free(data);
            return true;
        }
        return false;
    }

    void init(u16 width, u16 height) {
        f32 aspect = (float)width/(float)height;
		Vector3 pos = vector3(0.0f, 0.0f, -10.0f);
		Quaternion rotation = QUATERNION_IDENTITY;
		Pose pose = {pos, rotation};
		CameraDesc cam_desc = {ProjectionType::PERSPECTIVE, 45.0, 0.1, 100.0};
		cam = camera_create(cam_desc, pose, aspect);

        layout
            .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();

        vbh_cube = bgfx::createVertexBuffer(bgfx::makeRef(vertices, sizeof(vertices)), layout);
        vbh_plane = bgfx::createVertexBuffer(bgfx::makeRef(plane_vertices, sizeof(plane_vertices)), layout);
        
        bool result;
        result = load_texture(th1, "../../../resources/images/marble.jpg");
        result = load_texture(th2, "../../../resources/images/metal.png");

        ShaderManager sm;
        program = sm.compile(
                "../../../shaders/depthtest/v_depthtest.sc",
                "../../../shaders/depthtest/varying.def.sc",
                "../../../shaders/depthtest/f_depthtest.sc"
                );

        program_outline = sm.compile(
                        "../../../shaders/depthtest/v_depthtest.sc",
                        "../../../shaders/depthtest/varying.def.sc",
                        "../../../shaders/depthtest/f_outline.sc"
                        );

        tex1 = bgfx::createUniform("texture1", bgfx::UniformType::Sampler);

        bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height)); 

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL,
                to_rgba(color4(0.1, 0.1, 0.1, 1.0)), 1.0, 0);

        bgfx::touch(0);
    }

    void render(u16 width, u16 height, f32 dt) {
		static f32 total_time = 0;
		fly_camera_update(cam, dt);

        Matrix4x4 view = get_view(cam);
        Matrix4x4 proj = cam.proj;
        bgfx::setViewTransform(0, to_float_ptr(view), to_float_ptr(proj));

		bgfx::setViewRect(0, 0, 0, width, height);
		bgfx::touch(0);
        uint64_t state =  BGFX_STATE_DEFAULT 
                        ^ BGFX_STATE_CULL_CW 
                        | BGFX_STATE_WRITE_Z 
                        | BGFX_STATE_DEPTH_TEST_LESS;

        uint64_t stencil =  BGFX_STENCIL_TEST_ALWAYS 
                          | BGFX_STENCIL_FUNC_REF(1) 
                          | BGFX_STENCIL_FUNC_RMASK(0xFF) 
                          | BGFX_STENCIL_OP_FAIL_S_KEEP
                          | BGFX_STENCIL_OP_FAIL_Z_KEEP
                          | BGFX_STENCIL_OP_PASS_Z_REPLACE;


		Matrix4x4 model;
		set_identity(model);
        set_translation(model, vector3(-1.0, 0.0, -1.0));
		bgfx::setTransform(to_float_ptr(model));
        bgfx::setTexture(0, tex1, th1);
        bgfx::setVertexBuffer(0, vbh_cube);
		bgfx::setState(state);
        bgfx::setStencil(stencil);
        bgfx::submit(0, program);
        
        set_translation(model, vector3(2.0, 0.0, 0.0));
		bgfx::setTransform(to_float_ptr(model));
        bgfx::setTexture(0, tex1, th1);
        bgfx::setVertexBuffer(0, vbh_cube);
		bgfx::setState(state);
        bgfx::setStencil(stencil);
        bgfx::submit(0, program);

        set_identity(model);
		bgfx::setTransform(to_float_ptr(model));
        bgfx::setTexture(0, tex1, th2);
        bgfx::setVertexBuffer(0, vbh_plane);
		bgfx::setState(state);
        //bgfx::setStencil(stencil);
        bgfx::submit(0, program);
		total_time += dt;

        state =  BGFX_STATE_DEFAULT 
                ^ BGFX_STATE_CULL_CW;

        stencil =   BGFX_STENCIL_TEST_NOTEQUAL
                  | BGFX_STENCIL_FUNC_REF(1) 
                  | BGFX_STENCIL_FUNC_RMASK(0xFF);

		set_identity(model);
        set_scale(model, vector3(1.05, 1.05, 1.05));
        set_translation(model, vector3(-1.0, 0.0, -1.0));
		bgfx::setTransform(to_float_ptr(model));
        bgfx::setTexture(0, tex1, th1);
        bgfx::setVertexBuffer(0, vbh_cube);
		bgfx::setState(state);
        bgfx::setStencil(stencil);
        bgfx::submit(0, program_outline);
        
        set_translation(model, vector3(2.0, 0.0, 0.0));
		bgfx::setTransform(to_float_ptr(model));
        bgfx::setTexture(0, tex1, th1);
        bgfx::setVertexBuffer(0, vbh_cube);
		bgfx::setState(state);
        bgfx::setStencil(stencil);
        bgfx::submit(0, program_outline);
    }
}
}

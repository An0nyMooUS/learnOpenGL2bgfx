#include "shader_manager.h"
#include "renderer_colors.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include "core/memory/memory.h"
#include "core/containers/types.h"
#include "core/containers/array.h"
#include "core/math/types.h"
#include "core/math/color4.h"
#include "core/math/vector3.h"
#include "core/math/matrix4x4.h"
#include "core/math/quaternion.h"
#include "core/time.h"
#include "camera.h"
#include <stdio.h>
#include <stb_image.h>

namespace crown {
namespace renderercolors {

	// set up vertex data and indices
    // ------------------------------------------------------------------
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    static float vertices[] = {
        -0.5f, -0.5f, 0.5f,
         0.5f, -0.5f, 0.5f,
         0.5f,  0.5f, 0.5f,
         0.5f,  0.5f, 0.5f,
        -0.5f,  0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,

        -0.5f, -0.5f,  -0.5f,
         0.5f, -0.5f,  -0.5f,
         0.5f,  0.5f,  -0.5f,
         0.5f,  0.5f,  -0.5f,
        -0.5f,  0.5f,  -0.5f,
        -0.5f, -0.5f,  -0.5f,

        -0.5f,  0.5f,  -0.5f,
        -0.5f,  0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f,  -0.5f,
        -0.5f,  0.5f,  -0.5f,

         0.5f,  0.5f,  -0.5f,
         0.5f,  0.5f, 0.5f,
         0.5f, -0.5f, 0.5f,
         0.5f, -0.5f, 0.5f,
         0.5f, -0.5f,  -0.5f,
         0.5f,  0.5f, -0.5f,

        -0.5f, -0.5f, 0.5f,
         0.5f, -0.5f, 0.5f,
         0.5f, -0.5f,  -0.5f,
         0.5f, -0.5f,  -0.5f,
        -0.5f, -0.5f,  -0.5f,
        -0.5f, -0.5f, 0.5f,

        -0.5f,  0.5f, 0.5f,
         0.5f,  0.5f, 0.5f,
         0.5f,  0.5f,  -0.5f,
         0.5f,  0.5f,  -0.5f,
        -0.5f,  0.5f,  -0.5f,
        -0.5f,  0.5f, 0.5f
    };
    Vector3 v[] = {
                      vector3( 0.0,  0.0,  0.0), 
                      vector3( 2.0,  5.0, 15.0), 
                      vector3(-1.5, -2.2, 2.5),  
                      vector3(-3.8, -2.0, 12.3),  
                      vector3( 2.4, -0.4, 3.5),  
                      vector3(-1.7,  3.0, 7.5),  
                      vector3( 1.3, -2.0, 2.5),  
                      vector3( 1.5,  2.0, 2.5), 
                      vector3( 1.5,  0.2, 1.5), 
                      vector3(-1.3,  1.0, 1.5)  };
	Array<Vector3> *cubePositions;

	static bgfx::ProgramHandle program_cube;
	static bgfx::ProgramHandle program_lamp;
	static bgfx::VertexBufferHandle vbh_cube;
	static bgfx::VertexBufferHandle vbh_lamp;
	static bgfx::UniformHandle object_color;
	static bgfx::UniformHandle light_color;
	static bgfx::VertexLayout layout;
	static Camera cam;
	Vector3 light_pos = vector3(1.2f, 1.0f, 2.0f);

	void init (u16 width, u16 height) 
	{
		const bx::Vec3 at = {0.0f, 0.0f, 0.0f};
		const bx::Vec3 eye = {0.0f, 0.0f, -10.0f};
		f32 aspect = (float)width/(float)height;
		Vector3 pos = vector3(0.0f, 0.0f, -10.0f);
		float view[16];
		bx::mtxLookAt(view, eye, at);
		Quaternion rotation = quaternion(to_matrix3x3(get_inverted(matrix4x4(view))));
		rotation = QUATERNION_IDENTITY;
		Pose pose = {pos, rotation};
		CameraDesc cam_desc = {ProjectionType::PERSPECTIVE, 45.0, 0.1, 100.0};
		cam = camera_create(cam_desc, pose, aspect);
		cubePositions = CE_NEW(default_allocator(), Array<Vector3>)(default_allocator());
		array::push(*cubePositions, v, 10);

		ShaderManager sm;
		program_cube = sm.compile(
				"../../../shaders/colors/v_colors.sc"
			   ,"../../../shaders/colors/varying_colors.def.sc"
			   ,"../../../shaders/colors/f_colors.sc"
		   );
		program_lamp = sm.compile(
				"../../../shaders/colors/v_lamp.sc"
			   ,"../../../shaders/colors/varying_lamp.def.sc"
			   ,"../../../shaders/colors/f_lamp.sc"
		   );

		layout
			.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.end();

		vbh_cube = bgfx::createVertexBuffer(
				bgfx::makeRef(vertices, sizeof(vertices)),
				layout
				);
		vbh_lamp = bgfx::createVertexBuffer(
				bgfx::makeRef(vertices, sizeof(vertices)),
				layout
				);
		object_color = bgfx::createUniform("object_color", bgfx::UniformType::Vec4);
		light_color = bgfx::createUniform("light_color", bgfx::UniformType::Vec4);

		bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
		
		bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, to_rgba(color4(0.2, 0.3, 0.3, 1.0)), 1.0f, 0);
		bgfx::touch(0);
	}

	void render (u16 width, u16 height, f32 dt) 
	{
		fly_camera_update(cam, dt);

		Matrix4x4 view = get_view(cam);
		Matrix4x4 proj = cam.proj;
		bgfx::setViewTransform(0, to_float_ptr(view), to_float_ptr(proj));

		bgfx::setViewRect(0, 0, 0, width, height);
		bgfx::touch(0);

		//render cube
		Matrix4x4 model;
		set_identity(model);
		bgfx::setTransform(to_float_ptr(model));
		Vector4 obj_color = vector4(1.0, 0.5, 0.31, 1.0);
		Vector4 lgt_color = vector4(1.0, 1.0, 1.0, 1.0);
		bgfx::setUniform(object_color, to_float_ptr(obj_color));
		bgfx::setUniform(light_color, to_float_ptr(lgt_color));

		bgfx::setVertexBuffer(0, vbh_cube);
		bgfx::setState(BGFX_STATE_DEFAULT ^ BGFX_STATE_CULL_CW);
		bgfx::submit(0, program_cube);

		set_translation(model, light_pos);
		set_scale(model, vector3(0.2, 0.2, 0.2));
		bgfx::setTransform(to_float_ptr(model));
		bgfx::setVertexBuffer(0, vbh_lamp);
		bgfx::setState(BGFX_STATE_DEFAULT ^ BGFX_STATE_CULL_CW);
		bgfx::submit(0, program_lamp);
	}
	
}
}
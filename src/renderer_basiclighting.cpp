#include "shader_manager.h"
#include "renderer_basiclighting.h" 
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

namespace crown {
namespace rendererbasiclighting {
	// set up vertex data and indices
    // ------------------------------------------------------------------
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    static float vertices[] = {
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
 
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
    };

	static bgfx::ProgramHandle program_cube;
	static bgfx::ProgramHandle program_lamp;
	static bgfx::VertexBufferHandle vbh_cube;
	static bgfx::VertexBufferHandle vbh_lamp;
	static bgfx::UniformHandle object_color;
	static bgfx::UniformHandle light_color;
	static bgfx::UniformHandle light_position;
	static bgfx::UniformHandle inv_model;
	static bgfx::UniformHandle view_pos;
	static bgfx::VertexLayout layout_cube;
	static bgfx::VertexLayout layout_light;
	static Vector3 light_pos = vector3(1.2, 1.0, 2.0);
    static Camera cam;

	void init (u16 width, u16 height) 
	{
		f32 aspect = (float)width/(float)height;
		Vector3 pos = vector3(0.0f, 0.0f, -10.0f);
		Quaternion rotation = QUATERNION_IDENTITY;
		Pose pose = {pos, rotation};
		CameraDesc cam_desc = {ProjectionType::PERSPECTIVE, 45.0, 0.1, 100.0};
		cam = camera_create(cam_desc, pose, aspect);

		ShaderManager sm;
		program_cube = sm.compile(
				 "../../../shaders/basiclighting/v_cube.sc"
				,"../../../shaders/basiclighting/varying_cube.def.sc"
				,"../../../shaders/basiclighting/f_cube.sc"
				);
		program_lamp = sm.compile(
				"../../../shaders/colors/v_lamp.sc"
			   ,"../../../shaders/colors/varying_lamp.def.sc"
			   ,"../../../shaders/colors/f_lamp.sc"
		   );

		layout_cube
			.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.end();
		layout_light
			.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.skip(3 * sizeof(bgfx::AttribType::Float))
			.end();

		vbh_cube = bgfx::createVertexBuffer(
				bgfx::makeRef(vertices, sizeof(vertices)),
				layout_cube
				);
		vbh_lamp = bgfx::createVertexBuffer(
				bgfx::makeRef(vertices, sizeof(vertices)),
				layout_light
				);
		object_color = bgfx::createUniform("object_color", bgfx::UniformType::Vec4);
		light_color = bgfx::createUniform("light_color", bgfx::UniformType::Vec4);
		light_position = bgfx::createUniform("light_pos", bgfx::UniformType::Vec4);
		inv_model = bgfx::createUniform("inv_model", bgfx::UniformType::Mat4);
		view_pos = bgfx::createUniform("view_position", bgfx::UniformType::Vec4);

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
		Vector4 lgt_pos = vector4(light_pos.x, light_pos.y, light_pos.z, 1.0);
		bgfx::setUniform(object_color, to_float_ptr(obj_color));
		bgfx::setUniform(light_color, to_float_ptr(lgt_color));
		bgfx::setUniform(light_position, to_float_ptr(lgt_pos));
		bgfx::setUniform(inv_model, to_float_ptr(get_inverted(model)));
		bgfx::setUniform(view_pos, to_float_ptr(cam.pose.position));

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

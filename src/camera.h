#pragma once

#include "world/types.h"
#include "core/containers/types.h" 
#include "core/math/types.h"

namespace crown {
    struct Pose {
        Vector3 position;
        Quaternion rotation;
    };

	struct Camera
	{
		UnitId unit;

		ProjectionType::Enum projection_type;

        Pose pose;
        Matrix4x4 proj;

		Frustum frustum;
		f32 fov;
		f32 aspect;
		f32 near_range;
		f32 far_range;

		// Orthographic projection only
		f32 half_size;

		u16 view_x;
		u16 view_y;
		u16 view_width;
		u16 view_height;
	};

    Camera camera_create(const CameraDesc& cd, const Pose pose, f32 aspect);
    Matrix4x4 get_view(const Camera& cam);
    void fly_camera_update(Camera& cam, f32 dt);
}

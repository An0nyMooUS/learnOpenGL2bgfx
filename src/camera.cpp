#include "camera.h"
#include <stdio.h>
#include "core/types.h"
#include "core/strings/string_id.h" 
#include "device/device.h"
#include "device/input_manager.h"
#include "device/input_device.h" 
#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace crown {
    Camera camera_create(const CameraDesc& cd, const Pose pose, f32 aspect) {
        Camera cam;
        //cam.unit         = id;
        cam.projection_type = (ProjectionType::Enum)cd.type;
        cam.fov = cd.fov;
        cam.near_range = cd.near_range;
        cam.far_range = cd.far_range;
        cam.aspect = aspect;

        cam.pose = pose;

        const bgfx::Caps *caps = bgfx::getCaps();
        f32 bx_proj[16];
        switch (cam.projection_type)
        {
        case ProjectionType::ORTHOGRAPHIC:
            bx::mtxOrtho(bx_proj
                , -cam.half_size * cam.aspect
                , cam.half_size * cam.aspect
                , -cam.half_size
                , cam.half_size
                , cam.near_range
                , cam.far_range
                , 0.0f
                , caps->homogeneousDepth
                );
            break;

        case ProjectionType::PERSPECTIVE:
            bx::mtxProj(bx_proj
                , fdeg(cam.fov)
                , cam.aspect
                , cam.near_range
                , cam.far_range
                , caps->homogeneousDepth
                );
            break;

        default:
            printf("error\n");
            break;
        }

        cam.proj = matrix4x4(bx_proj);

        return cam;
    }

    Matrix4x4 get_view(const Camera& cam) {
        Matrix4x4 view =  matrix4x4(cam.pose.rotation, cam.pose.position);
        invert(view);
        //to_string(view);
        return view;
    }

    void fly_camera_update(Camera& cam, f32 dt) {
        static bool wkey = false; 
        static bool skey = false;
        static bool akey = false;
        static bool dkey = false;
        float translation_speed = 0.1;
        float rotation_speed = 0.14;

        InputDevice *keyboard = device()->_input_manager->keyboard();
        if (keyboard->pressed(85)) wkey = true;
        if (keyboard->pressed(keyboard->button_id(StringId32("s")))) skey = true;
        if (keyboard->pressed(keyboard->button_id(StringId32("a")))) akey = true;
        if (keyboard->pressed(keyboard->button_id(StringId32("d")))) dkey = true;

        if (keyboard->released(keyboard->button_id(StringId32("w")))) wkey = false;
        if (keyboard->released(keyboard->button_id(StringId32("s")))) skey = false;
        if (keyboard->released(keyboard->button_id(StringId32("a")))) akey = false;
        if (keyboard->released(keyboard->button_id(StringId32("d")))) dkey = false;

        InputDevice *mouse = device()->_input_manager->mouse();

        Vector3 delta = mouse->axis(mouse->axis_id(StringId32("cursor_delta")));
        float dx = delta.x;
        float dy = delta.y;

        Quaternion rotation_around_world_up = quaternion(vector3(0, 1, 0), rotation_speed*dx*dt);
		normalize(rotation_around_world_up);
        Quaternion rotation_around_camera_right = quaternion(vector3(1, 0, 0), rotation_speed*dy*dt);
		normalize(rotation_around_camera_right);

        cam.pose.rotation = rotation_around_world_up * cam.pose.rotation * rotation_around_camera_right;
		normalize(cam.pose.rotation);

        if (wkey) cam.pose.position += forward(cam.pose.rotation)*translation_speed;
        if (skey) cam.pose.position -= forward(cam.pose.rotation)*translation_speed;
        if (akey) cam.pose.position -= right(cam.pose.rotation)*translation_speed;
        if (dkey) cam.pose.position += right(cam.pose.rotation)*translation_speed;

    }

}

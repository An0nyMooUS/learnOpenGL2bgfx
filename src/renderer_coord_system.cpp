//#define STB_IMAGE_IMPLEMENTATION
#include "shader_manager.h"
#include "renderer_coord_system.h"
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
#include <stdio.h>
#include <stb_image.h>

namespace crown {
    namespace renderercoordsys {
// set up vertex data and indices
    // ------------------------------------------------------------------
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
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

    static bgfx::ProgramHandle program;
    static bgfx::VertexBufferHandle vbh;
    static bgfx::UniformHandle tex1;
    static bgfx::UniformHandle tex2;
    static bgfx::TextureHandle th1;
    static bgfx::TextureHandle th2;
    static bgfx::VertexLayout layout;

    void init(u16 width, u16 height) {
        cubePositions = CE_NEW(default_allocator(), Array<Vector3>)(default_allocator());
        array::push(*cubePositions, v, 10);
        int w, h, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load("../../../resources/images/container.jpg", &w, &h, &nrChannels, 0);
        const bgfx::Memory *im = bgfx::copy(data, nrChannels*w*h);

        int w2, h2, nrChannels2;
        unsigned char *data2= stbi_load("../../../resources/images/awesomeface.png", &w2, &h2, &nrChannels2, 0);
        const bgfx::Memory *im2 = bgfx::copy(data2, nrChannels2*w2*h2);
        if (data && data2)
        {
            //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            //// set the texture wrapping/filtering options (on the currently bound texture object)
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            th1 = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::RGB8, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, im);
            th2 = bgfx::createTexture2D(w2, h2, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, im2);
            printf("chegou w: %d\n", nrChannels2);
            //glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            printf("Failed to load texture\n");
        }
        stbi_image_free(data);
        stbi_image_free(data2);
        //build and compile our shader program
        ShaderManager sm;
        program = sm.compile(
                 "../../../shaders/coord_system/vs.sc"
                ,"../../../shaders/coord_system/varying.def.sc"
                ,"../../../shaders/coord_system/fs.sc"
        );
        //In bgfx we have to set layout before create buffer handle
        layout
            .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
        
        //create Buffer handler, (VAO, VBO, EBO stuff opengl)
        vbh = bgfx::createVertexBuffer(
                bgfx::makeRef(vertices, sizeof(vertices)),
                layout
                );

        tex1 = bgfx::createUniform("texture1", bgfx::UniformType::Sampler);
        tex2 = bgfx::createUniform("texture2", bgfx::UniformType::Sampler);

        // Set view rectangle for 0th view
        bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

        //Clear the view rect
        //bgfx::setViewClear(0, BGFX_CLEAR_COLOR , to_rgba(color4(0.2, 0.3, 0.3, 1.0)));
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, to_rgba(color4(0.2, 0.3, 0.3, 1.0)), 1.0f, 0);

        bgfx::touch(0);

    }

    void render(u16 width, u16 height, f32 dt) {
        const bx::Vec3 at  = { 0.0f, 0.0f,   0.0f };
        //const bx::Vec3 eye = { 0.0f, 0.0f, -5.0f };
        Vector3 eye = vector3(0.0f, 0.0f, 3.0f);
        //float view[16];
        Matrix4x4 view = matrix4x4(eye);
        //bx::mtxLookAt(view, eye, at);

        float proj[16];
        const bgfx::Caps* caps = bgfx::getCaps();
        bx::mtxProj(proj, 45.0, (float)800/(float)600, 0.1, 100.0, caps->homogeneousDepth);
        bgfx::setViewTransform(0, to_float_ptr(view), proj);


        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0,
                width,
                height);
        bgfx::touch(0);


        bgfx::setTexture(0, tex1, th1);
        bgfx::setTexture(1, tex2, th2);
        Matrix4x4 model;

        for (int i = 0; i < 10; ++i) {
            static float angle = 0;
            float speed = 50.0; //deg/s
            //angle += speed*dt;
            //if (i == 0)
            //angle -= speed*dt;
            angle = -20.0 * i;
            Vector3 axis = vector3(1.0, 0.3, -0.5);
            //Vector3 axis = vector3(0.0, 0.0, 1.0);
            normalize(axis);
            Quaternion rotation = quaternion(axis, frad(angle));
            //quaternion.x 
            Vector3 translation = (*cubePositions)[i];
            model = matrix4x4(rotation, translation);
            bgfx::setTransform(to_float_ptr(model));

            bgfx::setVertexBuffer(0, vbh);
            bgfx::setState(BGFX_STATE_DEFAULT ^ BGFX_STATE_CULL_CW);
            bgfx::submit(0, program);
        }

        // disable culling in default state and depth test
        //bgfx::setState(BGFX_STATE_DEFAULT ^ BGFX_STATE_CULL_CW ^ BGFX_STATE_DEPTH_TEST_LESS);

    }
}
}

#define STB_IMAGE_IMPLEMENTATION
#include "shader_manager.h"
#include "renderer_texture.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include <stdio.h>
#include <stb_image.h>

namespace crown {
    namespace renderertexture {  
// set up vertex data and indices
    // ------------------------------------------------------------------
    static float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };
    static const uint16_t indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    static bgfx::ProgramHandle program;
    static bgfx::VertexBufferHandle vbh;
    static bgfx::IndexBufferHandle ibh;
    static bgfx::UniformHandle tex1;
    static bgfx::UniformHandle tex2;
    static bgfx::TextureHandle th1;
    static bgfx::TextureHandle th2;
    static bgfx::VertexLayout layout;

    void init(u16 width, u16 height) {
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
                 "../../../shaders/hello-texture/v_texture.sc"
                ,"../../../shaders/hello-texture/varying_texture.def.sc"
                ,"../../../shaders/hello-texture/f_texture.sc"
        );
        //In bgfx we have to set layout before create buffer handle
        layout
            .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
        
        //create Buffer handler, (VAO, VBO, EBO stuff opengl)
        vbh = bgfx::createVertexBuffer(
                bgfx::makeRef(vertices, sizeof(vertices)),
                layout
                );
        printf("sizeof of vertices: %d\n", sizeof(vertices));
        ibh = bgfx::createIndexBuffer(
                bgfx::makeRef(indices, sizeof(indices))
                );

        tex1 = bgfx::createUniform("texture1", bgfx::UniformType::Sampler);
        tex2 = bgfx::createUniform("texture2", bgfx::UniformType::Sampler);

        // Set view rectangle for 0th view
        bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

        //Clear the view rect
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR , to_rgba(color4(0.2, 0.3, 0.3, 1.0)));

        bgfx::touch(0);

    }

    void render(u16 width, u16 height, f32 dt) {
        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0,
                width,
                height);


        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);
        bgfx::setTexture(0, tex1, th1);
        bgfx::setTexture(1, tex2, th2);

        // disable culling in default state and depth test
        bgfx::setState(BGFX_STATE_DEFAULT ^ BGFX_STATE_CULL_CW ^ BGFX_STATE_DEPTH_TEST_LESS);

        bgfx::submit(0, program);
    }
}
}

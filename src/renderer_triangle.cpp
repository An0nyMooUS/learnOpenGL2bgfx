#include "shader_manager.h"
#include "renderer_triangle.h"
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include <stdio.h>

namespace crown {
    namespace renderertriangle {
// set up vertex data and indices
    // ------------------------------------------------------------------
    static float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    static const uint16_t indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    bgfx::ProgramHandle program;
    bgfx::VertexBufferHandle vbh;
    bgfx::IndexBufferHandle ibh;
    static bgfx::VertexLayout layout;

    void init(u16 width, u16 height) {
        //build and compile our shader program
        ShaderManager sm;
        program = sm.compile(
                 "../../../shaders/hello-triangle/v_triangle.sc"
                ,"../../../shaders/hello-triangle/varying_triangle.def.sc"
                ,"../../../shaders/hello-triangle/f_triangle.sc"
        );
        //In bgfx we have to set layout before create buffer handle
        //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        //VertexLayout &add(Attrib::Enum _attrib, uint8_t _num, AttribType::Enum _type, bool _normalized = false, bool _asInt = false
        layout
            .begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .end();
        
        //create Buffer handler, (VAO, VBO, EBO stuff opengl)
        //unsigned int VBO, VAO, EBO;
        //glGenVertexArrays(1, &VAO);
        //glGenBuffers(1, &VBO);
        //glGenBuffers(1, &EBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        //glBindVertexArray(VAO);
        //glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        vbh = bgfx::createVertexBuffer(
                bgfx::makeRef(vertices, sizeof(vertices)),
                layout
                );

        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW
        ibh = bgfx::createIndexBuffer(
                bgfx::makeRef(indices, sizeof(indices))
                );

        // Set view rectangle for 0th view
        // glViewport(0, 0, width, height);
        // void bgfx::setViewRect(ViewId _id, uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height)
        bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

        //Clear the view rect
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, to_rgba(color4(0.2, 0.3, 0.3, 1.0)), 1.0f, 0);

        bgfx::touch(0);

    }


    void render(u16 width, u16 height, f32 dt) {
        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0,
                width,
                height);


        bgfx::setVertexBuffer(0, vbh);
        bgfx::setIndexBuffer(ibh);

        // disable culling in default state and depth test
        bgfx::setState(BGFX_STATE_DEFAULT ^ BGFX_STATE_CULL_CW ^ BGFX_STATE_DEPTH_TEST_LESS);

        bgfx::submit(0, program);
    }

}
}

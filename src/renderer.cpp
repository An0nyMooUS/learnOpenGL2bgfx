#include "shader_manager.h"
#include "renderer.h"
#include <bgfx/bgfx.h>
#include "core/time.h"
#include <bx/math.h>
namespace crown {

struct PosColorVertex {
    float m_x;
    float m_y;
    float m_z;
    uint32_t m_abgr;

    static void init() {
        ms_decl
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
            .end();
    };

    static bgfx::VertexLayout ms_decl;
};


bgfx::VertexLayout PosColorVertex::ms_decl;

static PosColorVertex s_cubeVertices[] =
{
    {  0.5f,  0.5f, 0.0f, 0xff0000ff },
    {  0.5f, -0.5f, 0.0f, 0xff0000ff },
    { -0.5f, -0.5f, 0.0f, 0xff00ff00 },
    { -0.5f,  0.5f, 0.0f, 0xff00ff00 }
};


static const uint16_t s_cubeTriList[] =
{
    0,1,3,
    1,2,3
};

bgfx::VertexBufferHandle m_vbh;
bgfx::IndexBufferHandle m_ibh;
bgfx::ProgramHandle m_program;

void Renderer::init(u16 width, u16 height) {
    ShaderManager sm;
    m_program = sm.compile(
            "../../../shaders/v_simple.sc"
            ,"../../../shaders/varying.def.sc"
            ,"../../../shaders/f_simple.sc"
            );

    PosColorVertex::init();
    m_vbh = bgfx::createVertexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)),
            PosColorVertex::ms_decl
            );

    m_ibh = bgfx::createIndexBuffer(
            // Static data can be passed with bgfx::makeRef
            bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList))
            );


    // Set view rectangle for 0th view
    bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

    // Clear the view rect
    bgfx::setViewClear(0,
            BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
            0x443355FF, 1.0f, 0);


    // Set empty primitive on screen
    bgfx::touch(0);

}

void Renderer::render(u16 width, u16 height) {
    const bx::Vec3 at  = { 0.0f, 0.0f,   0.0f };
    const bx::Vec3 eye = { 0.0f, 0.0f, 10.0f };

    // Set view and projection matrix for view 0.
    float view[16];
    bx::mtxLookAt(view, eye, at);

    float proj[16];
    bx::mtxProj(proj,
            60.0f,
            float(width)/float(height),
            0.1f, 100.0f,
            bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(0, view, proj);

    // Set view 0 default viewport.
    bgfx::setViewRect(0, 0, 0,
            width,
            height);

    bgfx::touch(0);


    float mtx[16];
    bx::mtxRotateY(mtx, 0.0f);

    // position x,y,z
    mtx[12] = 0.0f;
    mtx[13] = 0.0f;
    mtx[14] = 0.0f;

    // Set model matrix for rendering.
    bgfx::setTransform(mtx);

    // Set vertex and index buffer.
    bgfx::setVertexBuffer(0, m_vbh);
    bgfx::setIndexBuffer(m_ibh);

    // Set render states.
    bgfx::setState(BGFX_STATE_DEFAULT);

    // Submit primitive for rendering to view 0.
    bgfx::submit(0, m_program);
}
}

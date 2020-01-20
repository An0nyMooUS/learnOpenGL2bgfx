$input a_position, a_texcoord0
$output texCoord

#include <../common.sh>

void main() {
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    texCoord = a_texcoord0;
}

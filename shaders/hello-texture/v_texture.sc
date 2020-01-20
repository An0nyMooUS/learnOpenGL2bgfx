$input a_position, a_color0, a_texcoord0
$output ourColor, texCoord

#include <../common.sh>

void main() {
    //gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
    gl_Position = vec4(a_position, 1.0);
    ourColor = a_color0;
    texCoord = a_texcoord0;
}

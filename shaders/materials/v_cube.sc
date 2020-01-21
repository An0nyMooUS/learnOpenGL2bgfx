$input a_position, a_normal
$output v_normal, v_fragPos

#include <../common.sh>

uniform mat4 inv_model;

void main() {
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_fragPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
	v_normal = mat3(transpose(inv_model)) * a_normal;
}

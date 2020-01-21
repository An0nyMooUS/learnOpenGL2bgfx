#include <../common.sh>

uniform vec4 light_color;
uniform vec4 object_color;

void main() {
	float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * light_color.xyz;

	vec3 result = ambient * object_color.xyz;
	gl_FragColor = vec4(result, 1.0);
}

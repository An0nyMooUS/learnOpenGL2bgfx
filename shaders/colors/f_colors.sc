#include <../common.sh>

uniform vec4 object_color;
uniform vec4 light_color;

void main() {
	gl_FragColor = vec4(object_color.xyz * light_color.xyz, 1.0);
}

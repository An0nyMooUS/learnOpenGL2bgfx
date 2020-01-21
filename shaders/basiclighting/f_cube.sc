$input v_normal, v_fragPos

#include <../common.sh>

uniform vec4 light_color;
uniform vec4 object_color;
uniform vec4 light_pos;
uniform vec4 view_position;
#define view_pos view_position.xyz

void main() {
	float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * light_color.xyz;

	vec3 norm = normalize(v_normal);
	vec3 light_dir = normalize(light_pos.xyz - v_fragPos);
	float diffuse_impact = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = diffuse_impact * light_color.xyz;

	float specular_strength = 0.5;
	vec3 view_dir = normalize(view_pos - v_fragPos);
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 16);
	vec3 specular = specular_strength * spec * light_color.xyz;

	vec3 result = (ambient + diffuse + specular) * object_color.xyz;
	gl_FragColor = vec4(result, 1.0);
}

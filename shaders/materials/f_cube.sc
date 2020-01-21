$input v_normal, v_fragPos

#include <../common.sh>


uniform vec4 light_color;
uniform vec4 light_pos;
uniform vec4 view_position;
#define view_pos view_position.xyz

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular_shininess;
uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;


void main() {
	vec3 ambient = ambient.xyz * light_ambient.xyz;

	vec3 norm = normalize(v_normal);
	vec3 light_dir = normalize(light_pos.xyz - v_fragPos);
	float diffuse_impact = max(dot(norm, light_dir), 0.0);
	vec3 diffuse = (diffuse_impact * diffuse.xyz) * light_diffuse.xyz;

	vec3 view_dir = normalize(view_pos - v_fragPos);
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), specular_shininess.w);
	vec3 specular = (specular_shininess.xyz * spec) * light_specular.xyz;

	vec3 result = ambient + diffuse + specular;
	gl_FragColor = vec4(result, 1.0);
}

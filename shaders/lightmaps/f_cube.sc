$input v_normal, v_fragPos, v_texcoord0

#include <../common.sh>

uniform vec4 light_color;
uniform vec4 light_pos;
uniform vec4 view_position;

SAMPLER2D(diffuse, 0);
SAMPLER2D(specular, 1);
uniform vec4 specular_shininess;
uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

void main() {
	vec3 ambient = (light_ambient * texture2D(diffuse, v_texcoord0)).xyz;

	vec3 norm = normalize(v_normal);
	vec3 light_dir = normalize(light_pos.xyz - v_fragPos);
	float diffuse_impact = max(dot(norm, light_dir), 0.0);
	vec3 diff = (diffuse_impact * texture2D(diffuse, v_texcoord0)).xyz *
		light_diffuse.xyz;

	vec3 view_dir = normalize(view_position.xyz - v_fragPos);
	vec3 reflect_dir = reflect(-light_dir, norm);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), specular_shininess.w);
	vec3 specular = spec * light_specular.xyz * texture2D(specular, v_texcoord0).xyz;

	vec3 result = ambient + diff + specular;
	gl_FragColor = vec4(result, 1.0);
}

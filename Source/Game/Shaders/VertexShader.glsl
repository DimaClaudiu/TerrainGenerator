#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;
layout(location = 3) in vec3 v_color;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 ProjectilePosition;

// Output values to fragment shader
out vec3 world_position;
out vec3 world_normal;
out vec2 texcoord;

float getDistance(vec3 p, vec3 q)
{
	return sqrt(pow(p.x - q.x, 2) + pow(p.y - q.y, 2) + pow(p.z - q.z, 2));
}

void main()
{	
	texcoord = v_texture_coord;
	// compute world space vertex position and normal
	// send world position and world normal to Fragment Shader
	world_position = (Model * vec4(v_position, 1)).xyz;
    world_normal = normalize( mat3(Model) * normalize(v_normal) );
	vec3 changedPos = v_position;

	float myDistance = distance(ProjectilePosition, v_position);

//	if(myDistance < 0.3)
//		changedPos.y = v_position.y/(1.5 - 2*myDistance);

	gl_Position = Projection * View * Model * vec4(changedPos, 1.0);
}

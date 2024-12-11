#version 410

layout(location = 0) in vec3 vertex;  
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;  
layout(location = 3) in vec3 tangent;   
layout(location = 4) in vec3 binormal; 

uniform mat4 vertex_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform mat4 normal_model_to_world;

uniform sampler2D displacementMap;      
uniform float displacementScale;

uniform float keepdisplacement;  

// Outputs for fragment shader
out VS_OUT
{
	vec3 vertex;           
    vec3 normal;             
    vec2 fragtexcoord;           
}vs_out;

void main() {

    // Transform to world space
    vs_out.vertex = normalize( vec3(vertex_model_to_world * vec4(vertex, 1.0)) );
	//vs_out.normal = normalize( vec3(normal_model_to_world * vec4(normal, 1.0)) );
	vs_out.normal = normal;
	vs_out.fragtexcoord = texcoord;

	float displacement = texture(displacementMap, texcoord).r;
	vec3 displacedPosition;
	if (keepdisplacement == 1.0f)
	    displacedPosition = vertex + displacement * displacementScale * normal;
	else
	    displacedPosition = vertex - displacement * displacementScale * normal;

    // Calculate the clip-space position of the vertex
    //gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);
	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displacedPosition, 1.0);
}

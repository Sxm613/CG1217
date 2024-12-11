#version 410

layout(location = 0) in vec3 vertex;  // ����λ��
layout(location = 1) in vec3 normal;    // ����
layout(location = 2) in vec2 texcoord;  // ��������
layout(location = 3) in vec3 tangent;   // ����
layout(location = 4) in vec3 binormal; // ������

uniform mat4 vertex_model_to_world;//�������ģ�Ϳռ�ת��������ռ䡣
uniform mat4 vertex_world_to_clip;//�����������ռ�ת�����ü��ռ䡣
uniform mat4 normal_model_to_world;//�����ߴ�ģ�Ϳռ�ת��������ռ䡣
uniform float elapsed_time_s; // ʱ�䣬��

// ����ṹ��
out VS_OUT {
    vec3 vertex;            
    vec3 normal;           
    vec2 fragtexcoord;
	mat3 TBN; 
} vs_out;

void main() {
    vs_out.vertex = normalize( vec3(vertex_model_to_world * vec4(vertex, 1.0)) );
	vs_out.normal = normalize( vec3(normal_model_to_world * vec4(normal, 1.0)) );  

	vec3 T = normalize(tangent);
	vec3 B = normalize(binormal);
	vec3 N = normalize(normal);
    vs_out.TBN = mat3(T, B, N);

    vs_out.fragtexcoord = texcoord;

    // Calculate the clip-space position of the vertex
    gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(vertex, 1.0);

}

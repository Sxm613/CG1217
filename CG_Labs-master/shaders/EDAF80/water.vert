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

// ��������
const float A1 = 0.2;    // ���1
const vec2 D1 = vec2(-1.0, 0.0); // ����1
const float f1 = 0.2;    // Ƶ��1
const float p1 = 0.05;    // ��λ1
const float k1 = 5.0;    // ���1

const float A2 = 0.5;    // ���2
const vec2 D2 = vec2(-0.7, 0.7); // ����2
const float f2 = 0.4;    // Ƶ��2
const float p2 = 0.03;    // ��λ2
const float k2 = 2.0;    // ���2

// ����ṹ��
out VS_OUT {
    vec3 vertex;            
    vec3 normal;           
    vec2 fragtexcoord;
	mat3 TBN; 
    mat3 TBNwater;
	vec2 normalCoord0;
	vec2 normalCoord1;
	vec2 normalCoord2;
} vs_out;

// ���㵥�����ĸ߶�
 float wave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
 {
      float at = sin((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * 0.5 + 0.5;
      return amplitude * pow(at, sharpness);
 }

 // ����xƫ����
 float waveDerivativesX(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
 {
      float at = sin((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * 0.5 + 0.5;
      return 0.5 * sharpness * frequency * amplitude * pow(at, sharpness - 1) * cos((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * direction.x;
 }

  // ����yƫ����
 float waveDerivativesY(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
 {
      float at = sin((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * 0.5 + 0.5;
      return 0.5 * sharpness * frequency * amplitude * pow(at, sharpness - 1) * cos((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * direction.y;
 }


void main() {
    // �������߿ռ��������
	vec3 T = normalize(tangent);
	vec3 B = normalize(binormal);
	vec3 N = normalize(normal);
	vs_out.TBN = mat3(T, B, N); 

    // ��������ռ��еĶ���λ��
	vec3 displaced_vertex = vertex;

    // ����ˮ��߶�
    float H = wave(vertex.xz, D1, A1, p1, f1, k1, elapsed_time_s) +
              wave(vertex.xz, D2, A2, p2, f2, k2, elapsed_time_s);

    // λ�ƶ���� Y ����
	displaced_vertex.y += H;
	vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));

    // ���㷨��
	float dPdx = waveDerivativesX(vertex.xz, D1, A1, p1, f1, k1, elapsed_time_s) + waveDerivativesX(vertex.xz, D2, A2, p2, f2, k2, elapsed_time_s);
    float dPdz = waveDerivativesY(vertex.xz, D1, A1, p1, f1, k1, elapsed_time_s) + waveDerivativesY(vertex.xz, D2, A2, p2, f2, k2, elapsed_time_s);
    vec3 normal = vec3(-dPdx, 1.0, -dPdz);
    // �����ߴ��ݵ�Ƭ����ɫ��
    vs_out.normal = normalize(vec3(normal_model_to_world * vec4(normal, 0.0)));


	vec3 Twater = normalize(vec3(1.0, dPdx, 0.0 ));
	vec3 Bwater = normalize(vec3(0.0, dPdz, 1.0 ));
	vec3 Nwater = normalize(vec3(-dPdx, 1.0, -dPdz));
	vs_out.TBNwater = mat3(Twater, Bwater, Nwater);                // ���� TBN ����

	vec2 texScale = vec2(8, 4);
    float normalTime = mod(elapsed_time_s, 100.0);
    vec2 normalSpeed = vec2(-0.05, 0.0);
    vs_out.normalCoord0 = texcoord * texScale + normalTime * normalSpeed;
	vs_out.normalCoord1 = texcoord * texScale * 2 + normalTime * normalSpeed * 4;
	vs_out.normalCoord2 = texcoord * texScale * 4 + normalTime * normalSpeed * 8;

	    // �������ݵ�Ƭ����ɫ��
    vs_out.fragtexcoord = texcoord; // ��ֵ�� fragtexcoord

	// �������ղü��ռ�λ��
     gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displaced_vertex, 1.0);

}

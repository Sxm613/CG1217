#version 410

layout(location = 0) in vec3 vertex;  // 顶点位置
layout(location = 1) in vec3 normal;    // 法线
layout(location = 2) in vec2 texcoord;  // 纹理坐标
layout(location = 3) in vec3 tangent;   // 切线
layout(location = 4) in vec3 binormal; // 副切线

uniform mat4 vertex_model_to_world;//将顶点从模型空间转换到世界空间。
uniform mat4 vertex_world_to_clip;//将顶点从世界空间转换到裁剪空间。
uniform mat4 normal_model_to_world;//将法线从模型空间转换到世界空间。
uniform float elapsed_time_s; // 时间，秒

// 波的属性
const float A1 = 0.2;    // 振幅1
const vec2 D1 = vec2(-1.0, 0.0); // 方向1
const float f1 = 0.2;    // 频率1
const float p1 = 0.05;    // 相位1
const float k1 = 5.0;    // 锐度1

const float A2 = 0.5;    // 振幅2
const vec2 D2 = vec2(-0.7, 0.7); // 方向2
const float f2 = 0.4;    // 频率2
const float p2 = 0.03;    // 相位2
const float k2 = 2.0;    // 锐度2

// 输出结构体
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

// 计算单个波的高度
 float wave(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
 {
      float at = sin((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * 0.5 + 0.5;
      return amplitude * pow(at, sharpness);
 }

 // 计算x偏导数
 float waveDerivativesX(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
 {
      float at = sin((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * 0.5 + 0.5;
      return 0.5 * sharpness * frequency * amplitude * pow(at, sharpness - 1) * cos((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * direction.x;
 }

  // 计算y偏导数
 float waveDerivativesY(vec2 position, vec2 direction, float amplitude, float frequency, float phase, float sharpness, float time)
 {
      float at = sin((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * 0.5 + 0.5;
      return 0.5 * sharpness * frequency * amplitude * pow(at, sharpness - 1) * cos((position.x * direction.x + position.y * direction.y)* frequency + phase * time) * direction.y;
 }


void main() {
    // 计算切线空间的正交基
	vec3 T = normalize(tangent);
	vec3 B = normalize(binormal);
	vec3 N = normalize(normal);
	vs_out.TBN = mat3(T, B, N); 

    // 计算世界空间中的顶点位置
	vec3 displaced_vertex = vertex;

    // 计算水面高度
    float H = wave(vertex.xz, D1, A1, p1, f1, k1, elapsed_time_s) +
              wave(vertex.xz, D2, A2, p2, f2, k2, elapsed_time_s);

    // 位移顶点的 Y 坐标
	displaced_vertex.y += H;
	vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));

    // 计算法线
	float dPdx = waveDerivativesX(vertex.xz, D1, A1, p1, f1, k1, elapsed_time_s) + waveDerivativesX(vertex.xz, D2, A2, p2, f2, k2, elapsed_time_s);
    float dPdz = waveDerivativesY(vertex.xz, D1, A1, p1, f1, k1, elapsed_time_s) + waveDerivativesY(vertex.xz, D2, A2, p2, f2, k2, elapsed_time_s);
    vec3 normal = vec3(-dPdx, 1.0, -dPdz);
    // 将法线传递到片段着色器
    vs_out.normal = normalize(vec3(normal_model_to_world * vec4(normal, 0.0)));


	vec3 Twater = normalize(vec3(1.0, dPdx, 0.0 ));
	vec3 Bwater = normalize(vec3(0.0, dPdz, 1.0 ));
	vec3 Nwater = normalize(vec3(-dPdx, 1.0, -dPdz));
	vs_out.TBNwater = mat3(Twater, Bwater, Nwater);                // 传递 TBN 矩阵

	vec2 texScale = vec2(8, 4);
    float normalTime = mod(elapsed_time_s, 100.0);
    vec2 normalSpeed = vec2(-0.05, 0.0);
    vs_out.normalCoord0 = texcoord * texScale + normalTime * normalSpeed;
	vs_out.normalCoord1 = texcoord * texScale * 2 + normalTime * normalSpeed * 4;
	vs_out.normalCoord2 = texcoord * texScale * 4 + normalTime * normalSpeed * 8;

	    // 传递数据到片段着色器
    vs_out.fragtexcoord = texcoord; // 赋值给 fragtexcoord

	// 计算最终裁剪空间位置
     gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displaced_vertex, 1.0);

}

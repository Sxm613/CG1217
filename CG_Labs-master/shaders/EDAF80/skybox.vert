#version 410

layout (location = 0) in vec3 position; // 顶点位置

uniform mat4 vertex_world_to_clip; // 从世界空间到裁剪空间的矩阵
uniform mat4 view; // 视图矩阵

out VS_OUT {
    vec3 texCoords; // 用于片段着色器的纹理坐标
} vs_out;

void main()
{
    // 将位置传递给纹理坐标
    vs_out.texCoords = position; // 立方体坐标直接使用顶点位置
    gl_Position = vertex_world_to_clip * vec4(position, 1.0); // 应用投影矩阵
}

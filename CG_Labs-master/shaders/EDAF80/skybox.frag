#version 410

in VS_OUT {
    vec3 texCoords; // 从顶点着色器传递过来的纹理坐标
} fs_in;

uniform samplerCube skybox; // 立方体贴图

out vec4 color; // 输出颜色

void main()
{
    // 从立方体贴图中采样颜色
    color = texture(skybox, fs_in.texCoords); // 使用立方体坐标采样
}

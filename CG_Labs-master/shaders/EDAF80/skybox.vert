#version 410

layout (location = 0) in vec3 position; // ����λ��

uniform mat4 vertex_world_to_clip; // ������ռ䵽�ü��ռ�ľ���
uniform mat4 view; // ��ͼ����

out VS_OUT {
    vec3 texCoords; // ����Ƭ����ɫ������������
} vs_out;

void main()
{
    // ��λ�ô��ݸ���������
    vs_out.texCoords = position; // ����������ֱ��ʹ�ö���λ��
    gl_Position = vertex_world_to_clip * vec4(position, 1.0); // Ӧ��ͶӰ����
}

#version 410

in VS_OUT {
    vec3 texCoords; // �Ӷ�����ɫ�����ݹ�������������
} fs_in;

uniform samplerCube skybox; // ��������ͼ

out vec4 color; // �����ɫ

void main()
{
    // ����������ͼ�в�����ɫ
    color = texture(skybox, fs_in.texCoords); // ʹ���������������
}

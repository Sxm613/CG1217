#version 410

in VS_OUT {
    vec3 vertex;
	vec3 normal;  
    vec2 fragtexcoord; // ������������
    mat3 TBN; // TBN ����
} fs_in;

out vec4 fragcolor; // �����ɫ

uniform vec3 ambient_colour;
uniform float shininess_value;
uniform vec3 light_position; // ��Դλ��
uniform vec3 camera_position; // �����λ��

uniform sampler2D normalTexture; // ������ͼ
uniform sampler2D specularTexture;  // Specular texture map
uniform samplerCube skybox; // ��������ͼ


// Fresnel ЧӦ
float fresnelSchlick(float Vn, float R0) {
    return R0 + (1.0 - R0) * pow(1.0 - Vn, 5.0);
}

void main() {
    vec3 normal = normalize(fs_in.normal);

    // ������ɫ
	vec4 waterColordeep = vec4(0.0, 0.0, 0.1, 1.0);
	vec4 waterColorshallow = vec4(0.0, 0.5, 0.5, 1.0);

    vec3 lightDir = normalize(light_position - fs_in.vertex);
    vec3 viewDir = normalize(camera_position - fs_in.vertex);
	vec3 reflectDir = normalize(reflect(viewDir, normal));

    // ����������ͼ�л�ȡ������ɫ
    vec3 reflectedColor = texture(skybox, reflectDir).rgb ;
	vec4 reflection = vec4(reflectedColor, 1.0);

	float facing = 1.0 - max(dot(viewDir, normal), 0.0); 
	vec4 waterColor = mix(waterColordeep, waterColorshallow, facing);

	// Fresnel ����
    float R0 = 0.02037; // ��������ķ�����
    float fresnel = fresnelSchlick(dot(viewDir, normal), R0);

	vec4 finalreflectionColor = reflection * fresnel;

	
    // �����ɫ�ͷ���
    //fragcolor = waterColor + finalreflectionColor;
    fragcolor = reflection+vec4(0.1f, 0.1f, 0.0f, 1.0f);
	//fragcolor = vec4(normal * 0.5 + 0.5, 1.0); // ��ʾ����
	//fragcolor = finalrefractionColor;

}

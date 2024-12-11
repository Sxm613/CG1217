#version 410

in VS_OUT {
    vec3 vertex;
	vec3 normal;  
    vec2 fragtexcoord; // ������������
    mat3 TBN; // TBN ����
	mat3 TBNwater; // TBN ����
	vec2 normalCoord0;
	vec2 normalCoord1;
	vec2 normalCoord2;
} fs_in;

out vec4 fragcolor; // �����ɫ

uniform vec3 ambient_colour;
uniform float shininess_value;
uniform vec3 light_position; // ��Դλ��
uniform vec3 camera_position; // �����λ��

uniform sampler2D normalTexture; // ������ͼ
uniform samplerCube skybox; // ��������ͼ


// Fresnel 
float fresnelSchlick(float Vn, float R0) {
    return R0 + (1.0 - R0) * pow(1.0 - Vn, 5.0);
}

void main() {
    vec3 n0 = texture(normalTexture, fs_in.normalCoord0).rgb * 2.0 - 1.0;  // ת��Ϊ [-1,1] ��Χ
	vec3 n1 = texture(normalTexture, fs_in.normalCoord1).rgb * 2.0 - 1.0;
	vec3 n2 = texture(normalTexture, fs_in.normalCoord2).rgb * 2.0 - 1.0;
	vec3 nbump =normalize( n0 + n1 + n2);
    vec3 normal = normalize(fs_in.TBNwater * nbump); // ʹ�� TBN ����ת��������ռ�
	//vec3 normal = normalize(fs_in.normal);

    // ������ɫ
	vec4 waterColordeep = vec4(0.0, 0.2, 0.3, 1.0);
	vec4 waterColorshallow = vec4(0.0, 0.0, 0.02, 1.0);  // �������ɫ�������ӽ���ɫ

	float air = 1.0; // ����
    float water = 1.33; // ˮ
    float eta = air / water; // ������ˮ��������

    vec3 lightDir = normalize(light_position - fs_in.vertex);
    vec3 viewDir = normalize(camera_position - fs_in.vertex);
	vec3 reflectDir = normalize(reflect(viewDir, normal));
	// ��������
    vec3 refracted = normalize(refract(-viewDir, normal, eta));

    // ����������ͼ�л�ȡ������ɫ
    vec3 reflectedColor = texture(skybox, reflectDir).rgb ;
	vec4 reflection = vec4(reflectedColor, 1.0);
	vec3 refractedColor = texture(skybox, refracted).rgb;
	vec4 refraction = vec4(refractedColor, 1.0);

	float facing = 1.0 - max(dot(viewDir, normal), 0.0); 
	vec4 waterColor = mix(waterColordeep, waterColorshallow, facing);

	// Fresnel ����
    float R0 = 0.02037; // ��������ķ�����
    float fresnel = fresnelSchlick(dot(viewDir, normal), R0);

	vec4 finalreflectionColor = reflection * fresnel;
	vec4 finalrefractionColor = refraction * (1.0 - fresnel);
	vec4 finalColor = reflection * fresnel + refraction * (1.0 - fresnel);

    // �����ɫ�ͷ���
    fragcolor = waterColor + finalColor;
	//fragcolor = vec4(normal * 0.5 + 0.5, 1.0); // ��ʾ����
	//fragcolor = finalrefractionColor;

}

#version 410

in VS_OUT {
    vec3 vertex;
	vec3 normal;  
    vec2 fragtexcoord; // 接收纹理坐标
    mat3 TBN; // TBN 矩阵
	mat3 TBNwater; // TBN 矩阵
	vec2 normalCoord0;
	vec2 normalCoord1;
	vec2 normalCoord2;
} fs_in;

out vec4 fragcolor; // 输出颜色

uniform vec3 ambient_colour;
uniform float shininess_value;
uniform vec3 light_position; // 光源位置
uniform vec3 camera_position; // 摄像机位置

uniform sampler2D normalTexture; // 法线贴图
uniform samplerCube skybox; // 立方体贴图


// Fresnel 
float fresnelSchlick(float Vn, float R0) {
    return R0 + (1.0 - R0) * pow(1.0 - Vn, 5.0);
}

void main() {
    vec3 n0 = texture(normalTexture, fs_in.normalCoord0).rgb * 2.0 - 1.0;  // 转换为 [-1,1] 范围
	vec3 n1 = texture(normalTexture, fs_in.normalCoord1).rgb * 2.0 - 1.0;
	vec3 n2 = texture(normalTexture, fs_in.normalCoord2).rgb * 2.0 - 1.0;
	vec3 nbump =normalize( n0 + n1 + n2);
    vec3 normal = normalize(fs_in.TBNwater * nbump); // 使用 TBN 矩阵转换到世界空间
	//vec3 normal = normalize(fs_in.normal);

    // 基本颜色
	vec4 waterColordeep = vec4(0.0, 0.2, 0.3, 1.0);
	vec4 waterColorshallow = vec4(0.0, 0.0, 0.02, 1.0);  // 极深的蓝色，几乎接近黑色

	float air = 1.0; // 空气
    float water = 1.33; // 水
    float eta = air / water; // 空气到水的折射率

    vec3 lightDir = normalize(light_position - fs_in.vertex);
    vec3 viewDir = normalize(camera_position - fs_in.vertex);
	vec3 reflectDir = normalize(reflect(viewDir, normal));
	// 计算折射
    vec3 refracted = normalize(refract(-viewDir, normal, eta));

    // 从立方体贴图中获取反射颜色
    vec3 reflectedColor = texture(skybox, reflectDir).rgb ;
	vec4 reflection = vec4(reflectedColor, 1.0);
	vec3 refractedColor = texture(skybox, refracted).rgb;
	vec4 refraction = vec4(refractedColor, 1.0);

	float facing = 1.0 - max(dot(viewDir, normal), 0.0); 
	vec4 waterColor = mix(waterColordeep, waterColorshallow, facing);

	// Fresnel 因子
    float R0 = 0.02037; // 正常入射的反射率
    float fresnel = fresnelSchlick(dot(viewDir, normal), R0);

	vec4 finalreflectionColor = reflection * fresnel;
	vec4 finalrefractionColor = refraction * (1.0 - fresnel);
	vec4 finalColor = reflection * fresnel + refraction * (1.0 - fresnel);

    // 组合颜色和反射
    fragcolor = waterColor + finalColor;
	//fragcolor = vec4(normal * 0.5 + 0.5, 1.0); // 显示法线
	//fragcolor = finalrefractionColor;

}

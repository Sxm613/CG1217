#version 410

in VS_OUT {  
    vec3 vertex;             
    vec3 normal;           
    vec2 fragtexcoord;       
    mat3 TBN;                
} fs_in;

uniform vec3 light_position;         
uniform vec3 camera_position;       

uniform sampler2D normalMap;
uniform sampler2D displacementMap;

uniform sampler2D ambientlTexture;
uniform sampler2D diffuseTexture;      
uniform sampler2D specularTexture;   

uniform float displacementScale;          
uniform float ambientScale;
uniform float diffuseScale;
uniform float specularScale; 

uniform float shininess_value;      

out vec4 fragcolor;

void main() {
    vec2 fragtexcoord = fs_in.fragtexcoord;
    vec3 normalMapValue = texture(normalMap, fragtexcoord).rgb * 2.0 - 1.0;
    vec3 normal = normalize(fs_in.TBN * normalMapValue);

    vec3 lightDir = normalize(light_position - fs_in.vertex);
    vec3 viewDir = normalize(camera_position - fs_in.vertex);
    vec3 reflectDir = normalize(reflect(-lightDir, normal)); 

	vec3 ambient = ambientScale * texture(ambientlTexture, fragtexcoord).rgb;
    vec3 diffuse = diffuseScale * max(dot(normal, lightDir), 0.0) * texture(diffuseTexture, fragtexcoord).rgb;
    vec3 specular = specularScale * pow(max(dot(reflectDir, viewDir), 0.0), shininess_value) * texture(specularTexture, fragtexcoord).rgb;

    vec3 finalColor = ambient + diffuse + specular;
    fragcolor = vec4(finalColor, 1.0);
}

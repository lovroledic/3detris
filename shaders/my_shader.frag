#version 330 core

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

struct DirectionalLight {
	vec3 direction; // I'm using it to represent the direction TOWARDS the light (not FROM)

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform DirectionalLight dirLight;

out vec4 FragColor;

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);

void main()
{
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos); // Points from a fragment to the viewer

	vec3 result = calcDirLight(dirLight, normal, viewDir);

	FragColor = vec4(result, 1.0);
}

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.direction);
	vec3 reflectDir = reflect(-lightDir, normal);

	float diff = max(dot(normal, lightDir), 0.0);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));

	return ambient + diffuse + specular;
}
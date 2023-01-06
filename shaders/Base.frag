//Base lighting fragment shader
#version 450
#extension GL_ARB_separate_shader_objects:enable

/*layout(set = 0,binding = 2) uniform Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}mat;*/

struct VS_OUT
{
    vec3 FragPos;
    vec3 TangentViewPos;
    vec2 TexCoords;
    vec3 TangentFragPos;
    vec3 Normal;
};


layout(binding = 3) uniform sampler2D diffuse;
layout(binding = 4) uniform sampler2D normal;
layout(binding = 5) uniform sampler2D specular;
layout(binding = 6) uniform sampler2D emissive;
layout(binding = 7) uniform sampler2D height;
layout(binding = 8) uniform sampler2D metallic_roughness;
layout(binding = 9) uniform sampler2D ao;


layout(set = 0,binding = 1) uniform ViewPosition
{
	vec3 viewPos;
}v;

////////////////////////////////

layout(location = 0) out vec4 FragColor;
layout(location = 0) in VS_OUT vs_out;

void main()
{ 
    //vec3 viewDir   = normalize(vs_out.TangentViewPos - vs_out.TangentFragPos);
	//FragColor = texture(diffuse, vs_out.TexCoords);
	//FragColor = vec4(vs_out.FragPos,1.0);
	FragColor = vec4(0.0f,1.0f,0.0f,1.0f);
}


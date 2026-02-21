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

layout(set = 0, binding = 3) uniform sampler2D diffuse;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in VS_OUT vs_out;

void main()
{ 
    FragColor = texture(diffuse, vs_out.TexCoords);
}


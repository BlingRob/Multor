//Base lighting vertex shader
#version 450
#extension GL_ARB_separate_shader_objects:enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

struct VS_OUT
{
    vec3 FragPos;
    vec3 TangentViewPos;
    vec2 TexCoords;
    vec3 TangentFragPos;
    vec3 Normal;
};

layout(set = 0,binding = 0) uniform Transform
{
	mat4 model;
	mat4 PV;
	mat4 NormalMatrix;
} transform;

layout(location = 0) out VS_OUT vs_out;

void main()
{
    vs_out.FragPos = vec3(transform.model * vec4(position, 1.0));
    vs_out.Normal = normalize((transform.NormalMatrix * vec4(vertexNormal, 0.0)).xyz);
    vs_out.TexCoords = texCoord;
    gl_Position = transform.PV * transform.model * vec4(position, 1.0);
}

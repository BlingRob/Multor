// Directional shadow depth vertex shader
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;

layout(push_constant) uniform ShadowPush
{
    mat4 lightMVP;
} pc;

void main()
{
    gl_Position = pc.lightMVP * vec4(position, 1.0);
}

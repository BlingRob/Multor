//Base lighting vertex shader
#version 450
#extension GL_ARB_separate_shader_objects:enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 VertexNormal;
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
	mat3 NormalMatrix;
} transform;

layout(set = 0,binding = 1) uniform ViewPosition
{
	vec3 viewPos;
}v;

layout(location = 0) out VS_OUT vs_out;

vec3 vecs[8] = vec3[8]
(
	 vec3(-0.5f,   0.5f,  -0.5f),
	 vec3(0.5f,    0.5f,  -0.5f),
	 vec3(0.5f,    -0.5f,  -0.5f),
	 vec3(-0.5f,  -0.5f,  -0.5f),
	 vec3(-0.5f,  0.5f,   0.5f),
	 vec3(0.5f,   0.5f,   0.5f),
	 vec3(0.5f, -0.5f,   0.5f),
	 vec3(-0.5f,  -0.5f,  0.5f)
);

vec2 positions[3] = vec2[](
vec2(0.0, -0.5),
vec2(0.5, 0.5),
vec2(-1.0, 0.5)
);

mat4 PV = mat4(vec4(-1.707,0.986,-0.583,-0.577),vec4(1.707,0.986,-0.583,-0.577),vec4(0,-1.971,-0.583,-0.577),vec4(0,0,3.398,3.464));

void main()
{
    /*vs_out.FragPos = vec3(transform.model * vec4(position,1.0f));
	//vs_out.Normal = transform.NormalMatrix * VertexNormal;  
	
    //vec3 N = normalize(vs_out.Normal);
    //vec3 T = normalize(transform.NormalMatrix * aTangent);
    //vec3 B = normalize(transform.NormalMatrix * aBitangent);
    //T = normalize(T - dot(T, N) * N);

    //mat3 TBN = transpose(mat3(T,B,N));
    //translate coordinats in tangent space
    //vs_out.TangentViewPos     = TBN * v.viewPos;
    //vs_out.TangentFragPos     = TBN * vs_out.FragPos;*/
     
    //vs_out.TexCoords = texCoord;
	//gl_Position = transform.PV * vec4(vs_out.FragPos,1.0f);
	//gl_Position = transform.model * vec4(vecs[gl_InstanceIndex],1.0f);//transpose(PV)* 
	vec3 test = vec3(0.0f) * v.viewPos;
	gl_Position = vec4(transform.NormalMatrix * VertexNormal,1.0f);
	gl_Position = PV * transform.model * vec4(position,1.0f);
	//vs_out.FragPos = position;
	//gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	//gl_Position = transform.PV * transform.model * vec4(position,1.0f);
}

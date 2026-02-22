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

struct LightData
{
    vec4 lightVec;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 attenuation;
    ivec4 meta;
    vec4 spotDirection;
    vec4 spotPosition;
    vec4 spotAngles;
};

layout(set = 0, binding = 1) uniform Lights
{
    ivec4 counts;
    LightData lights[16];
} sceneLights;

layout(set = 0, binding = 2) uniform ViewPosition
{
    vec4 viewPos;
} cameraData;

layout(set = 0, binding = 3) uniform sampler2D diffuse;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in VS_OUT vs_out;

void main()
{ 
    vec3 baseColor = texture(diffuse, vs_out.TexCoords).rgb;
    vec3 N = normalize(vs_out.Normal);
    vec3 lighting = vec3(0.0);

    for (int i = 0; i < 16; ++i)
    {
        if (sceneLights.lights[i].meta.z == 0)
            continue;

        int lightType = sceneLights.lights[i].meta.x;
        vec3 L = vec3(0.0);
        float attenuation = 1.0;

        if (lightType == 1) // directional
        {
            L = normalize(-sceneLights.lights[i].lightVec.xyz);
        }
        else if (lightType == 2) // point
        {
            vec3 lightPos = sceneLights.lights[i].lightVec.xyz;
            vec3 toLight = lightPos - vs_out.FragPos;
            float d = length(toLight);
            if (d > 0.0001)
                L = toLight / d;
            vec3 clq = sceneLights.lights[i].attenuation.xyz;
            attenuation = 1.0 / max(clq.x + clq.y * d + clq.z * d * d, 0.0001);
        }
        else if (lightType == 3) // spot (basic support)
        {
            vec3 lightPos = sceneLights.lights[i].spotPosition.xyz;
            vec3 toLight = lightPos - vs_out.FragPos;
            float d = length(toLight);
            if (d > 0.0001)
                L = toLight / d;
            vec3 lightDir = normalize(-sceneLights.lights[i].spotDirection.xyz);
            float theta = dot(L, lightDir);
            float outerCos = cos(radians(sceneLights.lights[i].spotAngles.x));
            float innerCos = cos(radians(sceneLights.lights[i].spotAngles.y));
            float eps = max(innerCos - outerCos, 0.0001);
            float spotFactor = clamp((theta - outerCos) / eps, 0.0, 1.0);
            vec3 clq = sceneLights.lights[i].attenuation.xyz;
            attenuation = spotFactor /
                max(clq.x + clq.y * d + clq.z * d * d, 0.0001);
        }
        else
        {
            continue;
        }

        float ndotl = max(dot(N, L), 0.0);
        vec3 amb = sceneLights.lights[i].ambient.rgb;
        vec3 dif = sceneLights.lights[i].diffuse.rgb * ndotl;

        vec3 V = normalize(cameraData.viewPos.xyz - vs_out.FragPos);
        vec3 R = reflect(-L, N);
        float specPow = 16.0;
        float specTerm = pow(max(dot(V, R), 0.0), specPow);
        vec3 spec = sceneLights.lights[i].specular.rgb * specTerm;

        lighting += (amb + dif + spec) * attenuation;
    }

    lighting = max(lighting, vec3(0.05));
    FragColor = vec4(baseColor * lighting, 1.0);
}


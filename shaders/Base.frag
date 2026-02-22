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

struct DirectionalShadowEntry
{
    mat4 lightSpace;
    ivec4 meta; // x=shadowId, y=lightSlot, z=enabled
};

struct PointShadowEntry
{
    mat4 shadowMatrices[6];
    vec4 lightPosFar; // xyz = light pos, w = far plane
    ivec4 meta;       // x=shadowId, y=lightSlot, z=enabled
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

layout(set = 0, binding = 4) uniform DirectionalShadows
{
    ivec4 counts;
    DirectionalShadowEntry entries[10];
} dirShadows;

layout(set = 0, binding = 6) uniform PointShadows
{
    ivec4 counts;
    PointShadowEntry entries[5];
} pointShadows;

layout(set = 0, binding = 3) uniform sampler2D diffuse;
layout(set = 0, binding = 5) uniform sampler2DArrayShadow dirShadowMaps;
layout(set = 0, binding = 7) uniform samplerCubeArrayShadow pointShadowMaps;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in VS_OUT vs_out;

float calcDirectionalShadow(int lightSlot, vec3 normal, vec3 lightDir)
{
    for (int i = 0; i < dirShadows.counts.x && i < 10; ++i)
    {
        if (dirShadows.entries[i].meta.z == 0)
            continue;
        if (dirShadows.entries[i].meta.y != lightSlot)
            continue;

        vec4 lightClip = dirShadows.entries[i].lightSpace * vec4(vs_out.FragPos, 1.0);
        if (abs(lightClip.w) < 1e-6)
            return 1.0;

        vec3 proj = lightClip.xyz / lightClip.w;
        proj = proj * 0.5 + 0.5;

        if (proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
            return 1.0;

        float ndotl = max(dot(normal, lightDir), 0.0);
        float bias = max(0.0015 * (1.0 - ndotl), 0.00035);

        vec2 texelSize = 1.0 / vec2(textureSize(dirShadowMaps, 0).xy);
        float visibility = 0.0;
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                vec2 uv = proj.xy + vec2(x, y) * texelSize;
                visibility += texture(dirShadowMaps,
                                      vec4(uv, float(dirShadows.entries[i].meta.x),
                                           clamp(proj.z - bias, 0.0, 1.0)));
            }
        }
        return visibility / 9.0;
    }
    return 1.0;
}

int selectCubeFace(vec3 v)
{
    vec3 a = abs(v);
    if (a.x >= a.y && a.x >= a.z)
        return (v.x >= 0.0) ? 0 : 1;
    if (a.y >= a.x && a.y >= a.z)
        return (v.y >= 0.0) ? 2 : 3;
    return (v.z >= 0.0) ? 4 : 5;
}

float calcPointShadow(int lightSlot, vec3 normal, vec3 lightDir)
{
    for (int i = 0; i < pointShadows.counts.x && i < 5; ++i)
    {
        if (pointShadows.entries[i].meta.z == 0)
            continue;
        if (pointShadows.entries[i].meta.y != lightSlot)
            continue;

        vec3 lightPos = pointShadows.entries[i].lightPosFar.xyz;
        vec3 fragToLight = vs_out.FragPos - lightPos;
        float dist = length(fragToLight);
        if (dist < 1e-5)
            return 1.0;

        int face = selectCubeFace(fragToLight);
        vec4 lightClip = pointShadows.entries[i].shadowMatrices[face] *
                         vec4(vs_out.FragPos, 1.0);
        if (abs(lightClip.w) < 1e-6)
            return 1.0;

        vec3 proj = lightClip.xyz / lightClip.w;
        float compareDepth = proj.z * 0.5 + 0.5;
        float ndotl = max(dot(normal, lightDir), 0.0);
        float bias = max(0.0035 * (1.0 - ndotl), 0.001);
        float refDepth = clamp(compareDepth - bias, 0.0, 1.0);

        vec3 dir = normalize(fragToLight);
        vec3 tangent = normalize(abs(dir.y) < 0.99 ? cross(dir, vec3(0.0, 1.0, 0.0))
                                                   : cross(dir, vec3(1.0, 0.0, 0.0)));
        vec3 bitangent = normalize(cross(dir, tangent));
        float spread = 0.0025;

        float visibility = 0.0;
        visibility += texture(pointShadowMaps,
                              vec4(normalize(dir), float(pointShadows.entries[i].meta.x)),
                              refDepth);
        visibility += texture(pointShadowMaps,
                              vec4(normalize(dir + tangent * spread),
                                   float(pointShadows.entries[i].meta.x)),
                              refDepth);
        visibility += texture(pointShadowMaps,
                              vec4(normalize(dir - tangent * spread),
                                   float(pointShadows.entries[i].meta.x)),
                              refDepth);
        visibility += texture(pointShadowMaps,
                              vec4(normalize(dir + bitangent * spread),
                                   float(pointShadows.entries[i].meta.x)),
                              refDepth);
        visibility += texture(pointShadowMaps,
                              vec4(normalize(dir - bitangent * spread),
                                   float(pointShadows.entries[i].meta.x)),
                              refDepth);
        return visibility / 5.0;
    }
    return 1.0;
}

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
        float shadowFactor = 1.0;
        if (lightType == 1)
        {
            shadowFactor = calcDirectionalShadow(sceneLights.lights[i].meta.y, N, L);
        }
        else if (lightType == 2)
        {
            shadowFactor = calcPointShadow(sceneLights.lights[i].meta.y, N, L);
        }
        else if (lightType == 3)
        {
            shadowFactor = calcDirectionalShadow(sceneLights.lights[i].meta.y, N, L);
        }

        lighting += (amb + (dif + spec) * shadowFactor) * attenuation;
    }

    lighting = max(lighting, vec3(0.05));
    FragColor = vec4(baseColor * lighting, 1.0);
}


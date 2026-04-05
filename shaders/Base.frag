//Base lighting fragment shader
#version 450
#extension GL_ARB_separate_shader_objects:enable

const float PI = 3.14159265358979323846;

struct VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    vec3 Tangent;
    vec3 Bitangent;
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
    ivec4 meta;
};

struct PointShadowEntry
{
    mat4 shadowMatrices[6];
    vec4 lightPosFar;
    ivec4 meta;
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

layout(set = 0, binding = 8) uniform MaterialData
{
    vec4 baseColorFactor;
    vec4 emissiveFactor;
    vec4 pbrFactors;
    ivec4 textureFlags0;
    ivec4 textureFlags1;
    vec4 misc;
    vec4 ambient;
    vec4 diffuseColor;
    vec4 specularColor;
    vec4 phong;
} materialData;

layout(set = 0, binding = 15) uniform RenderOptions
{
    ivec4 options;
    vec4 pbrEnvironment;
    vec4 pbrEnvironment2;
} renderOptions;

layout(set = 0, binding = 3) uniform sampler2D diffuse;
layout(set = 0, binding = 5) uniform sampler2DArrayShadow dirShadowMaps;
layout(set = 0, binding = 7) uniform samplerCubeArrayShadow pointShadowMaps;
layout(set = 0, binding = 9) uniform sampler2D baseColorMap;
layout(set = 0, binding = 10) uniform sampler2D normalMap;
layout(set = 0, binding = 11) uniform sampler2D metallicMap;
layout(set = 0, binding = 12) uniform sampler2D roughnessMap;
layout(set = 0, binding = 13) uniform sampler2D aoMap;
layout(set = 0, binding = 14) uniform sampler2D emissiveMap;
layout(set = 0, binding = 16) uniform sampler2D environmentMap;
layout(set = 0, binding = 17) uniform sampler2D brdfLutMap;
layout(set = 0, binding = 18) uniform sampler2D irradianceMap;
layout(set = 0, binding = 19) uniform sampler2D prefilteredEnvironmentMap;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in VS_OUT vs_out;

float calcDirectionalShadow(int lightSlot, vec3 normal, vec3 lightDir)
{
    for (int i = 0; i < dirShadows.counts.x && i < 10; ++i)
    {
        if (dirShadows.entries[i].meta.z == 0 || dirShadows.entries[i].meta.y != lightSlot)
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
        if (pointShadows.entries[i].meta.z == 0 || pointShadows.entries[i].meta.y != lightSlot)
            continue;

        vec3 lightPos = pointShadows.entries[i].lightPosFar.xyz;
        float farPlane = max(pointShadows.entries[i].lightPosFar.w, 0.0001);
        float lightDistance = length(lightPos - vs_out.FragPos);
        float normalOffset = mix(0.02, 0.06, clamp(lightDistance / farPlane, 0.0, 1.0));
        vec3 samplePos = vs_out.FragPos + normal * normalOffset;
        vec3 fragToLight = samplePos - lightPos;
        if (length(fragToLight) < 1e-5)
            return 1.0;

        int face = selectCubeFace(fragToLight);
        vec4 lightClip = pointShadows.entries[i].shadowMatrices[face] * vec4(samplePos, 1.0);
        if (abs(lightClip.w) < 1e-6)
            return 1.0;

        vec3 proj = lightClip.xyz / lightClip.w;
        float compareDepth = proj.z * 0.5 + 0.5;
        float ndotl = max(dot(normal, lightDir), 0.0);
        float distanceBias = 0.002 * clamp(lightDistance / farPlane, 0.0, 1.0);
        float bias = max(0.01 * (1.0 - ndotl), 0.0035) + distanceBias;
        float refDepth = clamp(compareDepth - bias, 0.0, 1.0);

        vec3 dir = normalize(fragToLight);
        vec3 tangent = normalize(abs(dir.y) < 0.99 ? cross(dir, vec3(0.0, 1.0, 0.0))
                                                   : cross(dir, vec3(1.0, 0.0, 0.0)));
        vec3 bitangent = normalize(cross(dir, tangent));
        float spread = 0.004;

        float visibility = 0.0;
        visibility += texture(pointShadowMaps, vec4(normalize(dir), float(pointShadows.entries[i].meta.x)), refDepth);
        visibility += texture(pointShadowMaps, vec4(normalize(dir + tangent * spread), float(pointShadows.entries[i].meta.x)), refDepth);
        visibility += texture(pointShadowMaps, vec4(normalize(dir - tangent * spread), float(pointShadows.entries[i].meta.x)), refDepth);
        visibility += texture(pointShadowMaps, vec4(normalize(dir + bitangent * spread), float(pointShadows.entries[i].meta.x)), refDepth);
        visibility += texture(pointShadowMaps, vec4(normalize(dir - bitangent * spread), float(pointShadows.entries[i].meta.x)), refDepth);
        return visibility / 5.0;
    }
    return 1.0;
}

vec3 getNormal()
{
    vec3 N = normalize(vs_out.Normal);
    if (materialData.textureFlags0.z == 0)
        return N;

    vec3 T = normalize(vs_out.Tangent);
    vec3 B = normalize(vs_out.Bitangent);
    if (length(T) < 1e-4 || length(B) < 1e-4)
        return N;

    mat3 TBN = mat3(T, B, N);
    vec3 tangentNormal = texture(normalMap, vs_out.TexCoords).xyz * 2.0 - 1.0;
    tangentNormal.xy *= materialData.pbrFactors.z;
    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / max(denom, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float ggx2 = GeometrySchlickGGX(max(dot(N, V), 0.0), roughness);
    float ggx1 = GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec2 directionToEquirectUv(vec3 dir)
{
    dir = normalize(dir);
    float u = atan(dir.z, dir.x) / (2.0 * PI) + 0.5;
    float v = 0.5 - asin(clamp(dir.y, -1.0, 1.0)) / PI;
    return vec2(u, v);
}

vec3 samplePrefilteredEnvironment(vec3 reflectionDir, vec3 N, float roughness)
{
    vec3 R = normalize(reflectionDir);
    vec3 up = abs(R.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, R));
    vec3 bitangent = normalize(cross(R, tangent));

    float blurStrength = max(renderOptions.pbrEnvironment2.x, 0.0);
    float sampleRadius = mix(0.0,
                             0.12 * blurStrength *
                                 max(renderOptions.pbrEnvironment2.y, 0.0),
                             roughness * roughness);
    float centerWeight = mix(1.6, 0.45, roughness) *
                         max(renderOptions.pbrEnvironment2.z, 0.0);
    float ringWeight = mix(0.15, 1.0, roughness) *
                       max(renderOptions.pbrEnvironment2.w, 0.0);

    vec3 accum = texture(prefilteredEnvironmentMap, directionToEquirectUv(R)).rgb *
                 centerWeight;
    float totalWeight = centerWeight;

    if (sampleRadius > 1.0e-5)
    {
        const float angles[8] = float[8](
            0.0,
            PI * 0.25,
            PI * 0.5,
            PI * 0.75,
            PI,
            PI * 1.25,
            PI * 1.5,
            PI * 1.75);

        for (int i = 0; i < 8; ++i)
        {
            vec2 disk = vec2(cos(angles[i]), sin(angles[i]));
            vec3 sampleDir = normalize(R +
                                       tangent * (disk.x * sampleRadius) +
                                       bitangent * (disk.y * sampleRadius));
            float nd = max(dot(N, sampleDir), 0.0);
            float angularWeight = 0.85 + 0.15 * abs(disk.x);
            float weight = ringWeight * angularWeight * mix(0.35, 1.0, nd);
            accum += texture(prefilteredEnvironmentMap,
                             directionToEquirectUv(sampleDir)).rgb *
                     weight;
            totalWeight += weight;
        }
    }

    return accum / max(totalWeight, 1.0e-5);
}

vec3 approxEnvironmentSpecularFactor(vec3 N, vec3 V, vec3 F0, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    vec2 brdf = texture(brdfLutMap, vec2(NdotV, roughness)).rg;
    vec3 spec = F0 * brdf.x + brdf.y;
    spec *= renderOptions.pbrEnvironment.z;
    spec *= renderOptions.pbrEnvironment.y * renderOptions.pbrEnvironment.w;
    return spec;
}

void main()
{
    vec3 geomN = normalize(vs_out.Normal);
    vec3 N = getNormal();
    vec3 geomT = normalize(vs_out.Tangent);
    vec3 geomB = normalize(vs_out.Bitangent);
    vec3 directN = (renderOptions.options.z != 0) ? N : geomN;
    vec3 V = normalize(cameraData.viewPos.xyz - vs_out.FragPos);
    bool usePBR = materialData.textureFlags0.x == 1;

    vec4 sampledBaseColor = texture(diffuse, vs_out.TexCoords);
    if (usePBR && materialData.textureFlags0.y != 0)
        sampledBaseColor = texture(baseColorMap, vs_out.TexCoords);

    vec3 baseColor = sampledBaseColor.rgb;
    if (usePBR)
        baseColor *= materialData.baseColorFactor.rgb;

    float metallic = materialData.pbrFactors.x;
    float roughness = materialData.pbrFactors.y;
    float ao = materialData.pbrFactors.w;
    vec3 emissive = materialData.emissiveFactor.rgb;

    if (usePBR)
    {
        if (materialData.textureFlags0.w != 0 || materialData.textureFlags1.y != 0)
        {
            vec4 metallicSample = texture(metallicMap, vs_out.TexCoords);
            metallic *= (materialData.textureFlags1.y != 0) ? metallicSample.b : metallicSample.r;
        }
        if (materialData.textureFlags1.x != 0 || materialData.textureFlags1.y != 0)
        {
            vec4 roughnessSample = texture(roughnessMap, vs_out.TexCoords);
            roughness *= (materialData.textureFlags1.y != 0) ? roughnessSample.g : roughnessSample.r;
        }
        if (materialData.textureFlags1.z != 0)
            ao *= texture(aoMap, vs_out.TexCoords).r;
        if (materialData.textureFlags1.w != 0)
            emissive *= texture(emissiveMap, vs_out.TexCoords).rgb;
    }

    metallic = clamp(metallic, 0.0, 1.0);
    roughness = clamp(roughness, 0.045, 1.0);

    vec3 lighting = vec3(0.0);
    vec3 F0 = mix(vec3(0.04), baseColor, metallic);

    for (int i = 0; i < 16; ++i)
    {
        if (sceneLights.lights[i].meta.z == 0)
            continue;

        int lightType = sceneLights.lights[i].meta.x;
        vec3 L = vec3(0.0);
        float attenuation = 1.0;

        if (lightType == 1)
        {
            L = normalize(-sceneLights.lights[i].lightVec.xyz);
        }
        else if (lightType == 2)
        {
            vec3 lightPos = sceneLights.lights[i].lightVec.xyz;
            vec3 toLight = lightPos - vs_out.FragPos;
            float d = length(toLight);
            if (d > 0.0001)
                L = toLight / d;
            vec3 clq = sceneLights.lights[i].attenuation.xyz;
            attenuation = 1.0 / max(clq.x + clq.y * d + clq.z * d * d, 0.0001);
        }
        else if (lightType == 3)
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
            attenuation = spotFactor / max(clq.x + clq.y * d + clq.z * d * d, 0.0001);
        }
        else
        {
            continue;
        }

        float shadowFactor = 1.0;
        if (lightType == 1 || lightType == 3)
            shadowFactor = calcDirectionalShadow(sceneLights.lights[i].meta.y, directN, L);
        else if (lightType == 2)
            shadowFactor = calcPointShadow(sceneLights.lights[i].meta.y, directN, L);

        float NdotL = max(dot(directN, L), 0.0);
        if (NdotL <= 0.0)
        {
            lighting += sceneLights.lights[i].ambient.rgb * baseColor * attenuation;
            continue;
        }

        if (!usePBR)
        {
            vec3 amb = sceneLights.lights[i].ambient.rgb;
            vec3 dif = sceneLights.lights[i].diffuse.rgb * NdotL;
            vec3 R = reflect(-L, directN);
            float specPow = max(materialData.phong.x, 16.0);
            float specTerm = pow(max(dot(V, R), 0.0), specPow);
            vec3 spec = sceneLights.lights[i].specular.rgb * specTerm;
            lighting += (amb + (dif + spec) * shadowFactor) * attenuation;
            continue;
        }

        vec3 H = normalize(V + L);
        vec3 radiance = sceneLights.lights[i].diffuse.rgb * attenuation;
        float NDF = DistributionGGX(directN, H, roughness);
        float G = GeometrySmith(directN, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(directN, V), 0.0) * NdotL;
        vec3 specular = numerator / max(denominator, 0.0001);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
        vec3 diffuseBRDF = kD * baseColor / PI;
        vec3 ambientTerm = sceneLights.lights[i].ambient.rgb * baseColor * ao;

        lighting += ambientTerm * attenuation +
                    (diffuseBRDF + specular) * radiance * NdotL * shadowFactor;
    }

    vec3 finalColor;
    vec3 debugEnvDiffuse = vec3(0.0);
    vec3 debugEnvSpecular = vec3(0.0);
    if (!usePBR)
    {
        lighting = max(lighting, vec3(0.05));
        finalColor = baseColor * lighting;
    }
    else
    {
        vec3 envN = (renderOptions.options.y != 0) ? geomN : N;
        vec3 kSView = fresnelSchlick(max(dot(N, V), 0.0), F0);
        vec3 kDView = (vec3(1.0) - kSView) * (1.0 - metallic);
        vec3 envDiffuseColor =
            texture(irradianceMap, directionToEquirectUv(envN)).rgb;
        vec3 R = reflect(-V, envN);
        vec3 envSpecDir = normalize(mix(R, envN, roughness * roughness * 0.35));
        vec3 envSpecularColor =
            samplePrefilteredEnvironment(envSpecDir, envN, roughness);
        vec3 ambientDiffuse = kDView * baseColor * envDiffuseColor * ao *
                              renderOptions.pbrEnvironment.x;
        vec3 ambientSpecular =
            approxEnvironmentSpecularFactor(envN, V, F0, roughness) *
            envSpecularColor * ao;
        debugEnvDiffuse = ambientDiffuse;
        debugEnvSpecular = ambientSpecular;
        finalColor = lighting + emissive;
        finalColor += ambientDiffuse + ambientSpecular;
    }

    switch (renderOptions.options.x)
    {
        case 1:
            finalColor = baseColor;
            break;
        case 2:
            finalColor = N * 0.5 + 0.5;
            break;
        case 3:
            finalColor = vec3(metallic);
            break;
        case 4:
            finalColor = vec3(roughness);
            break;
        case 5:
            finalColor = vec3(ao);
            break;
        case 6:
            finalColor = emissive;
            break;
        case 7:
            finalColor = debugEnvDiffuse;
            break;
        case 8:
            finalColor = debugEnvSpecular;
            break;
        case 9:
            finalColor = geomN * 0.5 + 0.5;
            break;
        case 10:
            finalColor = geomT * 0.5 + 0.5;
            break;
        case 11:
            finalColor = geomB * 0.5 + 0.5;
            break;
        case 12:
            finalColor = abs(N - geomN);
            break;
        default:
            break;
    }

    FragColor = vec4(finalColor, sampledBaseColor.a);
}


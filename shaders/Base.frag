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
    vec4 lightRange;
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
    vec4 shadowSettings0;
    vec4 shadowSettings1;
    vec4 shadowSettings2;
    vec4 shadowSettings3;
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

#include "pbr_material.glsl"
#include "pbr_brdf.glsl"
#include "ibl_eval.glsl"
#include "shadow_eval.glsl"

void main()
{
    vec3 geomN = normalize(vs_out.Normal);
    vec3 N = getNormal();
    vec3 geomT = normalize(vs_out.Tangent);
    vec3 geomB = normalize(vs_out.Bitangent);
    vec3 directN = (renderOptions.options.z != 0) ? N : geomN;
    vec3 shadowN = geomN;
    vec3 V = normalize(cameraData.viewPos.xyz - vs_out.FragPos);
    SampledMaterial material = sampleMaterial();
    bool usePBR = material.usePBR;
    vec4 sampledBaseColor = material.sampledBaseColor;
    vec3 baseColor = material.baseColor;
    float metallic = material.metallic;
    float roughness = material.roughness;
    float ao = material.ao;
    vec3 emissive = material.emissive;

    vec3 lighting = vec3(0.0);
    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    float debugShadowFactor = 1.0;
    float debugShadowVisibilityRaw = 1.0;
    vec3 debugShadowInputDelta = abs(directN - shadowN);
    float debugShadowNdotL = 1.0;
    float debugShadowBias = 0.0;
    vec3 debugPointShadowFace = vec3(0.0);
    float debugPointShadowDistanceRatio = 0.0;
    float debugPointCompareDepth = 0.0;
    bool hasShadowDebugLight = false;

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
            shadowFactor = calcDirectionalShadow(sceneLights.lights[i].meta.y, shadowN, L);
        else if (lightType == 2)
            shadowFactor = calcPointShadow(sceneLights.lights[i].meta.y, shadowN, L);
        debugShadowVisibilityRaw = min(debugShadowVisibilityRaw, shadowFactor);
        shadowFactor = mix(1.0, shadowFactor, renderOptions.shadowSettings0.x);
        debugShadowFactor = min(debugShadowFactor, shadowFactor);

        float NdotL = max(dot(directN, L), 0.0);
        bool wantsThisLight =
            (renderOptions.options.w < 0) ||
            (sceneLights.lights[i].meta.y == renderOptions.options.w);
        if (!hasShadowDebugLight && wantsThisLight)
        {
            float shadowNdotL = max(dot(shadowN, L), 0.0);
            debugShadowNdotL = shadowNdotL;
            if (lightType == 1 || lightType == 3)
            {
                float dirBiasScale = max(renderOptions.shadowSettings0.y, 0.0);
                float directionalTerminatorNormalScale = max(renderOptions.shadowSettings2.w, 0.0);
                float directionalNormalOffsetScale = max(renderOptions.shadowSettings2.x, 0.0);
                float terminatorNormal =
                    (1.0 - shadowNdotL) * 0.02 * directionalTerminatorNormalScale;
                float normalOffset =
                    mix(0.0005, 0.003, 1.0 - shadowNdotL) * directionalNormalOffsetScale;
                debugShadowBias =
                    (max(0.0025 * (1.0 - shadowNdotL), 0.00075) * dirBiasScale) +
                    terminatorNormal + normalOffset;
            }
            else if (lightType == 2)
            {
                vec3 lightPos = sceneLights.lights[i].lightVec.xyz;
                float lightDistance = length(lightPos - vs_out.FragPos);
                float nearPlane = 0.1;
                float farPlane = 25.0;
                float invRange = 1.0 / max(farPlane - nearPlane, 1.0e-4);
                int pointShadowEntryIndex = -1;
                for (int p = 0; p < pointShadows.counts.x && p < 5; ++p)
                {
                    if (pointShadows.entries[p].meta.z != 0 &&
                        pointShadows.entries[p].meta.y == sceneLights.lights[i].meta.y)
                    {
                        nearPlane = pointShadows.entries[p].lightRange.x;
                        farPlane = max(pointShadows.entries[p].lightRange.y, nearPlane + 1.0e-4);
                        invRange = pointShadows.entries[p].lightRange.z;
                        pointShadowEntryIndex = p;
                        break;
                    }
                }
                debugPointShadowFace =
                    pointShadowFaceDebugColor(selectCubeFace(vs_out.FragPos - lightPos));
                debugPointShadowDistanceRatio =
                    clamp(lightDistance / farPlane, 0.0, 1.0);
                vec3 samplePos = vs_out.FragPos;
                vec3 fragToLight = samplePos - lightPos;
                int debugFace = selectCubeFace(fragToLight);
                if (pointShadowEntryIndex >= 0)
                {
                    vec4 lightClip =
                        pointShadows.entries[pointShadowEntryIndex]
                            .shadowMatrices[debugFace] *
                        vec4(samplePos, 1.0);
                    if (abs(lightClip.w) > 1.0e-6)
                    {
                        vec3 proj = lightClip.xyz / lightClip.w;
                        debugPointCompareDepth =
                            clamp(proj.z * 0.5 + 0.5, 0.0, 1.0);
                    }
                }
                float pointBiasScale = max(renderOptions.shadowSettings0.z, 0.0);
                float pointNormalOffsetScale = max(renderOptions.shadowSettings0.w, 0.0);
                float pointTerminatorNormalScale = max(renderOptions.shadowSettings2.y, 0.0);
                float pointTerminatorGeometryScale = max(renderOptions.shadowSettings2.z, 0.0);
                float distanceFactor =
                    clamp((lightDistance - nearPlane) * invRange, 0.0, 1.0);
                float distanceBias = mix(0.0015, 0.0045, distanceFactor);
                float depthBias = mix(0.0010, 0.0035, debugPointCompareDepth);
                float normalOffset =
                    mix(0.045, 0.11, distanceFactor) * pointNormalOffsetScale;
                float terminatorNormal =
                    (1.0 - shadowNdotL) * 0.06 * pointTerminatorNormalScale;
                float terminatorGeometry =
                    (1.0 - shadowNdotL) * 0.03 * pointTerminatorGeometryScale;
                debugShadowBias =
                    (max(0.016 * (1.0 - shadowNdotL), 0.0065) + distanceBias + depthBias) *
                        pointBiasScale +
                    normalOffset + terminatorNormal + terminatorGeometry;
                debugPointShadowDistanceRatio = distanceFactor;
            }
            hasShadowDebugLight = true;
        }
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
        case 13:
            finalColor = vec3(debugShadowFactor);
            break;
        case 14:
            finalColor = debugShadowInputDelta;
            break;
        case 15:
            finalColor = vec3(debugShadowNdotL);
            break;
        case 16:
        {
            float heat = clamp(debugShadowBias * 12.0, 0.0, 1.0);
            finalColor = mix(vec3(0.0, 0.0, 1.0),
                             vec3(1.0, 0.0, 0.0),
                             heat);
            break;
        }
        case 17:
            finalColor = vec3(debugShadowVisibilityRaw);
            break;
        case 18:
            finalColor = debugPointShadowFace;
            break;
        case 19:
            finalColor = mix(vec3(0.0, 0.2, 0.9),
                             vec3(1.0, 0.85, 0.15),
                             debugPointShadowDistanceRatio);
            break;
        case 20:
            finalColor = vec3(debugPointCompareDepth);
            break;
        default:
            break;
    }

    FragColor = vec4(finalColor, sampledBaseColor.a);
}


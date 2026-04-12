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

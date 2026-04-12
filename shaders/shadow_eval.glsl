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

        float directionalNormalOffsetScale = max(renderOptions.shadowSettings2.x, 0.0);
        float directionalTerminatorNormalScale = max(renderOptions.shadowSettings2.w, 0.0);
        float directionalTerminatorGeometryScale = max(renderOptions.shadowSettings3.x, 0.0);
        float dirBiasScale = max(renderOptions.shadowSettings0.y, 0.0);
        float ndotl = max(dot(normal, lightDir), 0.0);
        float terminatorNormal = (1.0 - ndotl) * 0.02 * directionalTerminatorNormalScale;
        float terminatorGeometry = (1.0 - ndotl) * 0.01 * directionalTerminatorGeometryScale;
        float normalOffset = mix(0.0005, 0.003, 1.0 - ndotl) * directionalNormalOffsetScale;
        vec3 samplePos = vs_out.FragPos + normal * (normalOffset + terminatorNormal) + lightDir * terminatorGeometry;
        vec4 offsetClip = dirShadows.entries[i].lightSpace * vec4(samplePos, 1.0);
        if (abs(offsetClip.w) < 1e-6)
            return 1.0;
        vec3 offsetProj = offsetClip.xyz / offsetClip.w;
        offsetProj = offsetProj * 0.5 + 0.5;
        float bias = max(0.0025 * (1.0 - ndotl), 0.00075) * dirBiasScale;
        vec2 texelSize = 1.0 / vec2(textureSize(dirShadowMaps, 0).xy);
        float directionalPcfSpreadScale = max(renderOptions.shadowSettings1.w, 0.0);
        vec2 filterScale = texelSize * directionalPcfSpreadScale;

        float visibility = 0.0;
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                vec2 uv = offsetProj.xy + vec2(x, y) * filterScale;
                visibility += texture(dirShadowMaps,
                                      vec4(uv, float(dirShadows.entries[i].meta.x),
                                           clamp(offsetProj.z - bias, 0.0, 1.0)));
            }
        }
        return max(visibility / 9.0, renderOptions.shadowSettings1.y);
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

vec3 pointShadowFaceDebugColor(int face)
{
    switch (face)
    {
        case 0: return vec3(1.0, 0.2, 0.2);
        case 1: return vec3(0.6, 0.1, 0.1);
        case 2: return vec3(0.2, 1.0, 0.2);
        case 3: return vec3(0.1, 0.6, 0.1);
        case 4: return vec3(0.2, 0.4, 1.0);
        case 5: return vec3(0.1, 0.2, 0.6);
        default: return vec3(0.0);
    }
}

int selectAxisFace(int axis, vec3 v)
{
    if (axis == 0)
        return (v.x >= 0.0) ? 0 : 1;
    if (axis == 1)
        return (v.y >= 0.0) ? 2 : 3;
    return (v.z >= 0.0) ? 4 : 5;
}

vec3 computePointShadowAxisWeights(vec3 fragToLight, out float seamFactor)
{
    vec3 a = abs(fragToLight);
    float axisSum = a.x + a.y + a.z;
    if (axisSum <= 1.0e-6)
    {
        seamFactor = 0.0;
        return vec3(1.0, 0.0, 0.0);
    }

    vec3 normalizedWeights = a / axisSum;
    float dominantWeight = max(normalizedWeights.x,
                               max(normalizedWeights.y, normalizedWeights.z));
    seamFactor = smoothstep(0.22, 0.5, 1.0 - dominantWeight);

    float blendExponent = mix(3.0, 1.35, seamFactor);
    vec3 axisWeights = pow(normalizedWeights, vec3(blendExponent));
    float weightSum = axisWeights.x + axisWeights.y + axisWeights.z;
    if (weightSum <= 1.0e-6)
    {
        seamFactor = 0.0;
        return vec3(1.0, 0.0, 0.0);
    }
    return axisWeights / weightSum;
}

float projectPointCompareDepth(PointShadowEntry entry, vec3 samplePos, int face)
{
    vec4 lightClip = entry.shadowMatrices[face] * vec4(samplePos, 1.0);
    if (abs(lightClip.w) < 1e-6)
        return -1.0;

    vec3 proj = lightClip.xyz / lightClip.w;
    float compareDepth = proj.z * 0.5 + 0.5;
    if (compareDepth <= 0.0 || compareDepth >= 1.0)
        return -1.0;
    return compareDepth;
}

float blendedPointCompareDepth(PointShadowEntry entry, vec3 samplePos, vec3 fragToLight)
{
    float seamFactor = 0.0;
    vec3 axisWeights = computePointShadowAxisWeights(fragToLight, seamFactor);
    int primaryFace = selectCubeFace(fragToLight);
    float primaryDepth = projectPointCompareDepth(entry, samplePos, primaryFace);
    if (seamFactor <= 1.0e-4)
        return primaryDepth;

    float depthX = projectPointCompareDepth(entry, samplePos,
                                            selectAxisFace(0, fragToLight));
    float depthY = projectPointCompareDepth(entry, samplePos,
                                            selectAxisFace(1, fragToLight));
    float depthZ = projectPointCompareDepth(entry, samplePos,
                                            selectAxisFace(2, fragToLight));

    float accumDepth = 0.0;
    float accumWeight = 0.0;

    if (depthX >= 0.0)
    {
        accumDepth += depthX * axisWeights.x;
        accumWeight += axisWeights.x;
    }
    if (depthY >= 0.0)
    {
        accumDepth += depthY * axisWeights.y;
        accumWeight += axisWeights.y;
    }
    if (depthZ >= 0.0)
    {
        accumDepth += depthZ * axisWeights.z;
        accumWeight += axisWeights.z;
    }

    if (accumWeight <= 1.0e-6)
        return primaryDepth;

    float blendedDepth = accumDepth / accumWeight;
    if (primaryDepth < 0.0)
        return blendedDepth;
    return mix(primaryDepth, blendedDepth, seamFactor);
}

float calcPointShadow(int lightSlot, vec3 normal, vec3 lightDir)
{
    for (int i = 0; i < pointShadows.counts.x && i < 5; ++i)
    {
        if (pointShadows.entries[i].meta.z == 0 || pointShadows.entries[i].meta.y != lightSlot)
            continue;

        vec3 lightPos = pointShadows.entries[i].lightPosFar.xyz;
        float nearPlane = pointShadows.entries[i].lightRange.x;
        float farPlane = max(pointShadows.entries[i].lightRange.y, nearPlane + 1.0e-4);
        float invRange = pointShadows.entries[i].lightRange.z;
        float lightDistance = length(lightPos - vs_out.FragPos);
        float pointNormalOffsetScale = max(renderOptions.shadowSettings0.w, 0.0);
        float pointTerminatorNormalScale = max(renderOptions.shadowSettings2.y, 0.0);
        float pointTerminatorGeometryScale = max(renderOptions.shadowSettings2.z, 0.0);
        float ndotl = max(dot(normal, lightDir), 0.0);
        float distanceRatio = clamp((lightDistance - nearPlane) * invRange, 0.0, 1.0);
        float normalOffset = mix(0.045, 0.11, distanceRatio) *
                             pointNormalOffsetScale;
        float terminatorNormal = (1.0 - ndotl) * 0.06 * pointTerminatorNormalScale;
        float terminatorGeometry = (1.0 - ndotl) * 0.03 * pointTerminatorGeometryScale;
        vec3 samplePos = vs_out.FragPos + normal * (normalOffset + terminatorNormal) + lightDir * terminatorGeometry;
        vec3 fragToLight = samplePos - lightPos;
        if (length(fragToLight) < 1e-5)
            return 1.0;

        float seamFactor = 0.0;
        computePointShadowAxisWeights(fragToLight, seamFactor);

        float compareDepth =
            blendedPointCompareDepth(pointShadows.entries[i], samplePos, fragToLight);
        if (compareDepth < 0.0)
            return 1.0;
        float pointBiasScale = max(renderOptions.shadowSettings0.z, 0.0);
        float depthCurve = smoothstep(0.1, 1.0, compareDepth);
        float slopeBias = max(0.016 * (1.0 - ndotl), 0.0065);
        float distanceBias = mix(0.0015, 0.0045, distanceRatio);
        float depthBias = mix(0.0010, 0.0035, depthCurve);
        float seamBias = mix(0.0, 0.0030, seamFactor);
        float bias =
            (slopeBias + distanceBias + depthBias + seamBias) * pointBiasScale;
        float refDepth = clamp(compareDepth - bias, 0.0, 1.0);

        vec3 dir = normalize(fragToLight);
        vec3 tangent = normalize(abs(dir.y) < 0.99 ? cross(dir, vec3(0.0, 1.0, 0.0))
                                                   : cross(dir, vec3(1.0, 0.0, 0.0)));
        vec3 bitangent = normalize(cross(dir, tangent));
        float pointPcfSpreadScale = max(renderOptions.shadowSettings1.x, 0.0);
        float spread = mix(0.0025, 0.0065, distanceRatio) *
                       mix(0.9, 1.25, seamFactor) *
                       pointPcfSpreadScale;

        float visibility = 0.0;
        float totalWeight = 0.0;
        visibility += texture(pointShadowMaps,
                              vec4(normalize(dir),
                                   float(pointShadows.entries[i].meta.x)),
                              refDepth) * mix(1.9, 1.35, seamFactor);
        totalWeight += mix(1.9, 1.35, seamFactor);

        const float angles[8] = float[8](
            0.0,
            0.78539816339,
            1.57079632679,
            2.35619449019,
            3.14159265359,
            3.92699081699,
            4.71238898038,
            5.49778714378);

        for (int s = 0; s < 8; ++s)
        {
            vec2 disk = vec2(cos(angles[s]), sin(angles[s]));
            vec3 sampleDir = normalize(dir +
                                       tangent * (disk.x * spread) +
                                       bitangent * (disk.y * spread));
            float ringWeight = mix(0.75, 1.0, seamFactor) - 0.12 * abs(disk.y);
            visibility += texture(pointShadowMaps,
                                  vec4(sampleDir,
                                       float(pointShadows.entries[i].meta.x)),
                                  refDepth) * ringWeight;
            totalWeight += ringWeight;
        }

        return max(visibility / max(totalWeight, 1.0e-5),
                   renderOptions.shadowSettings1.z);
    }
    return 1.0;
}

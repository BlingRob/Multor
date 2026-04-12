struct SampledMaterial
{
    vec4 sampledBaseColor;
    vec3 baseColor;
    float metallic;
    float roughness;
    float ao;
    vec3 emissive;
    bool usePBR;
};

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

SampledMaterial sampleMaterial()
{
    SampledMaterial material;
    material.usePBR = materialData.textureFlags0.x == 1;

    material.sampledBaseColor = texture(diffuse, vs_out.TexCoords);
    if (material.usePBR && materialData.textureFlags0.y != 0)
        material.sampledBaseColor = texture(baseColorMap, vs_out.TexCoords);

    material.baseColor = material.sampledBaseColor.rgb;
    if (material.usePBR)
        material.baseColor *= materialData.baseColorFactor.rgb;

    material.metallic = materialData.pbrFactors.x;
    material.roughness = materialData.pbrFactors.y;
    material.ao = materialData.pbrFactors.w;
    material.emissive = materialData.emissiveFactor.rgb;

    if (material.usePBR)
    {
        if (materialData.textureFlags0.w != 0 || materialData.textureFlags1.y != 0)
        {
            vec4 metallicSample = texture(metallicMap, vs_out.TexCoords);
            material.metallic *= (materialData.textureFlags1.y != 0)
                                     ? metallicSample.b
                                     : metallicSample.r;
        }
        if (materialData.textureFlags1.x != 0 || materialData.textureFlags1.y != 0)
        {
            vec4 roughnessSample = texture(roughnessMap, vs_out.TexCoords);
            material.roughness *= (materialData.textureFlags1.y != 0)
                                      ? roughnessSample.g
                                      : roughnessSample.r;
        }
        if (materialData.textureFlags1.z != 0)
            material.ao *= texture(aoMap, vs_out.TexCoords).r;
        if (materialData.textureFlags1.w != 0)
            material.emissive *= texture(emissiveMap, vs_out.TexCoords).rgb;
    }

    material.metallic = clamp(material.metallic, 0.0, 1.0);
    material.roughness = clamp(material.roughness, 0.045, 1.0);
    return material;
}

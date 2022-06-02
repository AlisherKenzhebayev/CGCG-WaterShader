Texture2D reflectionTexture;
Texture2D refractionTexture;
Texture2D normalTexture;
SamplerState SampleType;

cbuffer LightBuffer : register(b0)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float specularPower;
    float4 specularColor;
};

cbuffer WaterBuffer : register(b1)
{
    float waterTranslation;
    float reflectRefractScale;
    float2 padding2;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texUV : TEXCOORD0;
    float4 reflectionPosition : TEXCOORD1;
    float4 refractionPosition : TEXCOORD2;
    float depth : TEXCOORD3;
    float3 viewDirection : TEXCOORD4;
};

float3 reflect(float3 I, float3 N)
{
    return normalize(I - 2.0 * dot(N, I) * N);
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 SimpleSinPixelShader(PixelInputType input) : SV_TARGET
{
    float4 colorL1 = float4(15, 94, 156, 255);
    colorL1 /= 255;
    float4 colorL2 = float4(116, 204, 244, 255);
    colorL2 /= 255;
    
    float fixedDepth = 20.0f;
    float depth = clamp(abs(fixedDepth - input.depth) / fixedDepth, 0.0f, 1.0f);
    
    float2 reflectTexCoord;
    float2 refractTexCoord;
    
    float4 reflectionColor;
    float4 refractionColor;
    
    float4 envColor;
    float4 textureColor;
    
    float3 lightDir;
    float3 reflectDir;
    float lightIntensity;
    float4 color;
    float4 specular; 
    
    float4 normalMap; // Raw normal texture color
    float3 normalVec; // Sampled and float3-transformed vector
    float3 normal;
    
    // ----------------------
    // INITIALIZATIONS
    // ----------------------

    color = ambientColor;
    
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Invert the light direction for calculations.
    lightDir = -lightDirection;
    
    // ----------------------
    // CALCULATIONS
    // ----------------------
    
    // Move the position the water normal is sampled from to simulate moving water.	
    input.texUV.y += waterTranslation;

    // Calculate the projected reflection texture coordinates. (RasterTek UV remap)
    reflectTexCoord.x = clamp(input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f, 0.0f, 1.0f);
    reflectTexCoord.y = clamp(-input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f, 0.0f, 1.0f);
	
    // Calculate the projected refraction texture coordinates. (RasterTek UV remap)
    refractTexCoord.x = clamp(input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f, 0.0f, 1.0f);
    refractTexCoord.y = clamp(-input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f, 0.0f, 1.0f);
    
    normalMap = normalTexture.Sample(SampleType, input.texUV);
    normalVec = (normalMap.xyz * 2.0f) - 1.0f; //TODO: use/mix with normal map from the texture!
    normal = lerp(input.normal, normalVec, 0.0f);
    
    // Re-position the texture coordinate sampling position by the normal map value to simulate the rippling wave effect.
    reflectTexCoord = reflectTexCoord + (normal.xy * reflectRefractScale);
    refractTexCoord = refractTexCoord + (normal.xy * reflectRefractScale);
    
    // Sample the texture pixels from the textures using the updated texture coordinates.
    reflectionColor = reflectionTexture.Sample(SampleType, reflectTexCoord);
    refractionColor = refractionTexture.Sample(SampleType, refractTexCoord);
    
    // Combine the reflection and refraction results for the final color.
    // TODO: Add contribution from both to the mixed color.
    envColor = lerp(reflectionColor, refractionColor, 0.3f);
    
    // TODO: Add fresnel/schlick approximation
    textureColor = lerp(colorL1, colorL2, 0.5f);
    
    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.normal, lightDir));
    
    if (lightIntensity > 0.0f)
    {
        // Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
        color += (diffuseColor * lightIntensity);
        
        color = saturate(color);
    }

    // Calculate the specular component of the light
    reflectDir = reflect(-lightDir, normal);
    specular = pow(max(dot(input.viewDirection, reflectDir), 0.0f), specularPower) * specularColor;
        
    
    color = color * textureColor * envColor;

    color = saturate(color + specular);
    
    //color = float4(input.position.xz / 255, 1.0f, 1.0f);    
    //color = float4(input.normal, 1.0f);    
    return color;
}
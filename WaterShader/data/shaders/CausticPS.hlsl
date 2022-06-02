Texture2D sunTexture;
SamplerState SampleType;

cbuffer LightBuffer : register(b0)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float specularPower;
    float4 specularColor;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texUV : TEXCOORD0;
    float3 viewDirection : TEXCOORD4;
};

float3 reflect(float3 I, float3 N)
{
    return normalize(I - 2.0 * dot(N, I) * N);
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 CausticPixelShader(PixelInputType input) : SV_TARGET
{
    float4 colorL1 = float4(15, 94, 156, 255);
    colorL1 /= 255;
    float4 colorL2 = float4(116, 204, 244, 255);
    colorL2 /= 255;
    
    float fixedDepth = 20.0f;
    
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

    normal = input.normal;
    
    color = ambientColor;
    
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Invert the light direction for calculations.
    lightDir = -lightDirection;
    
    // ----------------------
    // CALCULATIONS
    // ----------------------
    
    //normalMap = normalTexture.Sample(SampleType, input.texUV);
    //normalVec = (normalMap.xyz * 2.0f) - 1.0f; //TODO: use/mix with normal map from the texture!
    //normal = lerp(input.normal, normalVec, 0.0f);
    
    textureColor = sunTexture.Sample(SampleType, input.texUV);
    
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
        
    
    color = color * textureColor;

    color = saturate(color + specular);
    
    //color = float4(input.position.xz / 255, 1.0f, 1.0f);    
    //color = float4(input.normal, 1.0f);    
    return color;
}
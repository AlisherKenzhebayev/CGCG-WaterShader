Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer : register(b0)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float padding1;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texUV : TEXCOORD0;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	float4 color;
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
     
    color = ambientColor;
    
    // Invert the light direction for calculations.
    lightDir = -lightDirection;
    
    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.normal, lightDir));
    
    if (lightIntensity > 0.0f)
    {
        // Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
        color += (diffuseColor * lightIntensity);
    }
    
    // saturate the final light color
    color = saturate(color);
    
    textureColor = shaderTexture.Sample(SampleType, input.texUV);
   
    color = color * textureColor;
    
    return color;
}
Texture2D shaderTexture;
SamplerState SampleType;

cbuffer LightBuffer
{
    float4 diffuseColor;
    float3 lightDirection;
    float padding;
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
float4 SimpleSinPixelShader(PixelInputType input) : SV_TARGET
{
    float4 textureColor;
    float3 lightDir;
    float lightIntensity;
    float4 color;


    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = shaderTexture.Sample(SampleType, input.texUV);
    
    // Invert the light direction for calculations.
    lightDir = -lightDirection;

    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(input.normal, lightDir));
    
    // Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
    color = saturate(diffuseColor * lightIntensity);

    // Multiply the texture pixel and the final diffuse color to get the final pixel color result.
    color = color * textureColor;

    //color = lerp(float4(0.12f, 0.4f, 0.7f, 0.6f), shaderTexture.Sample(SampleType, input.texUV), 0.5);
    //color = float4(input.normal, 1.0f);    
    return color;
}
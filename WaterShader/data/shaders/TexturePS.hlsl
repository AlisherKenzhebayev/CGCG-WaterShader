Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texUV: TEXCOORD0;
    float2 normalUV : TEXCOORD1;
};

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 TexturePixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
    
	textureColor = shaderTexture.Sample(SampleType, input.texUV);
    
    return textureColor;
}
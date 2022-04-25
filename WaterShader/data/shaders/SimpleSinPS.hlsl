Texture2D shaderTexture;
SamplerState SampleType;


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
    
    // TODO: implement lights, at some point with normal calculation
    
    textureColor = float4(0.12f, 0.4f, 0.7f, 0.6f);
    //textureColor = shaderTexture.Sample(SampleType, input.texUV);
    
    return textureColor;
}
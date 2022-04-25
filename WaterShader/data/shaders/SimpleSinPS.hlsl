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
    
    //textureColor = float4(0, 1, 0, 1);
    textureColor = shaderTexture.Sample(SampleType, input.texUV);
    
    return textureColor;
}

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer SineBuffer : register(b1)
{
    float4 commonConst;
    float4 waveHeights;
    float4 waveLengths;
    float4 waveOffset; 
    float4 waveSpeed; 
    float4 waveDirx; 
    float4 waveDiry; 
    float4 bumpSpeed;
    float4 piVector; 
    float4 sin7;
    float4 cos8;
    float4 frcFixup; 
    float4 psCommonConst; 
    float4 highlightColor;
    float4 waterColor; 
    float2 time;
};

struct VertexInputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 texUV : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texUV: TEXCOORD0;
};


PixelInputType SimpleSinVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;
    
    float bracketValue = mul(input.texUV.x, waveDirx);
    bracketValue += mul(input.texUV.y, waveDiry);
    
    bracketValue = mul(bracketValue, 2 / waveLengths);
    bracketValue += mul(waveSpeed, time);

    float sinValue = sin(bracketValue);
    float cosValue = cos(bracketValue);
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    //output.position.y += float4(10.0, 15.0, 30.0, 0.0);
    //TODO: vertex displacement
    
    if (output.position.y % 2 == 0)
    {
        output.position.y += 1000 * sinValue; // TODO: something's wrong, as the sine is 0
    }
    
    //TODO: update normals based on sine eq
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    // Normalize the normal vector.
    output.normal = normalize(output.normal);
    
    output.texUV = input.texUV;
    
    return output;
}
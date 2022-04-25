
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
    
    float4 frequency = 2 / waveLengths;
    
    // dot(Di * (x, y))
    float4 bracketValue = mul(waveDirx, input.texUV.x);
    bracketValue += mul(waveDiry, input.texUV.y);
    
    // * w
    bracketValue = mul(bracketValue, frequency);
    
    // + t*Phi
    float phase = mul(waveSpeed, frequency);
    bracketValue += mul(time.x, phase);
    
    bracketValue += waveOffset;
    
    /*
    // Take fractional component
    bracketValue.xy = frac(bracketValue);
    float2 res = frac(bracketValue.zwzw);
    bracketValue.zw = res.xyxy;
    
    bracketValue -= 0.5f;
    bracketValue *= piVector.w;
    */
    
    float4 sinValue = sin(bracketValue);
    float4 cosValue = cos(bracketValue);
    
    //TODO: vertex displacement
    //if(input.position.y % 2 == 0)
    input.position.xyz += input.normal * (dot(sinValue, waveHeights) / 2); // TODO: something's wrong
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    //TODO: update normals based on sine eq
    output.normal = frequency.xyz;
    // Normalize the normal vector.
    output.normal = normalize(output.normal);
    
    output.texUV = input.texUV;
    
    return output;
}
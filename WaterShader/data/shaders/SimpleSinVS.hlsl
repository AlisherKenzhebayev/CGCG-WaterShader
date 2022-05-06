
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
    float4 Q;
    float4 K;
    float4 bumpSpeed;
    float4 piVector;
    float4 sin7;
    float4 cos8;
    float4 frcFixup; 
    float4 psCommonConst; 
    float4 highlightColor;
    float4 waterColor; 
    float4 time;
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

float CalcQ(float Q, float freq, float ampl, float numWaves = 2)
{
    return Q / (freq * ampl * numWaves);
}

PixelInputType SimpleSinVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // w
    float4 frequency = sqrt((9.81 * piVector.w) / waveLengths);
    
    // dot(Di * (x, y)) - Taking as point of reference the UV mapping
    float4 dotX = mul(waveDirx, input.texUV.x);
    float4 dotY = mul(waveDiry, input.texUV.y);
    float4 bracketValue =   dotX + dotY;
    
    // This is ill-advised, as the values are a multiple times harder to calculate and set up + requires GPU side tesselation to look normal.
    //float4 bracketValue = mul(waveDirx, input.position.x * scale); 
    //bracketValue += mul(waveDiry, input.position.y * scale);
    
    // *= w
    bracketValue = mul(bracketValue, frequency);
    
    float4 phase = waveSpeed * frequency;
    float4 shift = phase * time.z;
    bracketValue += shift + waveOffset;
    
    float4 sinValue = sin(bracketValue);
    float4 cosValue = cos(bracketValue);
    
    // Starting off the Eq 8.1
    sinValue += 1.0;
    sinValue /= 2.0;
    sinValue = pow(sinValue, K);
    
    //TODO: vertex displacement
    //if(input.position.y % 2 == 0)
    input.position.xyz += input.normal * dot(sinValue, waveHeights) * 2;
    
    input.position.x += 1;
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    //TODO: update normals based on sine eq
    output.normal = sinValue.www; //sin(bracketValue.x).xxx;
    // Normalize the normal vector.
    //output.normal = normalize(output.normal);
    
    output.texUV = input.texUV;
    
    return output;
}
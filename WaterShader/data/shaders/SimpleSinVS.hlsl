
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
    float2 texUV : TEXCOORD0;
};

float4 CalcQ(float4 Q, float4 freq, float4 ampl)
{
    return Q / (freq * ampl);
}

PixelInputType SimpleSinVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // w
    float4 frequency = sqrt((9.81 * piVector.w) / waveLengths);
    float4 waveHeight = waveHeights;
    
    float4 x = input.position.xxxx;
    float4 y = input.position.zzzz;
    
    // dot(Di * (x, y)) - Taking as point of reference the UV mapping
    float4 dotX = waveDirx * x;
    float4 dotY = waveDiry * y;
    float4 bracketValue = dotX + dotY;
    
    /*// This is ill-advised, as the values are a multiple times harder to calculate and set up + requires GPU side tesselation to look normal.
    float4 bracketValue = mul(waveDirx, input.position.x * scale); 
    bracketValue += mul(waveDiry, input.position.y * scale);*/
    
    // *= w
    bracketValue = bracketValue * frequency;
    
    float4 phase = waveSpeed * frequency;
    float4 shift = phase * time.z;
    bracketValue += shift + waveOffset;
    
    float4 sinValue = sin(bracketValue);
    float4 cosValue = cos(bracketValue);
    
    // Starting off the Eq 8.1, mode in additive sines
    //sinValue += 1.0;
    //sinValue /= 2.0;
    //sinValue = pow(sinValue, K);
    
    // Calculating Gerstner waves
    // QiAi
    float4 k = piVector.wwww / waveLengths;

    float4 valSteepness = waveHeight * CalcQ(Q, frequency, waveHeight);
    float4 calcX = (cosValue * waveDirx);
    calcX = dot(valSteepness, calcX);
    float4 calcY = (cosValue * waveDiry);
    calcY = dot(valSteepness, calcY);
    
    
    // Vertex displacement
    float4 position = input.position;
    
    position.x += calcX;
    position.z += calcY;
    position.y = dot(sinValue, waveHeight);
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // TODO: update normals based on calculation of tangent and bitangent
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    
    // Normalize the normal vector.
    output.normal = normalize(output.normal);
    
    output.texUV = input.texUV;
    
    return output;
}
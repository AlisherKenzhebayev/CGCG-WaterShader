
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

float4 CalcQ(float4 Q, float4 freq, float4 ampl, float4 numWaves = 2)
{
    return Q / (freq * ampl);
}

PixelInputType SimpleSinVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // w
    float4 frequency = sqrt((9.81 * piVector.w) / waveLengths);
    
    float4 x = input.texUV.x;
    float4 y = input.texUV.y;
    
    // dot(Di * (x, y)) - Taking as point of reference the UV mapping
    float4 dotX = mul(waveDirx, x);
    float4 dotY = mul(waveDiry, y);
    float4 bracketValue = dotX + dotY;
    
    /*// This is ill-advised, as the values are a multiple times harder to calculate and set up + requires GPU side tesselation to look normal.
    float4 bracketValue = mul(waveDirx, input.position.x * scale); 
    bracketValue += mul(waveDiry, input.position.y * scale);*/
    
    // *= w
    bracketValue = mul(bracketValue, frequency);
    
    float4 phase = waveSpeed * frequency;
    float4 shift = phase * time.z;
    bracketValue += shift + waveOffset;
    
    float4 sinValue = sin(bracketValue);
    float4 cosValue = cos(bracketValue);
    
    // Starting off the Eq 8.1, mode in additive sines
    sinValue += 1.0;
    sinValue /= 2.0;
    sinValue = pow(sinValue, K);
    
    // Calculating Gerstner waves
    float4 valSteepness = mul(waveHeights, CalcQ(Q, frequency, waveHeights));
    float4 calcX = mul(dotX, cosValue);
    calcX = dot(calcX, valSteepness);
    calcX *= cosValue;
    float4 calcY = mul(dotY, cosValue);
    calcY = dot(calcY, valSteepness);
    calcY *= cosValue;
    
    // Vertex displacement
    input.position.x += calcX;
    input.position.z += calcY;
    input.position.y = dot(sinValue, waveHeights);
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    //TODO: update normals based on sine eq
    output.normal = sinValue.xxx; //sin(bracketValue.x).xxx;
    // Normalize the normal vector.
    //output.normal = normalize(output.normal);
    
    output.texUV = input.texUV;
    
    return output;
}
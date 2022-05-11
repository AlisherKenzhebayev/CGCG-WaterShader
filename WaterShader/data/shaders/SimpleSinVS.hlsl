
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer ReflectionBuffer : register(b1)
{
    matrix reflectionMatrix;
};

cbuffer SineBuffer : register(b2)
{
    float4 commonConst;
    float4 waveHeights;
    float4 waveLengths;
    float4 waveSpeed;
    float4 waveDirx;
    float4 waveDiry;
    float4 Q;
    float4 K;
    float4 bumpSpeed;
    float4 piVector;
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
    float4 reflectionPosition : TEXCOORD1;
    float4 refractionPosition : TEXCOORD2;
    float depth : TEXCOORD3;
};

float4 CalcQ(float4 Q, float4 freq, float4 ampl)
{
    return Q / (freq * ampl);
}

PixelInputType SimpleSinVertexShader(VertexInputType input)
{
    PixelInputType output;
    matrix reflectProjectWorld;
    matrix viewProjectWorld;
    
    input.position.w = 1.0f;
    
    // w
    float4 frequency = sqrt((9.81 * piVector.w) / waveLengths);
    float4 waveHeight = waveHeights;
    
    float4 x = input.position.x;
    float4 y = input.position.z;
    
    // dot(Di * (x, y)) - Taking as point of reference the UV mapping
    float4 dotX = waveDirx * x;
    float4 dotY = waveDiry * y;
    float4 bracketValue = dotX + dotY;
    
    // *= w
    bracketValue = bracketValue * frequency;
    
    float4 phase = waveSpeed * frequency;
    float4 shift = phase * time.z;
    bracketValue += shift;
    
    float4 sinValue = sin(bracketValue);
    float4 cosValue = cos(bracketValue);
    
    // Calculating Gerstner waves
    // QiAi
    float4 k = piVector.wwww / waveLengths;

    float4 Qi = CalcQ(Q, frequency, waveHeight);
    
    float4 valSteepness = waveHeight * Qi;
    float4 calcX = (cosValue * waveDirx);
    calcX = dot(valSteepness, calcX);
    float4 calcY = (cosValue * waveDiry);
    calcY = dot(valSteepness, calcY);
    
    
    // Vertex displacement
    float4 position = input.position;
    
    position.x = position.x + calcX;
    position.z = position.z + calcY;
    position.y = dot(sinValue, waveHeight);
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // Update normals based on calculation of tangent and bitangent
    float4 dotDP = waveDirx * position.x;
    dotDP += waveDiry * position.z;
    //dotDP += position.y;
    
    float4 WA = frequency * waveHeight;
    float4 S = sin(frequency * dotDP + shift);
    float4 C = cos(frequency * dotDP + shift);
    
    float3 binormal = float3(
        1 - dot(Qi, pow(waveDirx, 2) * WA * S), 
        -dot(Qi, waveDirx * waveDiry * WA * S),
        dot(waveDirx, WA * C));
    float3 tangent = float3(
        -dot(Qi, waveDirx * waveDiry * WA * S),
        1 - dot(Qi, pow(waveDiry, 2) * WA * S),
        dot(waveDiry, WA * C));
    float3 normal = float3(
        -dot(waveDiry, WA * C),
        1 - dot(Qi, WA * S),
        -dot(waveDirx, WA * C));
    
    output.normal = normal; 
    output.normal = mul(output.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);
    output.texUV = input.texUV;
    output.depth = input.position.y;
    
    // Calculate reflections and refractions
    // Create the reflection projection world matrix.
    reflectProjectWorld = mul(reflectionMatrix, projectionMatrix);
    reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

    // Calculate the input position against the reflectProjectWorld matrix.
    output.reflectionPosition = mul(input.position, reflectProjectWorld);
    
    // Create the view projection world matrix for refraction.
    viewProjectWorld = mul(viewMatrix, projectionMatrix);
    viewProjectWorld = mul(worldMatrix, viewProjectWorld);
   
    // Calculate the input position against the viewProjectWorld matrix.
    output.refractionPosition = mul(input.position, viewProjectWorld);
    
    return output;
}
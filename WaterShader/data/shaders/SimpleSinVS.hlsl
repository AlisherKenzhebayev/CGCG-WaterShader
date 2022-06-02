
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

cbuffer CameraBuffer : register(b3)
{
    float3 cameraPosition;
    float padding3;
}

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
    float3 viewDirection : TEXCOORD4;
};

float4 CalcQ(float4 Q, float4 freq, float4 ampl)
{
    return Q / (freq * ampl);
}

// Bothered from https://gamedev.net/forums/topic/687501-how-to-compute-normal-vectors-for-gerstner-waves/5337678/
float3 GerstnerWaveTessendorf(float aTime, float waveLength, float speed, float amplitude, float steepness, float2 direction, in float3 position)
{
  //  float L = waveLength; // wave crest to crest length in metres
 //   float A = amplitude; // amplitude - wave height (crest to trough)
    float k = 2.0 * 3.1416 / waveLength; // wave length
    float kA = k * amplitude;
    float2 D = normalize(direction); // normalized direction
    float2 K = D * k; // wave vector and magnitude (direction)
    

    // peak/crest steepness high means steeper, but too much 
    // can cause the wave to become inside out at the top
  //  float Q = steepness; //max(steepness, 0.1); 

    // Original formula, however is more difficult to control speed
    //float w = sqrt(9.82*k); // frequency (speed)
    //float wt = w*Time;
    
    float S = speed * 0.5; // Speed 1 =~ 2m/s so halve first
    float w = S * k; // Phase/frequency
    float wT = w * aTime;

    // Unoptimized:
    // float2 xz = position.xz - K/k*Q*A*sin(dot(K,position.xz)- wT);
    // float y = A*cos(dot(K,position.xz)- wT);

    // Calculate once instead of 4 times
    float KPwT = dot(K, position.xz) - wT;
    float S0 = sin(KPwT);
    float C0 = cos(KPwT);

    // Calculate the vertex offset along the X and Z axes
    float2 xz = position.xz - D * steepness * amplitude * S0;
    // Calculate the vertex offset along the Y (up/down) axis
    float y = amplitude * C0;

    // Calculate the tangent/bitangent/normal
    // Bitangent
    float3 B = float3(
        1 - (steepness * D.x * D.x * kA * C0),
        D.x * kA * S0,
        -(steepness * D.x * D.y * kA * C0));
    // Tangent
    float3 T = float3(
        -(steepness * D.x * D.y * kA * C0),
        D.y * kA * S0,
        1 - (steepness * D.y * D.y * kA * C0)
        );

    B = normalize(B);
    T = normalize(T);
    float3 N = cross(T, B);

    return N;
    
    //// Append the results
    //result.xz += xz;
    //result.y += y;
    //normal += N;
 
    //tangent += T;
    //bitangent += B;
}

PixelInputType SimpleSinVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition; 
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
    position.y = dot(sinValue, waveHeight);
    position.z = position.z + calcY;
    
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
        dot(waveDirx, WA * C),
        -dot(Qi, waveDirx * waveDiry * WA * S));
    float3 tangent = float3(
        -dot(Qi, waveDirx * waveDiry * WA * S),
        dot(waveDiry, WA * C),
        1 - dot(Qi, pow(waveDiry, 2) * WA * S));
    float3 normal = float3(
        -dot(waveDirx, WA * C),
        1 - dot(Qi, WA * S),
        -dot(waveDiry, WA * C));
    
    binormal = normalize(binormal);
    tangent = normalize(tangent);
    
    float3 normalCalc = cross(tangent, binormal);
    
    // Switch y and z..
    output.normal = normalCalc; //float3(normalCalc.x, -normalCalc.y, normalCalc.z);
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
    
    // Calculating view direction from the world position and camera information
    worldPosition = mul(input.position, worldMatrix);
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
    output.viewDirection = normalize(output.viewDirection);
    
    // Recompute normals again
    
    //float3 normalOut = float3(0, 0, 0);
    //float3 tangentOut = float3(0, 0, 0);
    //float3 bitangentOut = float3(0, 0, 0);
    
    //normalOut += GerstnerWaveTessendorf(time.z, waveLengths.x, waveSpeed.x, waveHeight.x, Qi.x, 
    //float2(waveDirx.x, waveDiry.x), input.position.xyz);
    //normalOut += GerstnerWaveTessendorf(time.z, waveLengths.y, waveSpeed.y, waveHeight.y, Qi.y,
    //float2(waveDirx.y, waveDiry.y), input.position.xyz);
    //normalOut += GerstnerWaveTessendorf(time.z, waveLengths.z, waveSpeed.z, waveHeight.z, Qi.z,
    //float2(waveDirx.z, waveDiry.z), input.position.xyz);
    //normalOut += GerstnerWaveTessendorf(time.z, waveLengths.w, waveSpeed.w, waveHeight.w, Qi.w,
    //float2(waveDirx.w, waveDiry.w), input.position.xyz);
    
    //output.normal = normalOut;
    //output.normal = mul(output.normal, (float3x3) worldMatrix);
    //output.normal = normalize(output.normal);
    
    return output;
}
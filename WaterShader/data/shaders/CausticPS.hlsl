Texture2D shaderTexture;
Texture2D sunTexture;
SamplerState SampleType;

cbuffer LightBuffer : register(b0)
{
    float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
    float specularPower;
    float4 specularColor;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 worldPosition : TEXCOORD3;
    float3 normal : NORMAL;
    float2 texUV : TEXCOORD0;
    float3 viewDirection : TEXCOORD1;
    float time : TEXCOORD2;
};

float3 reflect(float3 I, float3 N)
{
    return normalize(I - 2.0 * dot(N, I) * N);
}

const float kRefractionAir = 1.0; // Real world: 1.000293
const float kRefractionWater = 1.333;

float3 line_plane_intercept(float3 lineP, float3 lineN, float3 planeN, float planeD)
{
    // Unoptimized
    float distance = (planeD - dot(planeN, lineP)) / dot(lineN, planeN);
    // Optimized (assumes planeN always points up)
    //float distance = (planeD - lineP.y) / lineN.y;
    return lineP + lineN * distance;
}

#define VTXSIZE 0.1f    // Amplitude
#define WAVESIZE 5.0f   // Frequency
#define FACTOR 1.0f
#define SPEED 1.0f
#define OCTAVES 2
float2 gradwave(float x, float y, float timer)
{
    float dZx = 0.0f;
    float dZy = 0.0f;
    float octaves = OCTAVES;
    float factor = FACTOR;
    float d = sqrt(x * x + y * y);

    do
    {
        dZx += d * sin(timer * SPEED + (1 / factor) * x * y * WAVESIZE) * y * WAVESIZE;
        -factor * cos(timer * SPEED + (1 / factor) * x * y * WAVESIZE) * x / d;
        dZy += d * sin(timer * SPEED + (1 / factor) * x * y * WAVESIZE) * x * WAVESIZE;
        -factor * cos(timer * SPEED + (1 / factor) * x * y * WAVESIZE) * y / d;
        factor = factor / 2;
        octaves--;
    } while (octaves > 0);

    return float2(2 * VTXSIZE * dZx, 2 * VTXSIZE * dZy);
}

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 CausticPixelShader(PixelInputType input) : SV_TARGET
{    
    float kAir2Water = kRefractionAir / kRefractionWater;
    float kWater2Air = kRefractionWater / kRefractionAir;
    
    float fixedDepth = 20.0f;
    
    float4 textureColor;
    
    float3 lightDir;
    float3 reflectDir;
    float lightIntensity;
    float4 color;
    float4 specular;
    
    float4 normalMap; // Raw normal texture color
    float3 normalVec; // Sampled and float3-transformed vector
    float3 normal;
    
    // ----------------------
    // INITIALIZATIONS
    // ----------------------

    //input.position.y += fixedDepth;
    
    
    
    normal = input.normal;
    
    color = ambientColor;
    
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Invert the light direction for calculations.
    lightDir = -lightDirection;
    
    // ----------------------
    // CALCULATIONS
    // ----------------------
    
    float3 vRefract = refract(lightDir, normal, kAir2Water);
    // Calculate the distance along the Refraction ray from the ocean surface
    // to the interception point on the ocean floor.
    float distance = (fixedDepth - input.position.y) / vRefract.y;
    
    textureColor = shaderTexture.Sample(SampleType, input.texUV);
    
    // Calculate the amount of light on this pixel.
    lightIntensity = saturate(dot(float3(0,1,0), lightDir));
    
    if (lightIntensity > 0.0f)
    {
        // Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
        color += (diffuseColor * lightIntensity);
        
        color = saturate(color);
    }
    
    float3 intercept = line_plane_intercept(input.worldPosition.xyz, float3(normal.x, saturate(input.worldPosition.w), normal.z), float3(0, 1, 0), -0.8);
    
    // Calculate the caustics
    float4 caustics = sunTexture.Sample(SampleType, intercept.xy * 0.4);
    
    // Calculate the specular component of the light
    reflectDir = reflect(-lightDir, normal);
    specular = pow(max(dot(input.viewDirection, reflectDir), 0.0f), specularPower) * specularColor;
        
    caustics.a = 1.0f;
    
    color = textureColor;
    color += caustics;
    
    color = saturate(color);
    
    //color = float4(input.position.xz / 255, 1.0f, 1.0f);    
    //color = float4(input.normal.xz, 1.0f, 1.0f);    
    return color;
}
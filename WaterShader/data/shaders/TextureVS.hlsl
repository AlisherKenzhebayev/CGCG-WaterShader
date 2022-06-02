
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
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


PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.position += float4(0.0, 0.0, 0.0, 0.0);
    //TODO: vertex displacement
    
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    // Normalize the normal vector.
    output.normal = normalize(output.normal);
    
    output.texUV = input.texUV;
    
    return output;
}
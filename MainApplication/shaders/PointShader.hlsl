struct VSInputVertex
{
    //Vertex attributes
    [[vk::location(0)]] float4 position : POSITION; // Vertex position

    //Instance attributes
    [[vk::location(1)]] float4x4 model : TEXCOORD1; //4 locations for 4 * float4
    [[vk::location(5)]] float4 color : COLOR0;
};

//Vertex shader output to fragment shader input
struct VSOutput
{
    [[vk::location(0)]] float4 position : SV_POSITION;
    [[vk::location(1)]] float4 color : COLOR0;
};

// Uniform buffer (constant buffer)
cbuffer GlobalInfo : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float4 cameraPosition;
    uint4 lightCount;
}

struct LightInfo
{
    float4 lightPosition;
    float4 lightColor;

    float4 lightAmbient;
    float4 lightDiffuse;
    float4 lightSpecular;

    float4 maxLightDistance;
};

StructuredBuffer<LightInfo> lights : register(t1);

Texture2D textures[] : register(t2);
SamplerState textureSamplers[] : register(s2);

float3 GetScale(float4x4 model)
{
    return float3(
        length(model[0].xyz),
        length(model[1].xyz),
        length(model[2].xyz)
    );
}

VSOutput VSMain(VSInputVertex vertexInput)
{
    VSOutput output;

    float4 worldPos;
    worldPos = mul(vertexInput.model, float4(0.0, 0.0, 0.0, 1.0));

    float4 viewPos = mul(view, worldPos);
    viewPos += float4(vertexInput.position.xy * GetScale(vertexInput.model).xy, 0.0, 0.0);

    float4 clipPos = mul(projection, viewPos);

    output.position = clipPos;
    output.color = vertexInput.color;

    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return float4(input.color.xyz, 1.0f);
}
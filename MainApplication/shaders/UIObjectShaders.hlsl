struct UIVSInputVertex
{
    //Vertex attributes
    [[vk::location(0)]] float4 position : POSITION; // Vertex position
    [[vk::location(1)]] float4 texCoord : TEXCOORD0; // Vertex texture coordinates
    
    //Instance attributes
    [[vk::location(2)]] float4 objectPosition : TEXCOORD1;
    [[vk::location(3)]] float4 scale : TEXCOORD2;
    [[vk::location(4)]] float4 color : COLOR3;
    
    [[vk::location(5)]] float4 textureOffset : TEXCOORD8;
    [[vk::location(6)]] float4 characterTextureSizeAndOffset : TEXCOORD9;

    [[vk::location(7)]] uint4 displayProperties : TEXCOORD10;
};

//Vertex shader output to fragment shader input
struct VSOutput
{
    [[vk::location(0)]] float4 position : SV_POSITION;
    [[vk::location(1)]] float4 texCoord : TEXCOORD0;
    
    [[vk::location(2)]] float4 color : COLOR2;

    [[vk::location(3)]] uint4 displayProperties : TEXCOORD3;
};

// Uniform buffer (constant buffer)
cbuffer UIGlobalInfo : register(b0)
{
    uint4 screenSize;
}

Texture2D textures[] : register(t1);
SamplerState textureSamplers[] : register(s1);

VSOutput VSMain(UIVSInputVertex vertexInput)
{
    uint isTextCharacter = vertexInput.displayProperties.z;

    VSOutput output;
    
    float2 uiPos = vertexInput.position.xy;
    
    if (screenSize.x > screenSize.y)
    {
        float ratio = (float) screenSize.y / (float) screenSize.x;
        uiPos.x = uiPos.x * ratio;
    }
    else if (screenSize.y > screenSize.x)
    {
        float ratio = (float) screenSize.x / (float) screenSize.y;
        uiPos.y = uiPos.y * ratio;
    }
    
    uiPos = uiPos * vertexInput.scale.xy;
    
    if (isTextCharacter == 1)
    {
        uiPos = uiPos - vertexInput.characterTextureSizeAndOffset.zw;
    }
    
    uiPos = uiPos + vertexInput.objectPosition.xy;
    
    float3 clipPos;
    clipPos.x = uiPos.x;
    clipPos.y = 1 - ((uiPos.y + 1.0));
    clipPos.z = vertexInput.objectPosition.z;
    output.position = float4(clipPos, 1.0);
    
    if (isTextCharacter == 0)
    {
        output.texCoord = vertexInput.texCoord;
    }
    else
    {
        float2 texCoord = vertexInput.textureOffset.xy;
        texCoord = texCoord + (vertexInput.texCoord.xy * vertexInput.characterTextureSizeAndOffset.xy);
        output.texCoord = float4(texCoord, 0.0, 0.0);
    }
    
    output.color = vertexInput.color;
    output.displayProperties = vertexInput.displayProperties;
    
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    uint textured = input.displayProperties.x;
    uint textureIndex = input.displayProperties.y;

    float4 texColor = float4(1.0, 1.0, 1.0, 1.0);
    
    if (textured == 1)
    {
        texColor = textures[NonUniformResourceIndex(textureIndex)].Sample(textureSamplers[textureIndex], input.texCoord.xy);
    }
    
    return input.color * texColor;
}
struct VSInputVertex
{
    //Vertex attributes
    [[vk::location(0)]] float4 position : POSITION; // Vertex position
    [[vk::location(1)]] float4 normal : NORMAL; // Vertex normal
    [[vk::location(2)]] float4 texCoord : TEXCOORD0; // Vertex texture coordinates
    
    //Instance attributes
    [[vk::location(3)]] float4x4 model : TEXCOORD1; //4 locations for 4 * float4
    [[vk::location(7)]] float4x4 modelMatrixInverted : TEXCOORD2; //4 locations for 4 * float4
    
    [[vk::location(11)]] float4 scale : TEXCOORD8;
    
    [[vk::location(12)]] float4 ambient : COLOR3;
    [[vk::location(13)]] float4 diffuse : COLOR4;
    [[vk::location(14)]] float4 specular : COLOR5;

    [[vk::location(15)]] float4 opacityAndShininess : COLOR6;

    [[vk::location(16)]] uint4 displayProperties : TEXCOORD7;
};

//Vertex shader output to fragment shader input
struct VSOutput
{
    [[vk::location(0)]] float4 position : SV_POSITION;
    [[vk::location(1)]] float4 worldPosition : TEXCOORD1;
    [[vk::location(2)]] float4 texCoord : TEXCOORD0;
    
    [[vk::location(3)]] float4 normal : NORMAL2;
    [[vk::location(4)]] float4 ambient : COLOR3;
    [[vk::location(5)]] float4 diffuse : COLOR4;
    [[vk::location(6)]] float4 specular : COLOR5;
    [[vk::location(7)]] float4 opacityAndShininess : COLOR6;
    [[vk::location(8)]] uint4 displayProperties : TEXCOORD7;
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

VSOutput VSMain(VSInputVertex vertexInput)
{
    uint lit = vertexInput.displayProperties.x;
    uint textured = vertexInput.displayProperties.y;
    uint textureIndex = vertexInput.displayProperties.z;
    uint billboarded = vertexInput.displayProperties.w;

    VSOutput output;
    
    float4 worldPos;
    if (billboarded > 0)
    {
        worldPos = mul(vertexInput.model, float4(0.0, 0.0, 0.0, 1.0));
    }
    else
    {
        worldPos = mul(vertexInput.model, float4(vertexInput.position.xyz, 1.0));
    }
    
    float4 viewPos = mul(view, worldPos);
    
    if (billboarded > 0)
    {
        viewPos += float4(vertexInput.position.xy * vertexInput.scale.xy, 0.0, 0.0);
    }
    
    float4 clipPos = mul(projection, viewPos);
    
    output.position = clipPos;
    output.worldPosition = worldPos;
    
    float3x3 normalMatrix = (float3x3)transpose(vertexInput.modelMatrixInverted);
    
    output.normal = float4(mul(normalMatrix, vertexInput.normal.xyz), 1.0);
    output.ambient = vertexInput.ambient;
    output.diffuse = vertexInput.diffuse;
    output.specular = vertexInput.specular;
    output.opacityAndShininess = vertexInput.opacityAndShininess;
    output.displayProperties = vertexInput.displayProperties;
    output.texCoord = vertexInput.texCoord;
    
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    float opacity = input.opacityAndShininess.x;
    float shininess = input.opacityAndShininess.y;

    uint lit = input.displayProperties.x;
    uint textured = input.displayProperties.y;
    uint textureIndex = input.displayProperties.z;

    float4 texColor = float4(1.0, 1.0, 1.0, 1.0);
    
    if (textured == 1)
    {
        texColor = textures[NonUniformResourceIndex(textureIndex)].Sample(textureSamplers[textureIndex], input.texCoord.xy);
    }
    
    if (lit == 0)
    {
        return float4(input.diffuse.xyz, opacity) * texColor;
    }
    
    float3 objectDiffuse = texColor.xyz * input.diffuse.xyz;
    float3 objectAmbient = texColor.xyz * input.ambient.xyz;
    
    float3 result = float3(0, 0, 0);
    
    for (uint i = 0; i < lightCount.x; i++)
    {
        // ambient
        float3 ambient = lights[i].lightAmbient.xyz * objectAmbient;

        // diffuse 
        float3 norm = normalize(input.normal.xyz);
        float3 lightDir = normalize(lights[i].lightPosition.xyz - input.worldPosition.xyz);
        float diff = max(dot(norm, lightDir), 0.0);
        float3 diffuse = lights[i].lightDiffuse.xyz * (diff * objectDiffuse);

        // specular
        float3 viewDir = normalize(cameraPosition.xyz - input.worldPosition.xyz);
        float3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        float3 specular = lights[i].lightSpecular.xyz * (spec * input.specular.xyz);
     
        float distance = length(lights[i].lightPosition.xyz - input.worldPosition.xyz);
        float lerpT = distance / lights[i].maxLightDistance.x;
    
        lerpT = min(lerpT, 1.0);
    
        float3 currentResult = ambient + diffuse + specular;
        currentResult = currentResult * (1.0 - lerpT);

        result += currentResult;
    }
    
    return float4(result, opacity);
}
#version 450

layout(binding = 0) uniform GlobalInfo {
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;

    vec3 lightPosition;
    vec3 lightColor;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} globalInfo;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 inNormal;

layout(location = 2) in vec3 inAmbient;
layout(location = 3) in vec3 inDiffuse;
layout(location = 4) in vec3 inSpecular;
layout(location = 5) in float inShininess;
layout(location = 6) flat in uint inLit;

layout(location = 0) out vec4 outColor;

void main() {
    if(inLit == 0)
    {
        outColor = vec4(inDiffuse, 1.0);
    } else {
        // ambient
        vec3 ambient  = globalInfo.ambient * inAmbient;

        // diffuse 
        vec3 norm = normalize(inNormal);
        vec3 lightDir = normalize(globalInfo.lightPosition - fragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse  = globalInfo.diffuse * (diff * inDiffuse);

        // specular
        vec3 viewDir = normalize(globalInfo.cameraPosition - fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), inShininess);
        vec3 specular = globalInfo.specular * (spec * inSpecular);   

        vec3 result = ambient + diffuse + specular;
        outColor = vec4(result, 1.0);
    }
}
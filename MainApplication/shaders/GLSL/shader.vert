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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 2) in vec4 inModelRow0;
layout(location = 3) in vec4 inModelRow1;
layout(location = 4) in vec4 inModelRow2;
layout(location = 5) in vec4 inModelRow3;

layout(location = 6) in vec3 inAmbient;
layout(location = 7) in vec3 inDiffuse;
layout(location = 8) in vec3 inSpecular;
layout(location = 9) in float inShininess;
layout(location = 10) in uint inLit;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 outNormal;

layout(location = 2) out vec3 outAmbient;
layout(location = 3) out vec3 outDiffuse;
layout(location = 4) out vec3 outSpecular;
layout(location = 5) out float outShininess;
layout(location = 6) out uint outLit;

void main() {
    mat4 model = mat4(inModelRow0, inModelRow1, inModelRow2, inModelRow3);

    gl_Position = globalInfo.proj * globalInfo.view * model * vec4(inPosition, 1.0);
    fragPos = vec3(model * vec4(inPosition, 1.0));

    outNormal = mat3(transpose(inverse(model))) * inNormal;

    outAmbient = inAmbient;
    outDiffuse = inDiffuse;
    outSpecular = inSpecular;
    outShininess = inShininess;
    outLit = inLit;
}
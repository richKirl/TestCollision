#version 460 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out vec4 FragPosLightSpace;
out vec3 FragPos;
out vec3 Normal;
out vec3 fragPosition;     // позиция фрагмента
out vec3 sceneColor;       // исходный цвет сцены
void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    fragPosition = FragPos;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    sceneColor=vec3(0.5,0.7,0.15);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
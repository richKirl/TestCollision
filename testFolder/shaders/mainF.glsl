#version 460 core
out vec4 FragColor;

in vec3 fragPosition;     // позиция фрагмента
in vec3 sceneColor;       // исходный цвет сцены

uniform vec3 fogColor;
uniform float fogDensity;
uniform vec3 cameraPos;
in vec4 FragPosLightSpace;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform sampler2D shadowMap;

// Расчет тени
float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // преобразование в диапазон 0..1

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.001;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // Можно добавить мягкое затенение (PCF) для более гладких теней
    if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}


void main() {
    // // Амбиентное
    // vec3 ambient = 0.3 * objectColor;
    // // Диффузное
    // vec3 norm = normalize(Normal);
    // vec3 lightDir = normalize(lightPos - FragPos);
    // float diff = max(dot(norm, lightDir), 0.0);

    // float shadow = ShadowCalculation(FragPosLightSpace);

    // vec3 diffuse = (1.0 - shadow) * diff * objectColor * 2.0;
    // // Блик
    // vec3 viewDir = normalize(viewPos - FragPos);
    // vec3 reflectDir = reflect(-lightDir, norm);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // vec3 specular = vec3(0.5) * spec;

    // vec3 result = ambient + diffuse + specular;

    // Основной расчет освещения (можете оставить ваш код)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 ambient = 0.1 * objectColor;
    //float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 diffuse = diff * objectColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = vec3(0.2) * spec;
    //vec3 result = ambient + diffuse + specular;
//float shadow = ShadowCalculation(FragPosLightSpace);
vec3 result = (ambient + diffuse + specular) * objectColor;
    // Расчет расстояния до камеры
    float distance = length(fragPosition - cameraPos);

    // Объемный туман с градиентом
    float fogFactor = 1.0 - exp(-pow(distance * fogDensity, 20.0));
    float fogFactor1 = 0.5 - exp(-pow(distance * fogDensity, 20.0));
    fogFactor = clamp(fogFactor,fogFactor1, 1.0);

    // Мягкое смешивание с прозрачностью
    float alpha = 1.0; // Можно сделать плавным переходом или добавить альфу
    vec3 color = mix(result, fogColor, fogFactor);

    // Можно добавить альфу для прозрачности
    FragColor = vec4(result, 1.0);
    //FragColor = vec4(color, alpha);
}

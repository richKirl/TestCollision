#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform float exposure; // параметр экспозиции

void main()
{
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    // Тонмэппинг (например, Reinhard)
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // Увеличение яркости
    mapped = vec3(1.0) - exp(-mapped * exposure);
    FragColor = vec4(mapped, 1.0);
}
#version 460 core
in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
        vec4 texel = texture(uTexture,TexCoord);
	if(texel.a == 0.0)
		discard;
	FragColor= texel*vec4(1.0,1.0,1.0,1.0);
}
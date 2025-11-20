#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 colorAlpha;   // color sólido RGBA
uniform sampler2D diffuseTexture;
uniform bool useTexture;   // 0 = color, 1 = textura

void main()
{
    if (useTexture)
        FragColor = texture(diffuseTexture, TexCoords);
    else
        FragColor = colorAlpha;
}

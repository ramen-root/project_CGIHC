#version 330 core

struct Material
{
    sampler2D diffuse;
    float shininess;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;
uniform float time;

// Función para aplicar ruido usando la textura difusa
vec3 applyNoiseEffect(vec2 coords, float time)
{
    vec3 noiseColor = texture(material.diffuse, coords + vec2(sin(time), cos(time))).rgb;
    return noiseColor;
}

void main()
{
    vec4 baseColor = texture(material.diffuse, TexCoords);
    vec3 noiseEffect = applyNoiseEffect(TexCoords, time);
    vec3 finalColor = mix(baseColor.rgb, noiseEffect, 0.5); // Ajusta la mezcla según se requiera
    FragColor = vec4(finalColor, baseColor.a);
}
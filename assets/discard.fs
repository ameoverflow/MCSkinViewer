#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output pixel color
out vec4 finalColor;

void main()
{
    // Texel color fetching from texture map
    vec4 texelColor = texture(texture0, fragTexCoord)*colDiffuse*fragColor;

    // CRITICAL: if the pixel is transparent, discard it completely!
    if (texelColor.a < 0.1) discard;

    finalColor = texelColor;
}
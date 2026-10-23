#version 330 core
in vec2 v_TexCoords;
out vec4 FragColor;

uniform sampler2D u_ScreenTexture;
uniform float u_Strength = 1.0;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_ScreenTexture, 0));

    vec3 topLeft     = texture(u_ScreenTexture, v_TexCoords + vec2(-1.0,  1.0) * texelSize).rgb;
    vec3 topMid      = texture(u_ScreenTexture, v_TexCoords + vec2( 0.0,  1.0) * texelSize).rgb;
    vec3 topRight    = texture(u_ScreenTexture, v_TexCoords + vec2( 1.0,  1.0) * texelSize).rgb;
    vec3 midLeft     = texture(u_ScreenTexture, v_TexCoords + vec2(-1.0,  0.0) * texelSize).rgb;
    vec3 midRight    = texture(u_ScreenTexture, v_TexCoords + vec2( 1.0,  0.0) * texelSize).rgb;
    vec3 bottomLeft  = texture(u_ScreenTexture, v_TexCoords + vec2(-1.0, -1.0) * texelSize).rgb;
    vec3 bottomMid   = texture(u_ScreenTexture, v_TexCoords + vec2( 0.0, -1.0) * texelSize).rgb;
    vec3 bottomRight = texture(u_ScreenTexture, v_TexCoords + vec2( 1.0, -1.0) * texelSize).rgb;

    vec3 sobelX = -topLeft - 2.0 * midLeft - bottomLeft
                + topRight + 2.0 * midRight + bottomRight;
    vec3 sobelY = -topLeft - 2.0 * topMid - topRight
                + bottomLeft + 2.0 * bottomMid + bottomRight;

    float edge = length(sobelX) + length(sobelY);
    vec3 baseColor = texture(u_ScreenTexture, v_TexCoords).rgb;

    FragColor = vec4(baseColor + vec3(edge) * u_Strength, 1.0);
}

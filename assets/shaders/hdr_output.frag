#version 330 core

in vec2 v_TexCoords;
out vec4 FragColor;

uniform sampler2D u_ScreenTexture;
uniform sampler2D u_BloomTexture;

// Set by PostProcess::SetOutputTransform.
uniform int u_UseHDR = 0;

// Opt-in encode for the legacy LDR examples (08/09/10): their scene targets
// hold linear values and they have no HDR pass. 11/12 leave this at 0, so
// their LDR pass-through behavior is unchanged.
uniform int u_EncodeLDR = 0;

// Optional bloom added before tonemapping (HDR path only).
uniform int u_UseBloom = 0;
uniform float u_BloomStrength = 1.0;

void main()
{
    vec3 color = texture(u_ScreenTexture, v_TexCoords).rgb;

    if (u_UseHDR != 0)
    {
        if (u_UseBloom != 0)
        {
            color += texture(u_BloomTexture, v_TexCoords).rgb * u_BloomStrength;
        }

        // Reinhard tonemapping compresses HDR values into [0, 1) and keeps
        // highlight gradients that would otherwise hard-clip.
        vec3 mapped = color / (color + vec3(1.0));

        // The one and only linear -> sRGB conversion of the frame.
        color = pow(mapped, vec3(1.0 / 2.2));
    }

    // LDR path is pass-through when the scene target already holds
    // display-ready (sRGB) values. Legacy examples that store linear values
    // opt into the linear -> sRGB encode here.
    if (u_UseHDR == 0 && u_EncodeLDR != 0)
    {
        color = pow(color, vec3(1.0 / 2.2));
    }

    FragColor = vec4(color, 1.0);
}

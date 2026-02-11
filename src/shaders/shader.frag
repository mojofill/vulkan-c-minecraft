#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float light;
layout(location = 2) in float fragDepth;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

const vec3 fogColor = vec3(0.7, 0.8, 1.0); // sky color
const float fogStart = 100.0;
const float fogEnd   = 110.0;

void main() {
    outColor = light * texture(texSampler, fragTexCoord);

    // uncomment this code for some simple fogging
    vec3 blockColor = texture(texSampler, fragTexCoord).rgb;
    blockColor *= light;

    float fogFactor = clamp(
        (fragDepth - fogStart) / (fogEnd - fogStart),
        0.0, 1.0
    );

    vec3 finalColor = mix(blockColor, fogColor, fogFactor);
    outColor = vec4(finalColor, 1.0);
}
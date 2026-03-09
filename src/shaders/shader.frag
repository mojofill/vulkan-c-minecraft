#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float light;
layout(location = 2) in float fragDepth;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

// (VkClearColorValue) {0.53f, 0.81f, 0.92f, 1.0f};
const vec3 fogColor = vec3(0.53, 0.81, 0.92); // sky color
const float fogStart = 150.0;
const float fogEnd   = 170.0;

void main() {
    outColor = light * texture(texSampler, fragTexCoord);

    // uncomment this code for some simple fogging
    // vec3 blockColor = texture(texSampler, fragTexCoord).rgb;
    // blockColor *= light;

    // float fogFactor = clamp(
    //     (fragDepth - fogStart) / (fogEnd - fogStart),
    //     0.0, 1.0
    // );

    // vec3 finalColor = mix(blockColor, fogColor, fogFactor);
    // outColor = vec4(finalColor, 1.0);
}
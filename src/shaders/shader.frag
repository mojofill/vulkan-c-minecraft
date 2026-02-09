#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float light;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    // outColor = vec4(fragTexCoord, 0.0f, 1.0);
    outColor = light * texture(texSampler, fragTexCoord);
}
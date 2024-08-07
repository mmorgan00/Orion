#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform uniform_object {
    mat4 view; // camera's orientation in space
    mat4 projection;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

void main() {
        gl_Position = ubo.projection * ubo.view * vec4(in_position, 1.0);  
     fragTexCoord = inTexCoord;
}
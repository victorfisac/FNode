// Shader created with FNode 1.0 - Credits: Victor Fisac

#version 330

// Input attributes
in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in vec4 fragColor;

// Uniform attributes
uniform vec3 viewDirection;
uniform mat4 modelMatrix;
uniform float fragCurrentTime;

// Output attributes
out vec4 finalColor;

// Constant and uniform values
uniform sampler2D texture1;
const float node_12 = 0.000;
const vec4 node_10 = vec4(0.250, 1.000, 1.000, 0.000);
const vec3 node_05 = vec3(0.000, 0.000, 0.200);

void main()
{
    vec3 node_07 = texture(texture1, fragTexCoord).rgb;
    vec3 node_09 = fragNormal;
    vec4 node_11 = vec4(node_09, node_12);
    float node_08 = dot(node_11, node_10);
    vec3 node_06 = node_07*node_08;
    vec3 node_04 = node_06 + node_05;
    float node_03 = texture(texture1, fragTexCoord).a;
    if (node_03 == 0.0) discard;
    vec4 node_02 = vec4(node_04, node_03);

    finalColor = node_02;
}
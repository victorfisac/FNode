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
const vec4 node_21 = vec4(0.250, 1.000, 1.000, 0.000);
const vec3 node_28 = vec3(0.000, 0.000, 0.200);

void main()
{
    vec3 node_24 = texture(texture1, fragTexCoord).rgb;
    vec4 node_22 = vec4(fragNormal, 0.0);
    float node_23 = dot(node_21, node_22);
    vec3 node_26 = node_24*node_23;
    vec3 node_27 = node_26 + node_28;
    float node_30 = texture(texture1, fragTexCoord).a;
    if (node_30 == 0.0) discard;
    vec4 node_29 = vec4(node_27, node_30);

    finalColor = node_29;
}
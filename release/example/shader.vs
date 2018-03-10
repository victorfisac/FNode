// Shader created with FNode 1.0 - Credits: Victor Fisac

#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexColor;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 fragColor;

uniform mat4 mvp;
uniform float vertCurrentTime;

// Constant and uniform values
const float node_20 = 1.000;
const float node_28 = 0.000;
const vec4 node_27 = vec4(-1.000, 0.500, 0.500, 0.000);
const float node_17 = 0.016;
uniform sampler2D texture0;
const float node_38 = 3.14159265358979323846;

void main()
{
    fragPosition = vertexPosition;
    fragNormal = vertexNormal;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    mat4 node_41 = mvp;
    vec3 node_23 = vertexPosition;
    vec4 node_22 = vec4(node_23, node_20);
    vec3 node_25 = vertexNormal;
    vec4 node_29 = vec4(node_25, node_28);
    vec4 node_26 = node_29 + node_27;
    vec4 node_24 = normalize(node_26);
    float node_18 = texture(texture0, fragTexCoord).r;
    float node_35 = texture(texture0, fragTexCoord).b;
    float node_33 = node_35*node_38;
    float node_32 = vertCurrentTime;
    float node_34 = node_33 + node_32;
    float node_31 = sin(node_34);
    vec4 node_21 = node_24*node_17*node_18*node_31;
    vec4 node_39 = node_22 + node_21;
    vec4 node_40 = node_41*node_39;

    gl_Position = node_40;
}
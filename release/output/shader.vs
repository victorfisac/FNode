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

uniform mat4 mvpMatrix;
uniform float vertCurrentTime;

// Constant and uniform values
const vec4 node_02 = vec4(-1.000, 0.500, 0.500, 0.000);
const float node_07 = 0.016;
uniform sampler2D texture0;
const float node_13 = 3.14159265358979323846;

void main()
{
    fragPosition = vertexPosition;
    fragNormal = vertexNormal;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    mat4 node_18 = mvpMatrix;
    vec4 node_19 = vec4(vertexPosition, 1.0);
    vec4 node_04 = vec4(fragNormal, 0.0);
    vec4 node_03 = node_02 + node_04;
    vec4 node_05 = normalize(node_03);
    float node_09 = texture(texture0, fragTexCoord).r;
    float node_14 = vertCurrentTime;
    float node_10 = texture(texture0, fragTexCoord).b;
    float node_12 = node_10*node_13;
    float node_15 = node_14 + node_12;
    float node_16 = sin(node_15);
    vec4 node_06 = node_05*node_07*node_09*node_16;
    vec4 node_17 = node_19 + node_06;
    vec4 node_20 = node_18*node_17;

    gl_Position = node_20;
}
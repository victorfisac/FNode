#version 330

#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_REDUCE_MUL   (1.0/8.0)
#define FXAA_SPAN_MAX     8.0

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 viewportSize;

// Output fragment color
out vec4 finalColor;

void main()
{
    vec2 inverse_resolution = vec2(1.0/viewportSize.x,1.0/viewportSize.y);

    vec3 rgbNW = texture(texture0, fragTexCoord.xy + (vec2(-1.0,-1.0))*inverse_resolution).xyz;
    vec3 rgbNE = texture(texture0, fragTexCoord.xy + (vec2(1.0,-1.0))*inverse_resolution).xyz;
    vec3 rgbSW = texture(texture0, fragTexCoord.xy + (vec2(-1.0,1.0))*inverse_resolution).xyz;
    vec3 rgbSE = texture(texture0, fragTexCoord.xy + (vec2(1.0,1.0))*inverse_resolution).xyz;

    vec3 rgbM  = texture(texture0,  fragTexCoord.xy).xyz;

    vec3 luma = vec3(0.299, 0.587, 0.114);

    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))); 

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE)*(0.25*FXAA_REDUCE_MUL),FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),dir*rcpDirMin))*inverse_resolution;

    vec3 rgbA = 0.5*(texture(texture0,   fragTexCoord.xy   + dir*(1.0/3.0 - 0.5)).xyz + texture(texture0,   fragTexCoord.xy   + dir*(2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA*0.5 + 0.25*(texture(texture0,  fragTexCoord.xy   + dir* - 0.5).xyz + texture(texture0,  fragTexCoord.xy   + dir*0.5).xyz);

    float lumaB = dot(rgbB, luma);

    vec4 sum = vec4(0);
    vec4 tc = vec4(0);

    if((lumaB < lumaMin) || (lumaB > lumaMax)) finalColor = vec4(rgbA, 1.0);
    else finalColor = vec4(rgbB,1.0);
}
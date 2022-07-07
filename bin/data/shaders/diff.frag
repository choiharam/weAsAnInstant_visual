#version 330

uniform sampler2D background;
uniform sampler2D stream;
uniform vec2 resolution;
uniform float time;
in vec2 vTexCoord;
out vec4 vFragColor;

float threshold(vec3 c, float thresh){
    return step(thresh, (c.r+c.g+c.b)/3.0);
}

void main(void){
    vec2 st = gl_FragCoord.xy/resolution;
    vec3 vid  = texture(stream,st).rgb;
    vec3 bg = texture(background,st).rgb;
    vec3 color;
    
    float th = 0.9;
    float vidT = threshold(vid, th);
    float bgT = threshold(bg, th);
    
    float diff = abs(vidT - bgT);
    diff = distance(vid,bg);
    color = vec3(step(0.0001,diff));
    
    color = vid;
    vFragColor = vec4(color,1.0);
}


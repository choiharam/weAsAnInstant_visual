#version 330

in vec2 vTexCoord;
out vec4 outColor;

uniform vec2 resolution;
uniform float time;
uniform sampler2D video;
uniform sampler2D backbuffer;
uniform float refreshSpeed;
uniform float vergangen;

float random (vec2 uv){return fract(sin(dot(uv,vec2(12.9898,78.233)))*43758.5453123);}

void main(){
    vec2 st = gl_FragCoord.xy/resolution;
    
    vec4 color=vec4(0);
    vec4 back = texture(backbuffer, st);
    vec4 vid = texture(video, vec2(1.-st.x, st.y));
    
    
//    color = mix(back,vid,1);
    color = mix(back,vid,refreshSpeed);
    
    color += back*vergangen;

    outColor = color;
}

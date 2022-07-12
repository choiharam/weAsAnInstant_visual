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
float smoothEdge(vec2 _st, float fadeSize){
    float fade = smoothstep(0.0, fadeSize, _st.x);
    fade *= 1.0-smoothstep(1.0-fadeSize, 1.0, _st.x);
    fade *= smoothstep(0.0, fadeSize, _st.y);
    fade *= 1.0-smoothstep(1.0-fadeSize, 1.0, _st.y);
    return fade;
}

void main(){
    vec2 st = gl_FragCoord.xy/resolution;
    
    vec4 color=vec4(0);
    vec4 back = texture(backbuffer, st);
    vec4 vid = texture(video, vec2(1.-st.x, st.y));

    color = mix(back,vid,refreshSpeed);
    
    color += back*vergangen;
    
    color *= smoothEdge(st, 0.04);

    outColor = color;
}

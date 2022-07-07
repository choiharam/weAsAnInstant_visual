#version 330

in vec2 vTexCoord;
out vec4 outColor;
uniform vec2 resolution;
uniform float count;
uniform sampler2D mainframe;
uniform sampler2D ghosttex;
uniform float ghostsize;
uniform vec2 ghostpos;
uniform float ghostlife;

float getClamper(vec2 _st, float _scale){
    float result = 1.;
    result *= step(0., _st.x);
    result *= step(_st.x, 1./_scale);
    result *= step(0., _st.y);
    result *= step(_st.y, 1./_scale);
    return result;
}
float smoothClamper(vec2 _st, vec2 _size, vec2 _res, float _scale){
    float fadeAmt = 0.05;
    float clmp = smoothstep(0.0, fadeAmt, _st.x);
    float clmp2 = 1.-smoothstep(_size.x*_scale/_res.x-fadeAmt, _size.x*_scale/_res.x, _st.x);
    float clmp3 = smoothstep(0.0, fadeAmt, _st.y);
    float clmp4 = 1.-smoothstep(_size.y*_scale/_res.y-fadeAmt, _size.y*_scale/_res.y, _st.y);
    return clmp * clmp2 * clmp3 * clmp4;
}
void main(){
    const float PI = 2.0*atan(1.0) * 2.0;
    vec2 st = gl_FragCoord.xy/resolution;
    st.y = 1.-st.y;
    vec3 color=vec3(0);
    float scale = ghostsize;
    vec3 g0;
    
    // ----------- Main frame -----------
    vec3 mframe = texture(mainframe, vec2(st.x*3.-1.,st.y)).rgb;
    color += mframe * step(1./3.,st.x) * (1.-step(2./3.,st.x));
    // ----------- Main frame -----------
    
    if(count >0.0){
        // #beginRepeat
        st-=ghostpos;
        st *= scale;
        vec2 sizee = resolution/scale;
        g0 = texture(ghosttex, st*vec2(3.,1.)).rgb;
        g0 *= smoothClamper(st*vec2(3.,1.), sizee, resolution, scale);
        
        vec3 gg = g0 * sin(ghostlife * PI);
        color += vec3(gg.r+gg.g+gg.b)/3.;
        
        // #endRepeat
    }
    
    
    
    float sa = (color.r+color.g+color.b)/3.;
    color *= step(0.01, sa);
    
    outColor = vec4(color, 1);
}

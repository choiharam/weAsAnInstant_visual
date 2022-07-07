#version 330

in vec2 vTexCoord;
out vec4 outColor;
uniform vec2 resolution;
uniform sampler2D ghosts;
uniform sampler2D mainframe;

float getClamper(vec2 _st, vec2 _size, vec2 _res, vec2 _pos, float _scale){
    float result = 1.;
    vec2 sizeNorm = _size/_res;
    result *= step(_pos.x, _st.x/_scale+_pos.x);
    result *= 1.-step(_pos.x + sizeNorm.x, _st.x/_scale+_pos.x);
    result *= step(_pos.y, _st.y/_scale+_pos.y);
    result *= 1.-step(_pos.y + sizeNorm.y, _st.y/_scale+_pos.y);
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
//-------------------------------------------------------------------------------------
void main(){
    const float PI = 2.0*atan(1.0) * 2.0;
    vec2 st = gl_FragCoord.xy/resolution;
    vec3 color=vec3(0);// = texture(ghosts, st).rgb;
    vec3 gst = texture(ghosts, vec2(1.)-st).rgb;
    color += gst;
    vec3 mframe = texture(mainframe, vec2(1.)-vec2(st.x*3.-1.,st.y)).rgb;
    color += mframe * step(1./3.,st.x);
    
    outColor = vec4(color, 1);
}

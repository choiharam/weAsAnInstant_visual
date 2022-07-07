#version 330

in vec2 vTexCoord;
out vec4 outColor;
uniform vec2 resolution;
uniform float count;
uniform sampler2D ghost;
uniform sampler2D video;
uniform vec2 size;

float getClamper(vec2 _st, vec2 _size, vec2 _res, vec2 _pos, vec2 _scale){
    float result = 1.;
    vec2 sizeNorm = _size/_res;
    result *= step(_pos.x, _st.x/_scale.x+_pos.x);
    result *= 1.-step(_pos.x + sizeNorm.x, _st.x/_scale.x+_pos.x);
    result *= step(_pos.y, _st.y/_scale.y+_pos.y);
    result *= 1.-step(_pos.y + sizeNorm.y, _st.y/_scale.y+_pos.y);
    return result;
}

float smoothClamper(vec2 _st, vec2 _size, vec2 _res, vec2 _scale){
    float fadeAmt = 0.05;
    float clmp = smoothstep(0.0, fadeAmt, _st.x);
    float clmp2 = 1.-smoothstep(_size.x*_scale.x/_res.x-fadeAmt, _size.x*_scale.x/_res.x, _st.x);
    float clmp3 = smoothstep(0.0, fadeAmt, _st.y);
    float clmp4 = 1.-smoothstep(_size.y*_scale.x/_res.y-fadeAmt, _size.y*_scale.x/_res.y, _st.y);
    return clmp * clmp2 * clmp3 * clmp4;
}
//-------------------------------------------------------------------------------------
void main(){
    const float PI = 2.0*atan(1.0) * 2.0;
    vec2 st = gl_FragCoord.xy/resolution;
    vec3 color =vec3(0);
    vec2 scale = vec2(3,1);
    
    
    
        // grab current scene
    
    vec2 pos = vec2(0.,0.);
    st -= pos;
    st *= scale;
    vec2 sizee = resolution/scale;
    vec3 rtime = texture(video, st).rgb;
    rtime *= smoothClamper(st, sizee, resolution, scale);
        // end of grab current scene
    
    color += rtime*2;
    color *= 0.6;
    outColor = vec4(color, 1);
}

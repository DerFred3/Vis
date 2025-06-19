#version 410

in vec3 entryPoint;
out vec4 result;

uniform sampler3D volume;

uniform float smoothStepStart;
uniform float smoothStepWidth;
uniform float oversampling;
uniform vec3 voxelCount;
uniform vec3 cameraPosInTextureSpace;
uniform vec3 minBounds;
uniform vec3 maxBounds;

vec4 transferFunction(float v) {
  v = clamp((v - smoothStepStart) / (smoothStepWidth), 0.0, 1.0);
  return vec4(v*v * (3-2*v));
}

vec4 under(vec4 current, vec4 last) {
  last.rgb = last.rgb + (1.0-last.a) * current.a * current.rgb;
  last.a   = last.a + (1.0-last.a) * current.a;
  return last;
}

bool inBounds(vec3 pos) {
  return pos.x >= minBounds.x && pos.y >= minBounds.y && pos.z >= minBounds.z &&
         pos.x <= maxBounds.x && pos.y <= maxBounds.y && pos.z <= maxBounds.z;
}

void main() {
  // compute vector to camera in texture space  
  vec3 rayDirectionInTextureSpace = normalize(entryPoint-cameraPosInTextureSpace);

  // compute delta
  float samples = dot(abs(rayDirectionInTextureSpace),voxelCount);
  float opacityCorrection = 100/(samples*oversampling);
  vec3 delta = rayDirectionInTextureSpace/(samples*oversampling);

  vec3 currentPoint = entryPoint;
  result = vec4(0.0);
  do {
    // TODO: Implement raycaster and store the result in "result".
    vec4 sampled = texture(volume, currentPoint);
    vec4 current = vec4(transferFunction(sampled.r));
    result = under(current, result);

    currentPoint += delta;

    if (result.a >= 0.95) break;
  } while (inBounds(currentPoint));
}

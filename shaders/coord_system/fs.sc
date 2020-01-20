$input texCoord

#include <../common.sh>

SAMPLER2D(texture1, 0);
SAMPLER2D(texture2, 1);

void main() {
    gl_FragColor = mix(texture2D(texture1, texCoord), texture2D(texture2, texCoord), 0.2);
}

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define UNDEFINED -1

const uint8_t NUM_CHANNELS = 3;
const int32_t NUM_INTENSITIES = 4;

// this should be obtained through training

const int32_t C_DISCONTINUITY = 1;
const int32_t C_GRADIENT = 1;
const int32_t C_INTENSITY = 1;

enum Colors {
    RED = 0,
    GREEN,
    BLUE
};

enum PixelType {
    GRADIENT=0,
    CMYK,
    GRAYSCALE,
    RGB
};

enum DetectionMethod {
    HORIZONTAL=0,
    VERTICAL,
    POLAR,

    MAX_METHODS
};

#define M_PI 3.14159265358979323846  /* pi */

#endif // CONSTANTS_H

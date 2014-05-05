#ifndef DYNAMICEDGEDETECTOR_H
#define DYNAMICEDGEDETECTOR_H

#include <vector>
#include <QColor>
#include <cstdint>
#include <cmath>

#include "edgedetector.h"

const uint8_t NUM_CHANNELS = 3;

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
    RGB,
    CMYK,
    GRAYSCALE
};

class DynamicEdgeDetector : public EdgeDetector
{
    public:
        DynamicEdgeDetector() { }
        DynamicEdgeDetector(QImage* image, uint32_t const& width, uint32_t const& height);

        void calc();
        void calcIntensities();
        void calcGradients();
        void forwardScan();
        void backwardTrack();

        template <typename T>
        T getCost(uint32_t index, uint32_t disc, PixelType type) const;

        template <typename T>
        std::pair<uint32_t, T> getCost(uint32_t x, uint32_t y, uint32_t disc, PixelType type) const;

        uint32_t getWidth() const { return _width; }
        uint32_t getHeight() const { return _height; }

    private:
        std::vector<int32_t> _intensities;
        std::vector<int32_t> _gradients;
        std::vector<uint32_t> _fwdScanPtrs;
        std::vector<int32_t> _accumulated;

        uint32_t _width;
        uint32_t _height;
};

#endif // DYNAMICEDGEDETECTOR_H

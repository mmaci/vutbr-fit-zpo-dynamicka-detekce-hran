#ifndef DYNAMICEDGEDETECTOR_H
#define DYNAMICEDGEDETECTOR_H

#include <vector>
#include <QColor>
#include <cstdint>
#include <cmath>
#include <stdint.h>

#include "edgedetector.h"

#define UNDEFINED -1

const uint8_t NUM_CHANNELS = 3;
const int32_t NUM_INTENSITIES = 4;

// this should be obtained through training

const int32_t C_EDGE = 1;
const int32_t C_GRAY_LEVEL = 1;

const uint32_t C_RADIUS = 3;

enum Colors {
    RED = 0,
    GREEN,
    BLUE
};

enum PixelType {
    RGB,
    CMYK,
    GRAYSCALE,
    MYTEST
};

enum DetectionMethod {
    HORIZONTAL = 0,
    VERTICAL
};

class DynamicEdgeDetector : public EdgeDetector
{
    public:
        DynamicEdgeDetector() { }
        DynamicEdgeDetector(QImage* image, uint32_t const& width, uint32_t const& height);

        void calc(DetectionMethod method);
        void calcIntensities(DetectionMethod method);
        void calcGradients(DetectionMethod method);
        void forwardScan(DetectionMethod method);
        void backwardTrack(DetectionMethod method);
        void backwardTrackEdge(uint32_t const& startX, uint32_t const& startY);

        void calcCost(DetectionMethod method);
        void calcCost(uint32_t x, uint32_t y, DetectionMethod method);

        int32_t edgeStrength(uint32_t x, uint32_t y, DetectionMethod method);
        int32_t getMaxGradMagnitude(DetectionMethod method);
        int32_t getGradMagnitude(uint32_t x, uint32_t y, DetectionMethod method);

        int32_t* getAverageIntensity(uint32_t x, uint32_t y, uint32_t radius);
        int32_t grayDiff(uint32_t x, uint32_t y, uint32_t radius);

        template <typename T>
        T getCost(uint32_t const& index, uint32_t const& disc, PixelType const& type) const;

        template <typename T>
        std::pair<uint32_t, T> getCost(uint32_t const& x, uint32_t const& y, uint32_t const& disc, PixelType const& type) const;

        uint32_t getIndex(uint32_t const& x, uint32_t const& y) const {
            return x * getHeight() + y;
        }

        uint32_t getWidth() const { return _width; }
        uint32_t getHeight() const { return _height; }

    private:
        std::vector<int32_t> _intensities;
        std::vector<int32_t> _gradients;
        std::vector<int32_t> _ptrs;
        std::vector<int32_t> _accumulated;

        std::vector<int32_t> _cost;

        uint32_t _width;
        uint32_t _height;

        int32_t _maxGradMagnitude[2];
};

#endif // DYNAMICEDGEDETECTOR_H

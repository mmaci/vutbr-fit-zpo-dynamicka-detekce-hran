#ifndef DYNAMICEDGEDETECTOR_H
#define DYNAMICEDGEDETECTOR_H

#include <vector>
#include <QColor>
#include <cstdint>
#include <cmath>
#include <stdint.h>

#include "edgedetector.h"

#include "constants.h"

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
        void backwardTrackEdge(uint32_t const& startX, uint32_t const& startY);

        template <typename T>
        T getCost(uint32_t const& index, uint32_t const& disc) const;

        template <typename T>
        std::pair<uint32_t, T> getCost(uint32_t const& x, uint32_t const& y, uint32_t const& disc) const;

        uint32_t getDiff(uint32_t newX, uint32_t newY,uint32_t oldX,uint32_t oldY);

        uint32_t getIndex(uint32_t const& x, uint32_t const& y) const {
            return x * getHeight() + y;
        }

        uint32_t getPolarIndex(uint32_t const& x, uint32_t const& y) const {
            return x * getPolarHeight() + y;
        }

        QImage* makePolarImage();

        QImage* getPolarImage() { return _polarImage; }

        uint32_t getWidth() const { return _width; }
        uint32_t getHeight() const { return _height; }

        uint32_t getPolarWidth() const { return _polarWidth; }
        uint32_t getPolarHeight() const { return _polarHeight; }

        void setType(PixelType value);
        void setMethod(DetectionMethod value);
        void setThreshold(int32_t value);
        void setColor(QRgb value){_edgeColor = value; }
    private:
        std::vector<int32_t> _intensities;
        std::vector<int32_t> _gradients;
        std::vector<int32_t> _ptrs;
        std::vector<int32_t> _accumulated;
        std::vector<int32_t> _cartesianToPolarTable;

        PixelType _type;
        DetectionMethod _method;
        int32_t _threshold;
        QRgb _edgeColor;

        uint32_t _width;
        uint32_t _height;

        uint32_t _polarWidth;
        uint32_t _polarHeight;

        QImage* _polarImage;
};

#endif // DYNAMICEDGEDETECTOR_H

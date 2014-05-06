#ifndef DNAMICEDGEDETECTOR2_H
#define DNAMICEDGEDETECTOR2_H

#include <vector>
#include <QColor>
#include <cstdint>
#include <cmath>
#include <stdint.h>

#include "dynamicedgedetector.h"

class DynamicEdgeDetector2 : public EdgeDetector
{
public:
    DynamicEdgeDetector2() { }
    DynamicEdgeDetector2(QImage* image, uint32_t const& width, uint32_t const& height);

    void forwardScan();
    void backwardTrack();
    void backwardTrackEdge(uint32_t const& startX, uint32_t const& startY);

    void calc();

    uint32_t getDiff(uint32_t newX, uint32_t newY,uint32_t oldX,uint32_t oldY);

    uint32_t getWidth() const { return _width; }
    uint32_t getHeight() const { return _height; }

    uint32_t getIndex(uint32_t const& x, uint32_t const& y) const {
        return x * getHeight() + y;
    }

    void setType(PixelType value){_type = value;}
    void setMethod(DetectionMethod value){_method = value;}
    void setThreshold(int32_t value){_threshold = value;}
    void setColor(QRgb value){_edgeColor = value; }

private:
    std::vector<int32_t> _ptrs;
    std::vector<int32_t> _accumulated;

    PixelType _type;
    DetectionMethod _method;
    int32_t _threshold;
    QRgb _edgeColor;

    uint32_t _width;
    uint32_t _height;
};

#endif // DNAMICEDGEDETECTOR2_H

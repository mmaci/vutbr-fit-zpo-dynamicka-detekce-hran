#include "dynamicedgedetector.h"

#include <iostream>


DynamicEdgeDetector::DynamicEdgeDetector(QImage* image, uint32_t const& width, uint32_t const& height) :
    _width(width), _height(height), EdgeDetector(image) {

    _gradients = std::vector<int32_t>(width * height * 3, 0);
    _intensities = std::vector<int32_t>(width * height * 3, 0);
    _fwdScanPtrs = std::vector<uint32_t>(width * height, 0);
    _accumulated = std::vector<int32_t>(width * height, 0);
}

void DynamicEdgeDetector::calc() {
    calcIntensities();
    calcGradients();
    forwardScan();
}

void DynamicEdgeDetector::calcIntensities() {

    for (uint32_t x = 0; x < getWidth(); ++x) {
        for (uint32_t y = 0; y < getHeight(); ++y) {            
                int32_t op = std::min(4, static_cast<int32_t>(getHeight() - 1) - static_cast<int32_t>(y));
                for (uint8_t i = 0; i < op; ++i) {
                    QColor color = getImage()->pixel(x, y + i);
                    _intensities[(getHeight() * x + y) * 3] += color.red();
                    _intensities[(getHeight() * x + y) * 3 + 1] += color.green();
                    _intensities[(getHeight() * x + y) * 3 + 2] += color.blue();
                }
        } // HEIGHT
    } // WIDTH
}

void DynamicEdgeDetector::calcGradients() {
    for (uint32_t x = 0; x < getWidth(); ++x) {
        for (uint32_t y = 0; y < getHeight(); ++y) {
            for (int8_t i = -1; i <= 1; i += 2) {
                uint32_t Y = std::min(std::max(0, static_cast<int32_t>(y - i)), static_cast<int32_t>(getHeight() - 1));
                QColor color = getImage()->pixel(x, Y);
                _gradients[(getHeight() * x + y) * 3] += i * color.red();
                _gradients[(getHeight() * x + y) * 3 + 1] += i * color.green();
                _gradients[(getHeight() * x + y) * 3 + 2] += i * color.blue();
            }
        } // HEIGHT
    } // WIDTH
}

void DynamicEdgeDetector::forwardScan() {
    for (uint32_t x = 0; x < getWidth() - 1; ++x) {
        for (uint32_t y = 0; y < getHeight(); ++y) {

            int32_t minCost = INT_MAX;
            uint32_t minIndex;

            for (int8_t i = -2; i <= 2; ++i) {
                int32_t cost = 0;
                uint32_t Y = std::min(std::max(0, static_cast<int32_t>(y + i)), static_cast<int32_t>(getHeight() - 1));
                uint32_t X = x + 1;
                int32_t disc = abs(i);

                uint32_t index = getHeight() * X + Y;

                for (uint8_t channel = RED; channel < NUM_CHANNELS; ++channel) {

                    int32_t intensity = _intensities[index * 3 + channel];
                    int32_t gradient = _gradients[index * 3 + channel];

                    cost += C_DISCONTINUITY * disc - C_GRADIENT * gradient - C_INTENSITY * intensity;
                }

                if (cost < minCost) {
                    minIndex = index;
                    minCost = cost;
                }

                _fwdScanPtrs[getHeight() * x + y] = minIndex;
                _accumulated[getHeight() * x + y] = minCost;
            }

        } // HEIGHT
    } // WIDTH
}

// TODO: rewrite this to work dynamically
// this is just a temporary for one edge to detect
void DynamicEdgeDetector::backwardTrack() {

    int32_t minCost = INT_MAX;
    uint32_t minIndex;
    for (uint32_t y = 0; y < getHeight(); ++y) {
        if (_accumulated[y] < minCost) {
            minIndex = y;
            minCost = _accumulated[y];
        }
    }

    QImage* image = getImage();
    uint32_t x_i = 0;
    uint32_t y_i = minIndex;
    uint32_t nextIndex;
    for (uint32_t x = 0; x < getWidth(); ++x) {
        QRgb color = qRgb(255, 0, 0);
        image->setPixel(x_i, y_i, color);
        nextIndex = _fwdScanPtrs[x_i * getHeight() + y_i];
        y_i = nextIndex % getHeight();
        x_i = nextIndex / getHeight();
    }
}


#include "dynamicedgedetector.h"

#include <iostream>


DynamicEdgeDetector::DynamicEdgeDetector(QImage* image, uint32_t const& width, uint32_t const& height) :
    _width(width), _height(height), EdgeDetector(image) {

    _gradients = std::vector<int32_t>(width * height * 3, 0);
    _intensities = std::vector<int32_t>(width * height * 3, 0);
    _ptrs = std::vector<int32_t>(width * height, UNDEFINED);
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
                    uint32_t index = getIndex(x, y);
                    _intensities[index * 3] += color.red();
                    _intensities[index * 3 + 1] += color.green();
                    _intensities[index * 3 + 2] += color.blue();
                }
        } // HEIGHT
    } // WIDTH
}

void DynamicEdgeDetector::calcGradients() {
    for (uint32_t x = 0; x < getWidth(); ++x) {
        for (uint32_t y = 0; y < getHeight(); ++y) {
            uint32_t index = getIndex(x, y);
            for (int8_t i = -1; i <= 1; i += 2) {
                uint32_t Y = std::min(std::max(0, static_cast<int32_t>(y - i)), static_cast<int32_t>(getHeight() - 1));
                QColor color = getImage()->pixel(x, Y);                
                _gradients[index * 3] += i * color.red();
                _gradients[index * 3 + 1] += i * color.green();
                _gradients[index * 3 + 2] += i * color.blue();
            }
        } // HEIGHT
    } // WIDTH
}

template<typename T>
T DynamicEdgeDetector::getCost(uint32_t const& index, uint32_t const& disc, PixelType const& type) const {
    T cost = 0;
    switch (type) {
        case RGB:
        {
            for (uint8_t channel = RED; channel < NUM_CHANNELS; ++channel) {

                int32_t intensity = _intensities[index * 3 + channel];
                int32_t gradient = _gradients[index * 3 + channel];

                if (disc)
                    cost += C_DISCONTINUITY * disc;
                cost += -C_GRADIENT * gradient - C_INTENSITY * intensity;
            }
            break;
        }

        default:
            break;
    }
    return cost;
}

template<typename T>
std::pair<uint32_t, T> DynamicEdgeDetector::getCost(uint32_t const& x, uint32_t const& y, uint32_t const& disc = 0, PixelType const& type = RGB) const {
    uint32_t index = getIndex(x, y);
    int32_t cost = getCost<int32_t>(index, disc, RGB);

    return std::pair<uint32_t, T>(index, cost);
}



void DynamicEdgeDetector::forwardScan() {

    // calc first layer
    for (uint32_t y = 0; y < getHeight(); ++y) {
        std::pair<uint32_t, int32_t> cost = getCost<int32_t>(0, y);
        _accumulated[y] = cost.second;
    }

    for (uint32_t x = 1; x < getWidth(); ++x) {
        for (uint32_t y = 0; y < getHeight(); ++y) {

            int32_t minCost = INT_MAX;
            uint32_t minIndex;
            std::pair<uint32_t, int32_t> curCost = getCost<int32_t>(x, y);

            for (int8_t i = -2; i <= 2; ++i) {

                uint32_t Y = std::min(std::max(0, static_cast<int32_t>(y + i)), static_cast<int32_t>(getHeight() - 1));
                uint32_t X = x - 1;
                uint32_t disc = abs(i);

                std::pair<uint32_t, int32_t> cost = getCost<int32_t>(X, Y, disc, RGB);

                if (cost.second < minCost) {
                    minIndex = cost.first;
                    minCost = cost.second;
                }

                uint32_t index = getIndex(x, y);

                _ptrs[index] = static_cast<int32_t>(minIndex);
                _accumulated[index] = minCost + curCost.second;
            }

        } // HEIGHT
    } // WIDTH
}

// TODO: rewrite this to work dynamically
// this is just a temporary for one edge to detect
void DynamicEdgeDetector::backwardTrack() {

    int32_t avg = 0;
    for (uint32_t y = 0; y < getHeight(); ++y) {        
        avg += _accumulated[getIndex(getWidth() - 1, y)];
    }
    avg /= getHeight();

    avg/=2; // only trace edges from 25%
    for (uint32_t y = 0; y < getHeight(); ++y) {
        if (_accumulated[getIndex(getWidth()-1, y)] < avg) {

            backwardTrackEdge(getWidth()-1, y);

        }
    }
}

void DynamicEdgeDetector::backwardTrackEdge(uint32_t const& startX, uint32_t const& startY) {

    // edge color
    QRgb color = qRgb(0, 255, 0);

    QImage* image = getImage();

    uint32_t x_i = startX, y_i = startY;
    uint32_t index = getIndex(x_i, y_i);
    while (index != UNDEFINED) {
        image->setPixel(x_i, y_i, color);

        index = _ptrs[index];
        y_i = index % getHeight();
        x_i = index / getHeight();
    }
}


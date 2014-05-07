#include "dynamicedgedetector.h"

#include <iostream>
#include <Windows.h>

#define DEBUG


DynamicEdgeDetector::DynamicEdgeDetector(QImage* image, uint32_t const& width, uint32_t const& height) :
    _width(width), _height(height), EdgeDetector(image),
{

    _gradients = std::vector<int32_t>(width * height * 3, 0);
    _intensities = std::vector<int32_t>(width * height * 3, 0);
    _ptrs = std::vector<int32_t>(width * height, UNDEFINED);
    _accumulated = std::vector<int32_t>(width * height, 0);

    _cost = std::vector<int32_t>(width * height);

    _maxGradMagnitude[HORIZONTAL] = UNDEFINED;
    _maxGradMagnitude[VERTICAL] = UNDEFINED;
}

void DynamicEdgeDetector::calc(DetectionMethod method) {
    calcIntensities(method);
    calcGradients(method);
    forwardScan(method);
}

void DynamicEdgeDetector::calcCost(uint32_t x, uint32_t y, DetectionMethod method = VERTICAL) {
    return C_EDGE * edgeStrength(x, y, method) + C_GRAY_LEVEL * grayDiff(x, y, C_RADIUS);
}

int32_t DynamicEdgeDetector::edgeStrength(uint32_t x, uint32_t y, DetectionMethod method = VERTICAL) {
    return (getMaxGradMagnitude(method) - getGradMagnitude(x, y, method)) / getMaxGradMagnitude(method);
}

void sizeDiff(uint32_t x, uint32_t y) {

}

int32_t* DynamicEdgeDetector::getAverageIntensity(uint32_t x, uint32_t y, uint32_t radius) {
    QImage* image = getImage();

    uint32_t startX = max(0, x - radius), endX = min(getWidth() - 1, x + radius);
    uint32_t startY = max(0, y - radius), endY = min(getHeight() - 1, y + radius);

    uint32_t sum[3] = {0, 0, 0};
    for (uint32_t x = startX; x <= endX; ++x) {
        for(uint32_t y = startY; y <= endY; ++y) {
            sum[RED] += image->pixel(x, y).red();
            sum[GREEN] += image->pixel(x, y).green();
            sum[BLUE] += image->pixel(x, y).blue();
        }
    }

    uint32_t div = (endX - startX) * (endY - startY);
    for (uint32_t ch = RED; ch < NUM_CHANNELS; ++ch)
        sum[ch] /= div;

    return sum;
}

int32_t DynamicEdgeDetector::grayDiff(uint32_t x, uint32_t y, uint32_t radius) {
    QImage* image = getImage();

    int32_t* avgIntensity = getAverageIntensity(x, y, radius);

    return sqrt(abs(image->pixel(x, y) - avgIntensity[RED])
              + abs(image->pixel(x, y) - avgIntensity[GREEN])
              + abs(image->pixel(x, y) - avgIntensity[BLUE]));
}

int32_t DynamicEdgeDetector::getMaxGradMagnitude(DetectionMethod method = VERTICAL) {
    /* potential improvement can be made by taking only a certain percentil */

    if (_maxGradMagnitude[method] == UNDEFINED) {
        _maxGradMagnitude = MIN_INT;

        for (uint32_t y = 0; y < getHeight(); ++y) {
            for (uint32_t x = 0; x < getWidth(); ++x) {
                int32_t gradMagnitude = getGradMagnitude(x, y, method);
                if (_maxGradMagnitude[method] > gradMagnitude)
                    _maxGradMagnitude[method] = gradMagnitude;
            }
        }
    }

    return _maxGradMagnitude;
}

int32_t DynamicEdgeDetector::getGradMagnitude(uint32_t x, uint32_t y, DetectionMethod method = VERTICAL) {
    QImage* image = getImage();
    QColor c1, c2;
    switch (method) {
        case HORIZONTAL:
            if (x-1 <0 || x+1 >= getWidth())
                return UNDEFINED;

            c1 = image->pixel(x-1,y);
            c2 = image->pixel(x+1, y);
            break;

        case VERTICAL:
            if (y-1 <0 || y+1 >= getHeight())
                return UNDEFINED;

            c1 = image->pixel(x-1,y);
            c2 = image->pixel(x+1, y);
            break;

        default:
            return 0;
    }

    return abs(c1.red() - c2.red()) + abs(c1.green() - c2.green()) + abs(c1.blue() - c2.blue());
}

void DynamicEdgeDetector::calcCost(DetectionMethod method) {
    #ifdef DEBUG
        std::cout << "Calculating costs ..." << std::endl;
        uint32_t startTime = GetTickCount();
    #endif
    switch (method) {
        case HORIZONTAL:
        {
            for (uint32_t y = 0; y < getHeight(); ++y) {
                // the same as getIndex(0, y) but without computational overhead
                // when scanning horizontally use VERTICAL calcCost
                _cost[y] = calcCost(0, y, VERTICAL);
            }

            for (uint32_t x = 1; x < getWidth(); ++x) {
                for (uint32_t y = 0; y < getHeight(); ++y) {
                        int32_t minCost = MAX_INT;
                        for (uint8_t i = -NUM_INTENSITIES; i < NUM_INTENSITIES; ++i) {
                            uint32_t X = x - 1;
                            uint32_t Y = max(0, min(getHeight()-1, y + i));
                            uint32_t index = getIndex(X, Y);

                            if (_cost[index] < minCost)
                                minCost = _cost[index];
                        }
                        _cost[getIndex(x, y)] = minCost + calcCost(x, y, VERTICAL);
                } // HEIGHT
            } // WIDTH
            break;
        }
        case VERTICAL:
        {
            for (uint32_t x = 0; x < getWidth(); ++x) {
                // the same as getIndex(0, y) but without computational overhead
                // when scanning horizontally use VERTICAL calcCost
                _cost[y] = calcCost(x, 0, HORIZONTAL);
            }

            for (uint32_t y = 1; y < getHeight(); ++y) {
                for (uint32_t x = 0; x < getWidth(); ++x) {
                        int32_t minCost = MAX_INT;
                        for (uint8_t i = -NUM_INTENSITIES; i < NUM_INTENSITIES; ++i) {
                            uint32_t Y = y - 1;
                            uint32_t X = max(0, min(getWidth()-1, x + i));
                            uint32_t index = getIndex(X, Y);

                            if (_cost[index] < minCost)
                                minCost = _cost[index];
                        }
                        _cost[getIndex(x, y)] = minCost + calcCost(x, y, HORIZONTAL);
                } // HEIGHT
            } // WIDTH
            break;
        }
    }

    #ifdef DEBUG
        uint32_t diff = GetTickCount() - startTime;
        std::cout << "Finished in " << diff << "ms." << std::endl;
    #endif
}

void DynamicEdgeDetector::calcGradients(DetectionMethod method) {
    #ifdef DEBUG
        std::cout << "Calculating gradients ..." << std::endl;
        uint32_t startTime = GetTickCount();
    #endif

    switch (method) {
        case HORIZONTAL:
        {
            for (uint32_t x = 0; x < getWidth(); ++x) {
                for (uint32_t y = 0; y < getHeight(); ++y) {
                    uint32_t index = getIndex(x, y);
                    for (int8_t i = -1; i <= 1; i += 2) {
                        uint32_t Y = min(max(0, static_cast<int32_t>(y - i)), static_cast<int32_t>(getHeight() - 1));
                        QColor color = getImage()->pixel(x, Y);
                        _gradients[index * 3] += i * color.red();
                        _gradients[index * 3 + 1] += i * color.green();
                        _gradients[index * 3 + 2] += i * color.blue();
                    }
                } // HEIGHT
            } // WIDTH
            break;
        }
        case VERTICAL:
        {
            for (uint32_t x = 0; x < getWidth(); ++x) {
                for (uint32_t y = 0; y < getHeight(); ++y) {
                    uint32_t index = getIndex(x, y);
                    for (int8_t i = -1; i <= 1; i += 2) {
                        uint32_t X = min(max(0, static_cast<int32_t>(x - i)), static_cast<int32_t>(getWidth() - 1));
                        QColor color = getImage()->pixel(X, y);
                        _gradients[index * 3] += i * color.red();
                        _gradients[index * 3 + 1] += i * color.green();
                        _gradients[index * 3 + 2] += i * color.blue();
                    }
                } // HEIGHT
            } // WIDTH
            break;
        }
    }

    #ifdef DEBUG
        uint32_t diff = GetTickCount() - startTime;
        std::cout << "Finished in " << diff << "ms." << std::endl;
    #endif
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



void DynamicEdgeDetector::forwardScan(DetectionMethod method) {

    #ifdef DEBUG
        std::cout << "Forward scanning ..." << std::endl;
        uint32_t startTime = GetTickCount();
    #endif

    switch (method) {
        case HORIZONTAL:
        {
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

                        uint32_t Y = min(max(0, static_cast<int32_t>(y + i)), static_cast<int32_t>(getHeight() - 1));
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
            break;
        }

        case VERTICAL:
        {
            // calc first layer
            for (uint32_t x = 0; x < getWidth(); ++x) {
                std::pair<uint32_t, int32_t> cost = getCost<int32_t>(x, 0);
                _accumulated[getIndex(x, 0)] = cost.second;
            }

            for (uint32_t x = 0; x < getWidth(); ++x) {
                for (uint32_t y = 1; y < getHeight(); ++y) {

                    int32_t minCost = INT_MAX;
                    uint32_t minIndex;
                    std::pair<uint32_t, int32_t> curCost = getCost<int32_t>(x, y);

                    for (int8_t i = -2; i <= 2; ++i) {

                        uint32_t X = min(max(0, static_cast<int32_t>(x + i)), static_cast<int32_t>(getWidth() - 1));
                        uint32_t Y = y - 1;
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
            break;
        }
    }

    #ifdef DEBUG
        uint32_t diff = GetTickCount() - startTime;
        std::cout << "Finished in " << diff << "ms." << std::endl;
    #endif
}

// TODO: rewrite this to work dynamically
// this is just a temporary for one edge to detect
void DynamicEdgeDetector::backwardTrack(DetectionMethod method) {

    #ifdef DEBUG
        std::cout << "Backward tracking ..." << std::endl;
        uint32_t startTime = GetTickCount();
    #endif;


    int32_t min = INT_MAX;
    uint32_t minIndex;
    switch (method) {
        case HORIZONTAL:
        {

            for (uint32_t y = 0; y < getHeight(); ++y) {
                if (_accumulated[getIndex(getWidth() - 1, y)] < min) {
                    uint32_t index = getIndex(getWidth() - 1, y);
                    min = _accumulated[index];
                    minIndex = index;
                }
            }

            backwardTrackEdge(minIndex / getHeight(), minIndex % getHeight());

            break;
        }

        case VERTICAL:
        {
            int32_t threshold;
            for (uint32_t y = 0; y < getHeight(); ++y) {
                min = INT_MAX;

                for (uint32_t x = 0; x < getWidth(); ++x) {
                    if (_accumulated[getIndex(x, y)] < min) {
                        uint32_t index = getIndex(x, y);
                        min = _accumulated[index];
                        minIndex = index;
                    }
                }

                if (y == 0) {
                    threshold = min;
                }

                if (min <= threshold)
                    backwardTrackEdge(minIndex / getHeight(), minIndex % getHeight());
            }



            break;
        }
    }

    #ifdef DEBUG
        uint32_t diff = GetTickCount() - startTime;
        std::cout << "Finished in " << diff << "ms." << std::endl;
    #endif
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


#include "dynamicedgedetector.h"

#include <iostream>
#include <Windows.h>

#define DEBUG


DynamicEdgeDetector::DynamicEdgeDetector(QImage* image, uint32_t const& width, uint32_t const& height) :
    _width(width), _height(height), EdgeDetector(image) {

    _gradients = std::vector<int32_t>(width * height * 3, 0);
    _intensities = std::vector<int32_t>(width * height * 3, 0);
    _ptrs = std::vector<int32_t>(width * height, UNDEFINED);
    _accumulated = std::vector<int32_t>(width * height, 0);
    _type = RGB;
    _method = HORIZONTAL;
    _threshold = 1;
    _edgeColor = qRgb(0,0,0);

    _polarImage = makePolarImage();
}

void DynamicEdgeDetector::calc() {
    calcIntensities();
    calcGradients();
    forwardScan();
}

void DynamicEdgeDetector::calcIntensities() {
    #ifdef DEBUG
        std::cout << "Calculating intensities ..." << std::endl;
        uint32_t startTime = GetTickCount();
    #endif
    switch (_method) {
        case HORIZONTAL:
        {
            for (uint32_t x = 0; x < getWidth(); ++x) {
                for (uint32_t y = 0; y < getHeight(); ++y) {
                        int32_t op = qMin(NUM_INTENSITIES, static_cast<int32_t>(getHeight() - 1) - static_cast<int32_t>(y));
                        for (uint8_t i = 0; i < op; ++i) {
                            QColor color = getImage()->pixel(x, y + i);
                            uint32_t index = getIndex(x, y);
                            _intensities[index * 3] += color.red();
                            _intensities[index * 3 + 1] += color.green();
                            _intensities[index * 3 + 2] += color.blue();
                        }
                } // HEIGHT
            } // WIDTH
            break;
        }
        case VERTICAL:
        {
            for (uint32_t x = 0; x < getWidth(); ++x) {
                for (uint32_t y = 0; y < getHeight(); ++y) {
                        for (uint8_t i = 0; i < NUM_INTENSITIES; ++i) {
                            QColor color = getImage()->pixel(qMin(qMax(static_cast<uint32_t>(0), static_cast<uint32_t>(x + i)), getWidth()-1), y);
                            uint32_t index = getIndex(x, y);
                            _intensities[index * 3] += color.red();
                            _intensities[index * 3 + 1] += color.green();
                            _intensities[index * 3 + 2] += color.blue();
                        }
                } // HEIGHT
            } // WIDTH
            break;
        }
        case POLAR:
        {
            _intensities = std::vector<int32_t>(getPolarWidth() * getPolarHeight() * 3, 0);
            for (uint32_t x = 0; x < getPolarWidth(); ++x) {
                for (uint32_t y = 0; y < getPolarHeight(); ++y) {
                    int32_t op = qMin(NUM_INTENSITIES, static_cast<int32_t>(getPolarHeight() - 1) - static_cast<int32_t>(y));
                        for (uint8_t i = 0; i < op; ++i) {
                            QColor color = getPolarImage()->pixel(x, y + i);
                            uint32_t index = getPolarIndex(x, y);
                            _intensities[index * 3] += color.red();
                            _intensities[index * 3 + 1] += color.green();
                            _intensities[index * 3 + 2] += color.blue();
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

void DynamicEdgeDetector::calcGradients() {
    #ifdef DEBUG
        std::cout << "Calculating gradients ..." << std::endl;
        uint32_t startTime = GetTickCount();
    #endif

    switch (_method) {
        case HORIZONTAL:
        {
            for (uint32_t x = 0; x < getWidth(); ++x) {
                for (uint32_t y = 0; y < getHeight(); ++y) {
                    uint32_t index = getIndex(x, y);
                    for (int8_t i = -1; i <= 1; i += 2) {
                        uint32_t Y = qMin(qMax(0, static_cast<int32_t>(y - i)), static_cast<int32_t>(getHeight() - 1));
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
                        uint32_t Y = qMin(qMax(0, static_cast<int32_t>(y - i)), static_cast<int32_t>(getHeight() - 1));
                        QColor color = getImage()->pixel(x, Y);
                        _gradients[index * 3] += i * color.red();
                        _gradients[index * 3 + 1] += i * color.green();
                        _gradients[index * 3 + 2] += i * color.blue();
                    }
                } // HEIGHT
            } // WIDTH
        break;

        }
        case POLAR:
        {
            _gradients = std::vector<int32_t>(getPolarWidth() * getPolarHeight() * 3, 0);
            for (uint32_t x = 0; x < getPolarWidth(); ++x) {
                for (uint32_t y = 0; y < getPolarHeight(); ++y) {
                    uint32_t index = getPolarIndex(x, y);
                    for (int8_t i = -1; i <= 1; i += 2) {
                        uint32_t X = qMin(qMax(0, static_cast<int32_t>(x - i)), static_cast<int32_t>(getPolarWidth() - 1));
                        QColor color = getPolarImage()->pixel(X, y);
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
T DynamicEdgeDetector::getCost(uint32_t const& index, uint32_t const& disc, PixelType const& type, bool polar) const {
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
std::pair<uint32_t, T> DynamicEdgeDetector::getCost(uint32_t const& x, uint32_t const& y, uint32_t const& disc, PixelType const& type, bool polar) const {
    uint32_t index;
    if (polar)
        index = getPolarIndex(x, y);
    else
        index = getIndex(x, y);

    int32_t cost = getCost<int32_t>(index, disc, type, polar);

    return std::pair<uint32_t, T>(index, cost);
}



void DynamicEdgeDetector::forwardScan() {

    #ifdef DEBUG
        std::cout << "Forward scanning ..." << std::endl;
        uint32_t startTime = GetTickCount();
    #endif

    switch (_method) {
        case HORIZONTAL:
        {
            // calc first layer
            for (uint32_t y = 0; y < getHeight(); ++y) {
                std::pair<uint32_t, int32_t> cost = getCost<int32_t>(0, y, 0, _type, false);
                _accumulated[y] = cost.second;
            }

            for (uint32_t x = 1; x < getWidth(); ++x) {
                for (uint32_t y = 0; y < getHeight(); ++y) {

                    int32_t minCost = INT_MAX;
                    uint32_t minIndex;
                    std::pair<uint32_t, int32_t> curCost = getCost<int32_t>(x, y, 0, _type, false);

                    for (int8_t i = -2; i <= 2; ++i) {

                        uint32_t Y = qMin(qMax(0, static_cast<int32_t>(y + i)), static_cast<int32_t>(getHeight() - 1));
                        uint32_t X = x - 1;
                        uint32_t disc = abs(i);

                        std::pair<uint32_t, int32_t> cost = getCost<int32_t>(X, Y, disc, _type, false);

                        if (cost.second < minCost) {
                            minIndex = cost.first;
                            minCost = cost.second;
                        }
                    }
                    uint32_t index = getIndex(x, y);

                    _ptrs[index] = static_cast<int32_t>(minIndex);
                    _accumulated[index] = minCost + curCost.second;

                } // HEIGHT
            } // WIDTH
            break;
        }

        case VERTICAL:
        {
            // calc first layer
            for (uint32_t x = 0; x < getWidth(); ++x) {
                std::pair<uint32_t, int32_t> cost = getCost<int32_t>(x, 0, 0, _type, false);
                _accumulated[getIndex(x, 0)] = cost.second;
            }

            for (uint32_t x = 0; x < getWidth(); ++x) {
                for (uint32_t y = 1; y < getHeight(); ++y) {

                    int32_t minCost = INT_MAX;
                    uint32_t minIndex;
                    std::pair<uint32_t, int32_t> curCost = getCost<int32_t>(x, y, 0, _type, false);

                    for (int8_t i = -2; i <= 2; ++i) {

                        uint32_t X = qMin(qMax(0, static_cast<int32_t>(x + i)), static_cast<int32_t>(getWidth() - 1));
                        uint32_t Y = y - 1;
                        uint32_t disc = abs(i);

                        std::pair<uint32_t, int32_t> cost = getCost<int32_t>(X, Y, disc, _type, false);

                        if (cost.second < minCost) {
                            minIndex = cost.first;
                            minCost = cost.second;
                        }
                    }
                    uint32_t index = getIndex(x, y);

                    _ptrs[index] = static_cast<int32_t>(minIndex);
                    _accumulated[index] = minCost + curCost.second;


                } // HEIGHT
            } // WIDTH
            break;
        }

        case POLAR:
        {
            _ptrs = std::vector<int32_t>(getPolarWidth() * getPolarHeight(), UNDEFINED);
            _accumulated = std::vector<int32_t>(getPolarWidth() * getPolarHeight(), 0);

            // calc first layer
            for (uint32_t y = 0; y < getPolarHeight(); ++y) {
                std::pair<uint32_t, int32_t> cost = getCost<int32_t>(0, y, 0, _type, true);
                _accumulated[y] = cost.second;
            }


            for (uint32_t x = 1; x < getPolarWidth(); ++x) {
                for (uint32_t y = 0; y < getPolarHeight(); ++y) {

                    int32_t minCost = INT_MAX;
                    uint32_t minIndex;
                    std::pair<uint32_t, int32_t> curCost = getCost<int32_t>(x, y, 0, _type, true);

                    for (int8_t i = -2; i <= 2; ++i) {

                        uint32_t Y = qMin(qMax(0, static_cast<int32_t>(y + i)), static_cast<int32_t>(getPolarHeight() - 1));
                        uint32_t X = x - 1;
                        uint32_t disc = abs(i);

                        std::pair<uint32_t, int32_t> cost = getCost<int32_t>(X, Y, disc, _type, true);

                        if (cost.second < minCost) {
                            minIndex = cost.first;
                            minCost = cost.second;
                        }
                    }

                    uint32_t index = getPolarIndex(x, y);

                    _ptrs[index] = static_cast<int32_t>(minIndex);
                    _accumulated[index] = minCost + curCost.second;

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
void DynamicEdgeDetector::backwardTrack() {

    #ifdef DEBUG
        std::cout << "Backward tracking ..." << std::endl;
        uint32_t startTime = GetTickCount();
    #endif;

    int32_t avg = 0;
    switch (_method) {
        case HORIZONTAL:
        {
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
            break;
        }

        case VERTICAL:
        {
            for (uint32_t x = 0; x < getWidth(); ++x) {
                avg += _accumulated[getIndex(x, getHeight() - 1)];
            }
            avg /= getWidth();

            avg/=2;
            for (uint32_t x = 0; x < getWidth(); ++x) {
                if (_accumulated[getIndex(x, getHeight()-1)] < avg) {

                    backwardTrackEdge(x, getHeight()-1);

                }
            }
            break;
        }

        case POLAR:
        {
            for (uint32_t y = 0; y < getPolarHeight(); ++y) {
                avg += _accumulated[getPolarIndex(getPolarWidth() - 1, y)];
            }
            avg /= getPolarHeight();

            avg/=2; // only trace edges from 25%
            for (uint32_t y = 0; y < getPolarHeight(); ++y) {
                if (_accumulated[getPolarIndex(getPolarWidth()-1, y)] < avg) {

                    backwardTrackEdge(getPolarWidth()-1, y, true);

                }
            }
            break;
        }

    }

    #ifdef DEBUG
        uint32_t diff = GetTickCount() - startTime;
        std::cout << "Finished in " << diff << "ms." << std::endl;
    #endif
}

QImage* DynamicEdgeDetector::makePolarImage() {

    QImage* originalImage = getImage();

    float centerX = getWidth() / 2.f, centerY = getHeight() / 2.f;
    float radius = sqrt(centerX * centerX + centerY * centerY);
    uint32_t heightTransform = 360;
    _polarHeight = heightTransform;
    uint32_t widthTransform = static_cast<uint32_t>(radius);
    _polarWidth = widthTransform;
    _cartesianToPolarTable = std::vector<int32_t>(heightTransform * widthTransform, UNDEFINED);

    QImage* polarImage = new QImage(widthTransform, heightTransform, QImage::Format_RGB888);
    for (uint32_t y = 0; y < heightTransform; ++y) {
        for (uint32_t x = 0; x < widthTransform; ++x) {

            float r = static_cast<float>(x);
            float angle = (y / static_cast<float>(heightTransform)) * M_PI * 2;

            uint32_t xx = static_cast<uint32_t>(max(0, min(getWidth()-1, r * cos(angle) + centerX)));
            uint32_t yy = static_cast<uint32_t>(max(0, min(getHeight()-1, r * sin(angle) + centerY)));

            polarImage->setPixel(x, y, originalImage->pixel(xx, yy));
            _cartesianToPolarTable[getPolarIndex(x, y)] = getIndex(xx, yy);
        }
    }

    return polarImage;
}

void DynamicEdgeDetector::backwardTrackEdge(uint32_t const& startX, uint32_t const& startY, bool polar) {

    // edge color
   QRgb color = _edgeColor;

    QImage* image;
    if (!polar) {
        image = getImage();
        uint32_t x_i = startX, y_i = startY;
        uint32_t index = getIndex(x_i, y_i);
        while (index != UNDEFINED) {
            image->setPixel(x_i, y_i, color);

            index = _ptrs[index];
            y_i = index % getHeight();
            x_i = index / getHeight();
        }
    }
    else {
        image = getImage();
        uint32_t origIndex, originX, originY;
        uint32_t polarIndex = getPolarIndex(startX, startY);

        while (polarIndex != UNDEFINED) {

            origIndex = _cartesianToPolarTable[polarIndex];
            originY = origIndex % getHeight();
            originX = origIndex / getHeight();

            image->setPixel(originX, originY, color);

            polarIndex = _ptrs[polarIndex];
        }
    }


}

void DynamicEdgeDetector::setType(PixelType value)
{
    _type = value;
}

void DynamicEdgeDetector::setMethod(DetectionMethod value)
{
    _method = value;
}

void DynamicEdgeDetector::setThreshold(int32_t value)
{
    _threshold = value;
}


#include "dynamicedgedetector2.h"

#define DEBUG

DynamicEdgeDetector2::DynamicEdgeDetector2(QImage* image, uint32_t const& width, uint32_t const& height) :
    _width(width), _height(height), EdgeDetector(image) {

    _ptrs = std::vector<int32_t>(width * height, UNDEFINED);
    _accumulated = std::vector<int32_t>(width * height, 0);
    _type = RGB;
    _method = HORIZONTAL;
    _threshold = 1;
    _edgeColor = qRgb(0,0,0);
}


void DynamicEdgeDetector2::forwardScan() {

#ifdef DEBUG
    std::cout << "Forward scanning ..." << std::endl;
    uint32_t startTime = GetTickCount();
#endif

    switch (_method) {
    case HORIZONTAL:
    {
        // calc first layer
        for (uint32_t y = 0; y < getHeight(); ++y) {
            //std::pair<uint32_t, int32_t> cost = getCost<int32_t>(0, y, 0, _type);
            _accumulated[y] = 0;
        }

        for (uint32_t x = 1; x < getWidth(); ++x) {
            for (uint32_t y = 0; y < getHeight(); ++y) {

                uint32_t minCost = INT_MAX;
                uint32_t minIndex;
                for (int8_t i = -2; i <= 2; ++i) {

                    uint32_t Y = qMin(qMax(0, static_cast<int32_t>(y + i)), static_cast<int32_t>(getHeight() - 1));
                    uint32_t X = x - 1;
                    uint32_t oldIndex = getIndex(X,Y);
                    uint32_t diff = getDiff(x,y, X, Y);
                    uint32_t cost = _accumulated[oldIndex] + diff;
                    if (cost < minCost) {
                        minIndex = oldIndex;
                        minCost = cost;
                    }


                }
                uint32_t index = getIndex(x, y);

                _ptrs[index] = static_cast<int32_t>(minIndex);
                _accumulated[index] = minCost;
            } // HEIGHT
        } // WIDTH
        break;
    }

    case VERTICAL:
    {
        // calc first layer
        for (uint32_t x = 0; x < getWidth(); ++x) {
            _accumulated[getIndex(x, 0)] = 0;
        }

        for (uint32_t x = 0; x < getWidth(); ++x) {
            for (uint32_t y = 1; y < getHeight(); ++y) {

                uint32_t minCost = INT_MAX;
                uint32_t minIndex;

                for (int8_t i = -2; i <= 2; ++i) {

                    uint32_t X = qMin(qMax(0, static_cast<int32_t>(x + i)), static_cast<int32_t>(getWidth() - 1));
                    uint32_t Y = y - 1;

                    uint32_t oldIndex = getIndex(X,Y);
                    uint32_t diff = getDiff(x,y, X, Y);
                    uint32_t cost = _accumulated[oldIndex] + diff;
                    if (cost < minCost) {
                        minIndex = oldIndex;
                        minCost = cost;
                    }

                }
                uint32_t index = getIndex(x, y);

                _ptrs[index] = static_cast<int32_t>(minIndex);
                _accumulated[index] = minCost;

            } // HEIGHT
        } // WIDTH
        break;
    }
    case POLAR:
    case MAX_METHODS:    break;
    }

#ifdef DEBUG
    uint32_t diff = GetTickCount() - startTime;
    std::cout << "Finished in " << diff << "ms." << std::endl;
#endif
}

void DynamicEdgeDetector2::calc()
{
    forwardScan();
}

uint32_t DynamicEdgeDetector2::getDiff(uint32_t newX, uint32_t newY, uint32_t oldX, uint32_t oldY)
{
    uint32_t diff=0;
    QColor akt= getImage()->pixel(newX,newY);
    QColor old= getImage()->pixel(oldX,oldY);
    switch (_type) {
    case RGB:
    {

        diff = abs(akt.red()-old.red());
        diff += abs(akt.green()-old.green());
        diff += abs(akt.blue()-old.blue());
        break;
    }
    case CMYK:
    {
        akt.toCmyk();
        old.toCmyk();
        diff = abs(akt.cyan()-old.cyan());
        diff += abs(akt.magenta()-old.magenta());
        diff += abs(akt.yellow()-old.yellow());
        diff += abs(akt.black()-old.black());
        break;
    }case GRAYSCALE:
    {
        uint16_t greyscaleAkt = 0.2126 * akt.red() + 0.7152 *akt.green() + 0.0722 *akt.blue();
        uint16_t greyscaleold =0.2126 * old.red() + 0.7152 *old.green() + 0.0722 *old.blue();
        diff = abs(greyscaleAkt - greyscaleold);
        break;

    }
    }
    return diff;
}


void DynamicEdgeDetector2::backwardTrack() {
#ifdef DEBUG
    std::cout << "Backward tracking ..." << std::endl;
    uint32_t startTime = GetTickCount();
#endif

    switch (_method) {
    case HORIZONTAL:
    {
        for (uint32_t y= 0; y < getHeight(); y+=_threshold)
        {
            backwardTrackEdge(getWidth()-1, y);
        }

        break;
    }

    case VERTICAL:
    {

        for (uint32_t x = 0; x < getWidth(); x+=_threshold) {

            backwardTrackEdge(x, getHeight()-1);

        }
        break;
    }
    case POLAR:break;
    }

#ifdef DEBUG
    uint32_t diff = GetTickCount() - startTime;
    std::cout << "Finished in " << diff << "ms." << std::endl;
#endif
}

void DynamicEdgeDetector2::backwardTrackEdge(uint32_t const& startX, uint32_t const& startY) {

    // edge color
    QRgb color = _edgeColor;

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

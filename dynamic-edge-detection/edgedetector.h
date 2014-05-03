#ifndef EDGEDETECTOR_H
#define EDGEDETECTOR_H

#include <QImage>

class EdgeDetector
{
    public:
        EdgeDetector(QImage* image = NULL)
            : _image(image){ }

        QImage* getImage() { return _image; }

    private:
        QImage* _image;

};

#endif // EDGEDETECTOR_H

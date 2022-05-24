#ifndef __IMGEX_TRANSFORM_H
#define __IMGEX_TRANSFORM_H

/* Transforms - how to transform an image after it's loaded.
 * The base class for Transform is implemented in common.hh
 * Some transforms are stored with the image; some are session specific
 */
#include "common.hh"
#include <QRect>


/** Zoom */
class tf_zoom : public transform {
private:
    float zoom_;
protected:
    transform_id type() const noexcept override { return transform_id::TX_ZOOM; }
public:
    tf_zoom() noexcept : transform(), zoom_(1.0f) {}
    tf_zoom(float z) noexcept : transform(), zoom_(z) {}

    QRect apply(QRect bbox, QPixmap &img) const override;
};


/** Pan - change the offset of the top left corner */
class tf_move : public transform {
private:
    int x_, y_;
protected:
    transform_id type() const noexcept override { return transform_id::TX_MOVE; }
public:
    tf_move() noexcept : x_(0), y_(0) {}
    tf_move(int x, int y) noexcept : x_(x), y_(y) {}

    QRect apply(QRect bbox, QPixmap &img) const override;
};


class tf_crop : public transform {
private:
    /** crop box relative to pixmap size */
    QRect box_;
protected:
    transform_id type() const noexcept override { return transform_id::TX_CROP; }
public:
    tf_crop() noexcept {}
    tf_crop(QRect const &qr) noexcept : box_(qr) {}

    QRect apply(QRect bbox, QPixmap &img) const override;
};

#endif

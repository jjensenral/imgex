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
    transform *clone() const override;

    QRect apply(Transformable &owner, QRect bbox, QPixmap &img) const override;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<transform>(*this);
        ar & zoom_;
    }

    void copy_from(const transform &other) override;
};


/** Move in parent window - change the offset of the top left corner */
class tf_move : public transform {
private:
    int x_, y_;
protected:
    transform_id type() const noexcept override { return transform_id::TX_MOVE; }
public:
    tf_move() noexcept : x_(0), y_(0) {}
    tf_move(int x, int y) noexcept : x_(x), y_(y) {}
    transform *clone() const override;

    QRect apply(Transformable &owner, QRect bbox, QPixmap &img) const override;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<transform>(*this);
        ar & x_;
        ar & y_;
    }
    void copy_from(const transform &other) override;
};


class tf_crop : public transform {
private:
    /** crop box relative in pixmap (local) coordinates */
    int x_, y_, w_, h_;
protected:
    transform_id type() const noexcept override { return transform_id::TX_CROP; }
public:
    tf_crop() noexcept {}
    tf_crop(QRect const &qr) noexcept : x_(qr.x()), y_(qr.y()), w_(qr.width()), h_(qr.height()) {}
    transform *clone() const override;

    QRect apply(Transformable &owner, QRect bbox, QPixmap &img) const override;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<transform>(*this);
        ar & x_;
        ar & y_;
        ar & w_;
        ar & h_;
    }
    void copy_from(const transform &other) override;
};

#endif

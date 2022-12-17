#include "decor.hh"
#include "image.hh"
#include "transform.hh"

#include <algorithm>
#include <iterator>
#include <QPixmap>
#include <iostream>



struct transform::transform_t {
    // Starting from a new image in upper left (0,0) transformations are applied in the following order
    QRect crop_;
    float zoom_;
    QPoint move_;

    constexpr transform_t() noexcept : crop_(), zoom_(1.0), move_(0, 0) {}

    // void reset() noexcept;
    /** Zoom to absolute zoom factor */
    void zoom_to(float);
    /** Move top left corner to global coordinate (in main window) */
    void move_to(QPoint);
    /** Crop to global coordinates */
    void crop(QRect);
};

void
transform::transform_t::crop(QRect q)
{

}

void transform::transform_t::zoom_to(float) {

}

void transform::transform_t::move_to(QPoint) {

}


transform::transform() : impl_(new transform_t)
{
}


transform &transform::operator=(const transform &other) {
    if(this != &other) {
        *impl_ = *other.impl_;
    }
    return *this;
}


transform::~transform()
{
    delete impl_;
    impl_ = nullptr;
}


std::ostream &
operator<<(std::ostream &os, transform const &wf)
{
    auto const crop{wf.impl_->crop_};
    os << "[C(" << crop.x() << ',' << crop.y() << ';'
            << crop.width() << ',' << crop.height() << "),Z("
       << wf.impl_->zoom_ << "),M("
       << wf.impl_->move_.x() << ',' << wf.impl_->move_.y() << ")]\n";
    return os;
}

void transform::crop(QRect crop) noexcept
{
    impl_->crop_.adjust(crop.x(), crop.y(), 0, 0);
    impl_->crop_.setSize(crop.size());
}

void transform::move_to(QPoint p) noexcept
{
    impl_->move_ = p;
}

void transform::zoom_to(float g) noexcept
{
    impl_->zoom_ = g;
}

float transform::get_zoom() const noexcept
{
    return impl_->zoom_;
}




Transformable::Transformable(const ImageFile &fn) : txfs_(), img_(), wbox_()
{
    QString path{fn.getPath()};
    if(!img_.load(path))
        throw FileNotFound(path);
    // XXX for now, all new transformable start at global upper left
    wbox_ = QRect(QPoint(0,0), img_.size());
    // TODO: restore transform associated with ImageFile and run it
}


void
Transformable::add_from_decorator(const XILDecorator &dec)
{
    // FIXME
//    transform *tf{dec.to_transform()};
//    add_transform(tf);
}



void
Transformable::run()
{
    // FIXME
}


QRect Transformable::move_to(QPoint point)
{
    QRect oldbox{wbox_};
    wbox_.setTopLeft(point);
    txfs_.move_to(point);
    return oldbox | wbox_;
}

QRect Transformable::zoom_to(float g)
{
    QRect oldbox{wbox_};
    QSize target{wbox_.size()};
    target.setHeight(target.height() * g + 0.99f );
    target.setWidth(target.width() * g + 0.99f );
    img_.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    // keep the upper left corner in place for now
    wbox_.setSize(target);
    // One box will be larger than the other depending on whether we zoom in or out
    return oldbox | wbox_;
}

QRect Transformable::crop(QRect c)
{
    QRect oldbox{wbox_};
    // crop the current image
    wbox_.adjust(c.x(), c.y(), 0, 0);
    img_ = img_.copy(c);
    // Update the transform by unzooming the current crop instructions
    auto z = txfs_.get_zoom();
    c.setSize(c.size() / z);
    // topleft is local - relative to the current image
    c.setTopLeft(c.topLeft() / z);
    txfs_.crop(c);
    // Since we crop within the image oldbox should always be the larger
    return oldbox;
}


void Transformable::copy_from(const Transformable &orig)
{
    img_ = orig.img_.copy();
    wbox_ = orig.wbox_;
    txfs_ = orig.txfs_;
}

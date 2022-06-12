#include "transform.hh"
#include "decor.hh"
#include "image.hh"
#include <algorithm>
#include <QPixmap>


/* Defined in common.hh
 * A workflow is implemented by, quite simply, applying one transform at a time, to *this
 */

workflow::~workflow()
{
    //std::for_each(w_.begin(), w_.end(), [](transform const *p) { delete p; });
    w_.clear();
}


Transformable::Transformable(const ImageFile &fn) : txfs_(), img_(), wbox_()
{
    QString path{fn.getPath()};
    if(!img_.load(path))
        throw FileNotFound(path);
    // XXX for now, all new transformable start at global upper left
    wbox_ = QRect(QPoint(0,0), img_.size());
    // TODO: restore workflow associated with ImageFile and run it
}


void
Transformable::add_from_decorator(const XILDecorator &dec)
{
    transform *tf{dec.to_transform()};
    add_transform(tf);
    apply(tf);
}


void Transformable::apply(transform *tf)
{
    wbox_ = tf->apply(*this, img_.rect(), img_);
}


void Transformable::moveto(QPoint point)
{
    wbox_.setTopLeft(point);
}

void Transformable::add_transform(transform *tf)
{
    txfs_.add(tf);
    apply(tf);
}


transform *
tf_zoom::clone() const
{
    tf_zoom *z = new tf_zoom();
    z->zoom_ = zoom_;
}


QRect
tf_zoom::apply(Transformable &owner, QRect bbox, QPixmap &img) const
{
    QSize target{bbox.size()};
    target.setHeight(target.height() * zoom_ + 0.99f );
    target.setWidth(target.width() * zoom_ + 0.99f );
    img.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    // keep the upper left corner in place for now
    bbox.setSize(target);
    return bbox;
}


QRect tf_move::apply(Transformable &owner, QRect bbox, QPixmap &img) const
{
    // XXX not implemented yet
    bbox.adjust(x_, y_, 0, 0);
    return transform::apply(owner, bbox, img);
}


transform *
tf_move::clone() const
{
    tf_move *mv = new tf_move();
    mv->x_ = x_;
    mv->y_ = y_;
    return mv;
}



transform *
tf_crop::clone() const
{
    tf_crop *c = new tf_crop();
    c->x_ = x_; c->y_ = y_; c->w_ = w_; c->h_ = h_;
    return c;
}


QRect tf_crop::apply(Transformable &owner, QRect bbox, QPixmap &img) const
{
    QRect box(x_, y_, w_, h_);
    img = img.copy(box);
    // Image is moved to where the crop rectangle is (but in global coords)
    bbox.adjust(x_, y_, 0, 0);
    owner.moveto(bbox.topLeft());
    bbox.setWidth(box.width());
    bbox.setHeight(box.height());
    // Return global coordinates
    return transform::apply(owner, bbox, img);
}

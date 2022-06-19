#include "transform.hh"
#include "decor.hh"
#include "image.hh"
#include <algorithm>
#include <QPixmap>
#include <iostream>


/* Defined in common.hh
 * A workflow is implemented by, quite simply, applying one transform at a time, to *this
 */

workflow::~workflow()
{
    //std::for_each(w_.begin(), w_.end(), [](transform const *p) { delete p; });
    w_.clear();
}


void
workflow::add(transform *t)
{
    if(!w_.empty()) {
        transform *last = w_.back();
        if (t->type() == last->type()) {
            *last += *t;
            delete t;
        }
    }
    w_.push_back(t);
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


void
transform::merge(transform *other)
{
    copy_from(*other);
}



transform *
tf_zoom::clone() const
{
    tf_zoom *z = new tf_zoom();
    z->zoom_ = zoom_;
}


void
tf_zoom::copy_from(const transform &other)
{
    if(other.type() != transform::transform_id::TX_ZOOM)
        throw bad_transform();
    zoom_ = dynamic_cast<tf_zoom const &>(other).zoom_;
}


QRect
tf_zoom::apply(Transformable &owner, QRect bbox, QPixmap &img) const
{
    owner.workCopy();
    // Resize around focus point, or centre?
//	QPoint focus{ c.isNull() ? img_.rect().center() : c };
    QPoint focus{ img.rect().center() };

    // Target size
    QSize size{ bbox.size() };
    size.setHeight( size.height() * zoom_ + 0.99f );
    size.setWidth( size.width() * zoom_ + 0.99f );
    QRect oldbox = bbox;
    img = img.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    bbox.setSize( img.size() );
    bbox.moveCenter(focus);


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


void tf_move::copy_from(const transform &other)
{
    if(other.type() != transform::transform_id::TX_MOVE)
        throw bad_transform();
    tf_move const &o = dynamic_cast<tf_move const &>(other);
    x_ = o.x_;
    y_ = o.y_;
}

transform *
tf_crop::clone() const
{
    tf_crop *c = new tf_crop();
    c->x_ = x_; c->y_ = y_; c->w_ = w_; c->h_ = h_;
    return c;
}


void
tf_crop::copy_from(const transform &other)
{
    if(other.type() != transform::transform_id::TX_CROP)
        throw bad_transform();
    tf_crop const &o = dynamic_cast<tf_crop const &>(other);
    x_ = o.x_; y_ = o.y_;
    w_ = o.w_; h_ = o.h_;
}


QRect tf_crop::apply(Transformable &owner, QRect bbox, QPixmap &img) const
{
    QRect box(x_, y_, w_, h_);
    img = img.copy(box);
    std::cerr << "crop " << x_ << ' ' << y_ << ' ' << w_ << ' ' << h_ << std::endl;
    // Image is moved to where the crop rectangle is (but in global coords)
    bbox.adjust(x_, y_, 0, 0);
    owner.moveto(bbox.topLeft());
    bbox.setWidth(box.width());
    bbox.setHeight(box.height());
    // Return global coordinates
    return transform::apply(owner, bbox, img);
}

#include "transform.hh"
#include "decor.hh"
#include "image.hh"
#include <algorithm>
#include <iterator>
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


std::ostream &
operator<<(std::ostream &os, workflow const &wf)
{
    os << '[';
    auto const end = wf.w_.cend();
    auto cur = wf.w_.cbegin();
    if(cur != end) {
        for (;;) {
            os << **cur;
            if (++cur == end)
                break;
            os << ',';
        }
    }
    os << ']' << std::endl;
    return os;
}


void
workflow::add(transform *t)
{
    if(!w_.empty()) {
        switch (t->type()) {
            case transform::transform_id::TX_NOP:
                break;
            case transform::transform_id::TX_CROP:
                break;
            case transform::transform_id::TX_ZOOM_TO:
                // Because zoom is absolute, it replaces all previous instances
                remove_type(transform::transform_id::TX_ZOOM_TO);
                w_.push_front(t);
                return;
            case transform::transform_id::TX_MOVE_TO:
                remove_type(transform::transform_id::TX_MOVE_TO);
                // Place move_to first in list after the zoom
                auto q{w_.begin()};
                if (!w_.empty() && (*q)->type() == transform::transform_id::TX_ZOOM_TO)
                    std::advance(q, 1);
                w_.insert(q, t);
                return;
        }
    }
    w_.push_back(t);
}


void
workflow::remove_type(transform::transform_id type, void (*cb)(const transform *))
{
    auto const the_end = w_.end();
    auto q = w_.begin();
    while( q != the_end ) {
        if ((*q)->type() == type) {
            if (cb) cb(*q);
            q = w_.erase(q);
        } else
            ++q;
    }
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


void Transformable::apply(transform const *tf)
{
    wbox_ = tf->apply(*this, img_.rect(), img_);
}


void
Transformable::run()
{
    std::for_each(txfs_.cbegin(), txfs_.cend(),
                  [this](transform const *tf) { apply(tf); });
}


void Transformable::moveto(QPoint point)
{
    wbox_.setTopLeft(point);
}


void Transformable::add_transform(transform *tf)
{
    //apply(tf);   // must apply first since add hands over ownership
    txfs_.add(tf);
}


void
transform::merge(transform *other)
{
    // default merge (of transforms of the same type) is to become the second one
    // (which is then discarded)
    copy_from(*other);
}


std::ostream &
operator<<(std::ostream &os, transform const &tf)
{
    tf.print(os);
    return os;
}


void transform::print(std::ostream &os) const
{
    os << "BASE()";
}


transform *
tf_zoom_to::clone() const
{
    tf_zoom_to *z = new tf_zoom_to();
    z->zoom_ = zoom_;
    return z;
}


void
tf_zoom_to::copy_from(const transform &other)
{
    if(other.type() != transform::transform_id::TX_ZOOM_TO)
        throw bad_transform();
    zoom_ = dynamic_cast<tf_zoom_to const &>(other).zoom_;
}


QRect
tf_zoom_to::apply(Transformable &owner, QRect bbox, QPixmap &img) const
{
    owner.workCopy();
    // Resize around focus point, or centre?
//	QPoint focus{ c.isNull() ? img_.rect().center() : c };
    QPoint focus{ img.rect().center() };

#if 0
    // Target size
    QSize size{ bbox.size() };
    size.setHeight( size.height() * zoom_ + 0.99f );
    size.setWidth( size.width() * zoom_ + 0.99f );
    QRect oldbox = bbox;
    img = img.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    bbox.setSize( img.size() );
    bbox.moveCenter(focus);
#endif

    QSize target{bbox.size()};
    target.setHeight(target.height() * zoom_ + 0.99f );
    target.setWidth(target.width() * zoom_ + 0.99f );
    img.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    // keep the upper left corner in place for now
    bbox.setSize(target);
    return bbox;
}


void
tf_zoom_to::print(std::ostream &os) const
{
    os << "ZOOM(" << zoom_ << ')';
}


QRect tf_move_to::apply(Transformable &owner, QRect bbox, QPixmap &img) const
{
    bbox.setTopLeft(QPoint(x_,y_));
    return transform::apply(owner, bbox, img);
}


transform *
tf_move_to::clone() const
{
    tf_move_to *mv = new tf_move_to();
    mv->x_ = x_;
    mv->y_ = y_;
    return mv;
}


void tf_move_to::copy_from(const transform &other)
{
    if(other.type() != transform::transform_id::TX_MOVE_TO)
        throw bad_transform();
    tf_move_to const &o = dynamic_cast<tf_move_to const &>(other);
    x_ = o.x_;
    y_ = o.y_;
}


void
tf_move_to::print(std::ostream &os) const
{
    os << "MOVE(" << x_ << ',' << y_ << ')';
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


void
tf_crop::print(std::ostream &os) const
{
    os << "CROP(" << x_ << ' ' << y_ << ' ' << w_ << ' ' << h_ << ')';
}
#include "transform.hh"
#include "decor.hh"
#include <algorithm>
#include <QPixmap>


/* Defined in common.hh
 * A workflow is implemented by, quite simply, applying one transform at a time, to *this
 */

workflow::~workflow()
{
    std::for_each(w_.begin(), w_.end(), [](transform const *p) { delete p; });
    w_.clear();
}


void
Transformable::add_from_decorator(const XILDecorator &dec)
{
    transform *tf{dec.to_transform()};
}


QRect
Transformable::apply(QPixmap &img)
{
    QRect bbox{QPoint(0,0), img.size()};
    std::for_each(txfs_.cbegin(), txfs_.cend(),
                  [&](transform const *tf){ bbox = tf->apply(*this, bbox, img); });
    return bbox;
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
tf_crop::clone() const
{
    tf_crop *c = new tf_crop();
    c->box_ = box_;
    return c;
}


QRect tf_crop::apply(Transformable &owner, QRect bbox, QPixmap &img) const
{
    // shift owner to crop_.topLeft
    // scroll pixmap by crop_.topLeft
    // truncate pixmap to crop_.size
}

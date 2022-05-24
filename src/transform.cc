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
Transformable::apply(QPixmap &img) const
{
    QRect bbox{QPoint(0,0), img.size()};
    std::for_each(txfs_.cbegin(), txfs_.cend(),
                  [&](transform const *tf){ bbox = tf->apply(bbox, img); });
    return bbox;
}


QRect
tf_zoom::apply(QRect bbox, QPixmap &img) const
{
    QSize target{bbox.size()};
    target.setHeight(target.height() * zoom_ + 0.99f );
    target.setWidth(target.width() * zoom_ + 0.99f );
    img.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    // keep the upper left corner in place for now
    bbox.setSize(target);
    return bbox;
}


QRect tf_move::apply(QRect bbox, QPixmap &img) const
{
    // XXX: check whether the pan is possible
    bbox.adjust(x_, y_, 0, 0);
    return transform::apply(bbox, img);
}


QRect tf_crop::apply(QRect bbox, QPixmap &img) const
{
    // TODO
    return transform::apply(bbox, img);
}

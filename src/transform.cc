#include "decor.hh"
#include "image.hh"
#include "transform.hh"

#include <algorithm>
#include <iterator>
#include <QPixmap>
#include <iostream>



std::ostream &
operator<<(std::ostream &os, Transformable::transform const &wf)
{
    auto const crop{wf.crop_};
    os << "[C(" << crop.x() << ',' << crop.y() << ';'
            << crop.width() << ',' << crop.height() << "),Z("
       << wf.zoom_ << "),M("
       << wf.move_.x() << ',' << wf.move_.y() << ")]\n";
    return os;
}



Transformable::Transformable(const ImageFile &fn) : img_(), cache_(), wbox_(), txfs_()
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
    txfs_.move_ = point;
    return oldbox | wbox_;
}

QRect Transformable::zoom_to(float g)
{
    txfs_.zoom_ = g;
    if(cache_.isNull())
        cache_ = img_.copy();
    QSize target{cache_.size()};
    target.setHeight(target.height() * g + 0.99f );
    target.setWidth(target.width() * g + 0.99f );
    img_ = cache_.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QRect oldbox{wbox_};
    // FIXME allow zooming around centre or mouse point

    wbox_.setSize(img_.size());
    // One box will be larger than the other depending on whether we zoom in or out
    return oldbox | wbox_;
}

QRect Transformable::crop(QRect c)
{
    txfs_.crop_.adjust(c.x(), c.y(), 0, 0);
    txfs_.crop_.setSize(c.size());
    img_ = img_.copy(c);
    cache_ = QPixmap();

    QRect oldbox{wbox_};
    // crop the current image
    wbox_.adjust(c.x(), c.y(), 0, 0);
    img_ = img_.copy(c);
    // Update the transform by unzooming the current crop instructions
    auto z = txfs_.zoom_;
    c.setSize(c.size() / z);
    // topleft is local - relative to the current image
    c.setTopLeft(c.topLeft() / z);
    txfs_.crop_ = c;
    // Since we crop within the image oldbox should always be the larger
    return oldbox;
}


void Transformable::copy_from(const Transformable &orig)
{
    img_ = orig.img_.copy();
    wbox_ = orig.wbox_;
    txfs_ = orig.txfs_;
}

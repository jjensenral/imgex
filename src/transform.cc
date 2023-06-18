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
    dec.to_transform(*this, txfs_);
}



void
Transformable::run()
{
    // FIXME
}


QRect Transformable::move_to(qpoint<xwindow> point)
{
    QRect oldbox{wbox_.asQRect()};
    QRect newbox{point, oldbox.size()};
    txfs_.move_ = point;
    wbox_ = newbox;
    return oldbox | newbox;
}

QRect Transformable::zoom_to(float g)
{
    txfs_.zoom_ = g;
    if(cache_.isNull())
        cache_ = img_.copy();
    QSize target = zoom_box(g);
    img_ = cache_.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    qbox<xwindow> oldbox{wbox_};
    // FIXME allow zooming around centre or mouse point
    QSize offset = (target - wbox_.size())/2;
    wbox_.setSize(target);
    // As the image grows/shrinks, shift top left accordingly
    //move_to(QPoint(wbox_.x()+offset.width(), wbox_.y()+offset.height()));
    // One box will be larger than the other depending on whether we zoom in or out
    return (oldbox | wbox_).asQRect();
}

QRect Transformable::crop(QRect c)
{
    // Note that c comes in local coordinates
    img_ = img_.copy(c);

    QRect oldbox{wbox_.asQRect()}; // note global coordinates (top left rel to parent window)
    // Shift display box so the result image is in the box it was selected from
    wbox_.move(c.x(), c.y());
    wbox_.setSize(c.size());
    // crop the current pre-zoom image as well
    if(txfs_.has_zoom()) {
        // Update the pre-zoomed image by unzooming the current crop instructions
        auto z = txfs_.zoom_;
        c.setSize(c.size() / z);
        // topleft is local - relative to the current image
        c.setTopLeft(c.topLeft() / z);
        if(!cache_.isNull())
            cache_ = cache_.copy(c);
    }
    txfs_.crop_.adjust(c.x(), c.y(), 0, 0);
    txfs_.crop_.setSize(c.size());

    // Since we crop within the image oldbox should always be the larger
    return oldbox;
}


void Transformable::copy_from(const Transformable &orig)
{
    img_ = orig.img_.copy();
    wbox_ = orig.wbox_;
    txfs_ = orig.txfs_;
    cache_ = QPixmap();
}

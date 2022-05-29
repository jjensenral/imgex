/** Implementation of decorators */


#include "decor.hh"
#include "transform.hh"
#include <iostream>		// debug
#include <QMouseEvent>
#include <QPainter>


XILDecorator::event_status_t
XILDecorator::event_status_or(event_status_t a, event_status_t b) noexcept
{
    // everything takes precedence over NOP
    if(b == event_status_t::EV_NOP) return a;
    if(a == event_status_t::EV_NOP) return b;
    // DELME takes precedence over everything else
    if(a == event_status_t::EV_DELME || b == event_status_t::EV_DELME)
        return event_status_t::EV_DELME;
    // REDRAW takes precedence over DONE
    if(a == event_status_t::EV_REDRAW || b == event_status_t::EV_REDRAW)
        return event_status_t::EV_REDRAW;
    return event_status_t::EV_DONE;
}



XILDecorator::event_status_t
XILMouseEventDecorator::handleEvent(QEvent &qev)
{
    XILDecorator::event_status_t ret = event_status_t::EV_NOP;
	switch(qev.type()) {
	case QEvent::MouseButtonPress:
		ret = mPress(dynamic_cast<QMouseEvent&>(qev));
		qev.accept();
		break;
	case QEvent::MouseButtonRelease:
		ret = mRelease(dynamic_cast<QMouseEvent&>(qev));
		qev.accept();
		break;
	case QEvent::MouseMove:
		ret = mMove(dynamic_cast<QMouseEvent&>(qev));
		qev.accept();
		break;
	}
    return ret;
}



XILDecorator::event_status_t
XILMouseEventDecorator::mPress(QMouseEvent const &ev)
{
	switch(ev.button()) {
	case Qt::LeftButton:
		track_ = true;
        oldq_ = ev.pos();
		return event_status_t::EV_DONE;
	}
    return event_status_t::EV_NOP;
}


XILDecorator::event_status_t
XILMouseEventDecorator::mRelease(QMouseEvent const &ev)
{
	switch(ev.button()) {
	case Qt::LeftButton:
		track_ = false;
		return event_status_t::EV_DONE;
	}
    return event_status_t::EV_NOP;
}



XILDecorator::event_status_t
XILMouseEventDecorator::mMove(QMouseEvent const &ev)
{
	if(track_) {
		QPoint loc = ev.pos();
        delta = loc-oldq_;
        oldq_ = loc;
        owner_->mkexpose();
        return event_status_t::EV_DONE;
	}
    return event_status_t::EV_NOP;
}


XILDecorator::event_status_t
XILCropDecorator::handleEvent(QEvent &qev) {
    event_status_t ret = XILMouseEventDecorator::handleEvent(qev);
    return ret;
}


XILDecorator::event_status_t
XILCropDecorator::mPress(const QMouseEvent &qev)
{
    event_status_t ret = event_status_t::EV_NOP;
    if(qev.button() == Qt::LeftButton) {
        QPoint delta = qev.pos() - crop_.center();
        // we move whichever corner is closest, relative to the current crop rectangle
        bool top = delta.y() < 0;
        if(delta.x() < 0)
            corner_ = top ? corner_t::NW : corner_t::SW;
        else
            corner_ = top ? corner_t::NE : corner_t::SE;
        ret = event_status_t::EV_REDRAW;
    }
     return event_status_or(ret, XILMouseEventDecorator::mPress(qev));
}


XILDecorator::event_status_t
XILCropDecorator::mRelease(const QMouseEvent &qev)
{
    event_status_t ret = event_status_t::EV_NOP;
    if(qev.button() == Qt::LeftButton) {
        corner_ = corner_t::NONE;
        ret = event_status_t::EV_DONE;
    }
    return event_status_or(ret, XILMouseEventDecorator::mRelease(qev));
}


XILDecorator::event_status_t
XILCropDecorator::mMove(const QMouseEvent &qev)
{
    event_status_t ret = event_status_t::EV_NOP;
    if(track_) {
        XILMouseEventDecorator::mMove(qev); // calculates delta
        switch(corner_) {
            case corner_t::NW:
                crop_.setTopLeft(crop_.topLeft()+delta);
                break;
            case corner_t::NE:
                crop_.setTopRight(crop_.topRight()+delta);
                break;
            case corner_t::SW:
                crop_.setBottomLeft(crop_.bottomLeft()+delta);
                break;
            case corner_t::SE:
                crop_.setBottomRight(crop_.bottomRight()+delta);
                break;
            case corner_t::NONE: ;
        }
        crop_ = crop_.normalized();
        ret = event_status_t::EV_REDRAW;
    }
    return event_status_or(ret, XILMouseEventDecorator::mMove(qev));
}


void
XILCropDecorator::render(QPainter &qp)
{
    if(crop_.isNull()) {
        crop_ = owner_->box();
        crop_.adjust(0, 0, -1, -1);
    }
    qp.save();
    qp.setPen(Qt::SolidLine);
    qp.setPen(Qt::white);
    qp.setCompositionMode(QPainter::CompositionMode_Difference);
    qp.drawRect(crop_);
    qp.restore();
}


transform *
XILCropDecorator::to_transform() const
{
    if(crop_.isNull())
        return new transform();
    return new tf_crop{crop_};
}

void
BorderDecorator::render(QPainter &qp)
{
	// default is parent box
	QRect box{box_.isNull() ? owner_->box() : box_};
	// Clip to parent
	box &= owner_->box();
	// Shrink to fit inside window
	box.adjust(0, 0, -1, -1);
	qp.save();
	qp.setPen(col_);
	qp.setBrush(Qt::NoBrush);
	qp.setBackgroundMode(Qt::TransparentMode);
	qp.drawRect(box);
	qp.restore();
}

/** Implementation of decorators */


#include "decor.hh"
#include "transform.hh"
#include <iostream>		// debug
#include <QMouseEvent>
#include <QPainter>


XILDecorator::event_status_t
XILMouseEventDecorator::handleEvent(QEvent &qev)
{
	switch(qev.type()) {
	case QEvent::MouseButtonPress:
		mPress(dynamic_cast<QMouseEvent&>(qev));
		qev.accept();
		break;
	case QEvent::MouseButtonRelease:
		mRelease(dynamic_cast<QMouseEvent&>(qev));
		qev.accept();
		break;
	case QEvent::MouseMove:
		mMove(dynamic_cast<QMouseEvent&>(qev));
		qev.accept();
		break;
	}
}



void
XILMouseEventDecorator::mPress(QMouseEvent const &ev)
{
	switch(ev.button()) {
	case Qt::LeftButton:
		track_ = true;
		break;
	}
}


void
XILMouseEventDecorator::mRelease(QMouseEvent const &ev)
{
	switch(ev.button()) {
	case Qt::LeftButton:
		track_ = false;
		break;
	}
}



void
XILMouseEventDecorator::mMove(QMouseEvent const &ev)
{
	if(track_) {
		loc = ev.pos();
	}
}


XILDecorator::event_status_t
XILCropDecorator::handleEvent(QEvent &qev) {
    event_status_t ret = XILMouseEventDecorator::handleEvent(qev);
    if(ret == event_status_t::EV_NOP) ret = event_status_t::EV_REDRAW;
    return ret;
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

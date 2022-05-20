/** Implementation of decorators */


#include "decor.hh"
#include <iostream>		// debug
#include <QMouseEvent>


XILDecorator::event_status_t
XILEventDecorator::handleEvent(QEvent &qev)
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
XILEventDecorator::mPress(QMouseEvent const &ev)
{
	switch(ev.button()) {
	case Qt::LeftButton:
		track_ = true;
		break;
	}
}


void
XILEventDecorator::mRelease(QMouseEvent const &ev)
{
	switch(ev.button()) {
	case Qt::LeftButton:
		track_ = false;
		break;
	}
}



void
XILEventDecorator::mMove(QMouseEvent const &ev)
{
	if(track_) {
		loc = ev.pos();
	}
}



void
BorderDecorator::render(QPainter &qp) const
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

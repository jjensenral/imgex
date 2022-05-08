/** Implementation of decorators */


#include "decor.hh"
#include <iostream>



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

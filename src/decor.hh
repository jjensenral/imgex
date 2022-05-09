#ifndef __IMGEX_DECOR_H
#define __IMGEX_DECOR_H

/** Decorators for XILImage (defined in xwin.hh)
 * The base class is also defined in xwin.hh
 */

#include <QPainter>
#include <QRect>
#include <QColor>
#include "xwin.hh"


class BorderDecorator : public XILDecorator {
private:
	QRect box_;
	QColor col_;
public:
	/** Create in local coordinates
	 * If created with a null box, will use its owner's border */
	BorderDecorator(QRect qr, QColor qc) : box_(qr), col_(qc) {}
	virtual void render(QPainter &) const override;
};


#endif

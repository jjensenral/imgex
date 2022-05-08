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
	XILImage const *owner_;
	QRect box_;
	QColor col_;
public:
	/** Create in local coordinates
	 * If created with a null box, will use its owner's border */
	BorderDecorator(XILImage const *owner, QRect qr, QColor qc): owner_(owner), box_(qr), col_(qc) {}
	virtual void render(QPainter &) const override;
};


#endif

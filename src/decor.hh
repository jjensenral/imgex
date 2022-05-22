#ifndef __IMGEX_DECOR_H
#define __IMGEX_DECOR_H

/** Decorators for XILImage (defined in xwin.hh)
 * The base class is also defined in xwin.hh
 */

#include <QPainter>
#include <QRect>
#include <QColor>
#include "xwin.hh"

/** Virtual class for decorators capturing mouse events */

class XILMouseEventDecorator : public XILDecorator {
private:
    bool track_;
    QPoint oldq_;
protected:
    QPoint loc;
    virtual void mPress(QMouseEvent const &);
    virtual void mRelease(QMouseEvent const &);
    virtual void mMove(QMouseEvent const &);
public:
    virtual event_status_t handleEvent(QEvent &) override;
};



/** Crop decorator */
class XILCropDecorator : public XILMouseEventDecorator {
private:
    QRect crop_;
public:
    virtual event_status_t handleEvent(QEvent &) override;

    virtual void render(QPainter &qp) override;
};

/** Simple decorator which draws a border around the image */

class BorderDecorator : public XILDecorator {
private:
	QRect box_;
	QColor col_;
public:
	/** Create in local coordinates
	 * If created with a null box, will use its owner's border */
	BorderDecorator(QRect qr, QColor qc) : box_(qr), col_(qc) {}
	virtual void render(QPainter &) override;
};


#endif

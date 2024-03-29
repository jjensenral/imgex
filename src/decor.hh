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
    QPoint oldq_;
protected:
    bool track_;
    QPoint delta;
    virtual event_status_t mPress(QMouseEvent const &);
    virtual event_status_t mRelease(QMouseEvent const &);
    virtual event_status_t mMove(QMouseEvent const &);
public:
    virtual event_status_t handleEvent(QEvent &) override;

    void to_transform(Transformable &image, Transformable::transform &transform) const override;
};



/** Crop decorator */
class XILCropDecorator : public XILMouseEventDecorator {
private:
    /** Area to crop in local (image) coordinates */
    QRect crop_;
    /** Track which corner we are moving (if any) */
    enum class corner_t { NONE, NW, NE, SW, SE } corner_;
protected:
    event_status_t mPress(QMouseEvent const &) override;
    event_status_t mMove(QMouseEvent const &) override;
    event_status_t mRelease(QMouseEvent const &) override;

public:
    virtual event_status_t handleEvent(QEvent &) override;

    virtual void render(QPainter &qp) override;

    void to_transform(Transformable &image, Transformable::transform &transform) const override;

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

    void to_transform(Transformable &image, Transformable::transform &transform) const override;
};


#endif

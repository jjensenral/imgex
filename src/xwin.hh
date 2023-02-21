/* Qt interface */

#ifndef __IMGEX_XWIN_H
#define __IMGEX_XWIN_H


#include <list>
#include <memory>
#ifdef __cpp_concepts
#include <concepts>
#endif

#include <QPaintDeviceWindow>
#include <QBackingStore>
#include <QPainter>
#include <QWindow>
#include <QRect>


#include "transform.hh"
#include "image.hh"


/**
 * There are four kinds of coordinates
 * 1. local coordinates within a single XILImage
 * 2. Coordinates within an XWindow that hosts the XILImage
 * 3. Coordinates in the screen, noting the user may have unmaximised/unfullscreened the XWindow
 * 4. Coordinates in what Qt calls the "virtual geometry" of the desktop which may be composed of multiple screens
 */

// The template is currently only used to disambiguate the type
template<typename KIND>
struct qpoint: public QPoint
{
    // Construct from QRect
    explicit qpoint<KIND>(QPoint const &q): QPoint(q) {}
    // Treating it as the common base type requires explicit call
    // QPoint asQPoint() const noexcept { return *this; }
};

// These are currently just dummy bookkeeping (typekeeping) types
struct xilimage {};
struct xwindow {};
struct screen {};
struct desktop {};


class XWindow;
class QScreen;
class Session;


#include <QGuiApplication>


/** Decorator for XILImage class */

class XILImage;

class XILDecorator {
protected:
	XILImage const *owner_;

	/* owner is set by XILImage::add_decorator
	* Constructor is protected to prevent people creating decorators outside
	* of XILImage::add_decorator */
	XILDecorator() : owner_(nullptr) {}
public:
	/** Render to a painter but don't flush */
	virtual void render(QPainter &) = 0;
	virtual ~XILDecorator() = default;

	/** Decorator event handler can pass status back to XILImage */
    enum class event_status_t { EV_DONE, EV_NOP, EV_DELME, EV_REDRAW };

    /** General utility for combining event status */
    // defined in decor.cc
    static event_status_t event_status_or(event_status_t, event_status_t) noexcept;

	/** Handle (or not) event */
	[[nodiscard]] virtual event_status_t handleEvent(QEvent &ev)
	{
		ev.setAccepted(false);
		return event_status_t::EV_NOP;
	}

    /** Some decorators present a transform visually.
     * By default a NOP transform is returned. */
    virtual void to_transform(Transformable &, Transformable::transform &) const = 0;

	/* Give XILImage permission to add owner */
	friend class XILImage;
};


class XILImage final : public QWindow, public Transformable {

 private:
    /** Reference to the image which we need to update with transformations etc */
    std::unique_ptr<Image> orig_;

	QBackingStore canvas_;

	/** Parent window */
	XWindow *parent_;

	/** Offset into the child window for dragging */
	QPoint loc;

	/** Whether this window is being moved, need to track movement */
	bool track_;
	/** Previous point for tracking, used by mouseMoveEvent */
	QPoint oldq_;
	/** Whether this window is focused */
	bool focused_;
	/** Whether to resize on zoom */
	bool resize_on_zoom_;

	/** scaling factor */
	float zoom_;

	QString name_;

    /** Storage for decorators (overlays) */
	std::list<XILDecorator *> decors_;

	/** Render the image in the window */
	void render();

	/** Optionally pass (mouse) event to Decorator
	 * \return true if event handled */
	bool decor_event(QEvent &);

    /* Rehome this XILImage
     * A XILImage is always(?) owned by an XWindow.
     * However, while moving with the mouse the user may move it over to another XWindow
     * in which case we need to transfer ownership and redraw.
     *
     * Note that Qt keeps sending events to the window in which the mouse was pressed until the
     * mouse button is released, even if the pointer leaves the window or moves into another.
     *
     * Moreover, a user might unmaximize XWindows and have them overlap each other or there may
     * be parts of the screen uncovered by an XWindow.
     * For the former we can choose to transfer ownership when the mouse moves into another covering
     * XWindow even though the image could remain in the covered XWindow.
     * For the latter, a XILImage should never be outside of an XWindow, inaccessible to the user,
     * so we can either undock it completely - it becomes an independent window - or reset back to
     * where the move started.
     *
     * The return value is boolean: true if we were rehomed (or attempted rehomed)
     */
    bool rehome_at(qpoint<desktop> q);

public:
	XILImage(XWindow &, std::unique_ptr<Image>, QString const &);
	~XILImage();
	XILImage(XILImage const &) = delete;
    // base class QImage has deleted move constructor
	XILImage(XILImage &&) = delete;
	XILImage &operator=(XILImage const &) = delete;
    XILImage &operator=(XILImage &&) = delete;

    /** (Re)copy orig to working copy */
    void copy_from(Transformable const &orig) override;

    QRect zoom_to(float) override;

    QRect crop(QRect rect) override;

    QRect move_to(QPoint point) override;

    /** (Re)run transform on current image */
    void run() override;

    /** Call clear */
	// void clear(Display *d, Window w) const { XClearArea(d, w, wbox_.x, wbox_.y, wbox_.h, wbox_.y, 0); }
	/** Does this image contain point x,y (as mapped on window) */
	bool contains(int x, int y) const noexcept { return wbox_.contains(x,y); }
	bool contains(QPoint p) const noexcept { return wbox_.contains(p); }
	/** Does this image intersect a given box? */
	bool intersects(xwParentBox const &b) const noexcept { return wbox_.intersects(b); }
	
	/** Bounding box in own coordinates */
	QRect box() const noexcept { return QRect(0, 0, wbox_.width(), wbox_.height()); }

    /** Create an expose event for the parent window */
    void mkexpose(xwParentBox const &) const;
    void mkexpose() const { mkexpose(wbox_); }

    /** Events, as defined by QWindow */
	/*
	void focusInEvent(QFocusEvent *) override { qWarning("  Focus %s", qPrintable(name_)); focused_ = true; };
	void focusOutEvent(QFocusEvent *) override { qWarning("Unfocus %s", qPrintable(name_)); focused_ = false; };
	*/
	void mousePressEvent(QMouseEvent *) override;
	void mouseReleaseEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void wheelEvent(QWheelEvent *) override;
	void exposeEvent(QExposeEvent *) override;

	void add_decorator(XILDecorator *dec)
	{
		dec->owner_ = this;
		decors_.push_back(dec);
	}

	friend class XWindow;
};



/** XWindow - full screen window to draw stuff on.
 * A Window will be full size, the size of the screen, if possible.
 * Images are moved around inside windows, or between windows.
 */
class XWindow final : public QWindow {
	/** List of all images, whether owned by this XWindow or not, lowest first */
	static std::list<std::shared_ptr<XILImage>> ximgs_;
	/** Return a ptr to image at x,y or nullptr if there isn't one
	 * Note this can't be const because it returns a pointer to a
	 * non-const XILImage
	 */
	XILImage *img_at(auto args...) noexcept;
	/** Background */
	QBackingStore qbs_;
    /** Reference to the session that we're part of */
    Session &ses_;
 public:
	XWindow(Session &ses, QScreen *scr = nullptr);
	XWindow(XWindow const &) = delete;
    // The move ctor is unsafe (probably) because XWindow inherits from QWindow
	XWindow(XWindow &&) = delete;
	XWindow &operator=(XWindow const &) = delete;
	XWindow &operator=(XWindow &&) = delete;
    ~XWindow() override;

    /** Make an image in this window */
	void mkimage(ImageFile const &, QString);
	/** Events may be received by the main window, in which case the event needs dispatching to the child window */
	void resizeEvent(QResizeEvent *) override;
	void mousePressEvent(QMouseEvent *) override;
	void mouseReleaseEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void wheelEvent(QWheelEvent *) override;
	void exposeEvent(QExposeEvent *) override;

    bool contains(QPoint p) const noexcept { return geometry().contains(p); }
    /** Call session to find another owner */
    XWindow *xwindow_at(qpoint<desktop> q);

    /** Redraw the whole window */
	void redraw(QRect);

    /** handle expose */
	void expose(QRect const &);
};


// Specialised coordinate types
// General definition.  TO comes first because the rest can be deduced from the arguments
template<typename TO, typename FROM, typename AUX>
#ifdef __cpp_concepts
requires std::same_as<AUX, XILImage> || std::same_as<AUX, XWindow>
#endif
qpoint<TO> convert(qpoint<FROM> const &, AUX const &);

#endif

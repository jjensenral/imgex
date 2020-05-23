/* Qt interface */

#ifndef __IMGEX_XWIN_H
#define __IMGEX_XWIN_H


#include <list>
#include <memory>

#include <QPaintDeviceWindow>
#include <QBackingStore>
#include <QWindow>
#include <QRect>


#include "common.hh"
#include "session.hh"


class XWindow;
class XMain;
class Image;			// defined in image.hh
class ImageFile;		// defined in image.hh



#include <QGuiApplication>




class XILImage final : public QWindow, public Transformable {
 private:
	/** placement on window; width and height equivalent to the original image size times scale */
	QRect wbox_;

	QBackingStore canvas_;
	/** keep the original image and a working copy */
	QPixmap work_, orig_;

	/** Base coordinates for tracking movement, relative to root window */
	int locX, locY;
	/** Whether this window is being moved, need to track movement */
	bool track_;
	/** Whether this window is focused */
	bool focused_;
	/** Whether to resize on zoom */
	bool resize_on_zoom_;

	/** scaling factor */
	float zoom_;
	/** (Re)copy orig to working copy */
	void workCopy();

	/** Render the image in the window */
	void render();

	/** Move window */
	void moveto(int x, int y) noexcept
	{
		wbox_.setX(x); wbox_.setY(y);
		setPosition(x, y);
	}

	/** Resize window and image to size relative to original image
	 *  \param bool whether to resize the window or just the work copy.
	*/
	void resize(bool);

	/** Entry point for being visited by a transform */
	void apply(transform const &) override;

public:
	XILImage(XWindow &, Image const &);
	~XILImage() noexcept;
	XILImage(XILImage const &) = delete;
	XILImage(XILImage &&) = default;
	XILImage &operator=(XILImage const &) = delete;

	/** Call clear */
	// void clear(Display *d, Window w) const { XClearArea(d, w, wbox_.x, wbox_.y, wbox_.h, wbox_.y, 0); }
	/** Does this image contain point x,y (as mapped on window) */
	bool contains(int x, int y) const noexcept { return wbox_.contains(x,y); }
	/** Does this image intersect a given box? */
	bool intersects(QRect const &b) const noexcept { return wbox_.intersects(b); }
	
	/** Events, as defined by QWindow */
	void focusInEvent(QFocusEvent *) override { focused_ = true; };
	void focusOutEvent(QFocusEvent *) override { focused_ = false; };
	void mousePressEvent(QMouseEvent *) override;
	void mouseReleaseEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void wheelEvent(QWheelEvent *) override;

	friend class XWindow;
};



/** XWindow - full screen window to draw stuff on.
 * A Window will be full size, the size of the screen, if possible.
 * Images are moved around inside windows, or between windows.
 */
class XWindow final : public QWindow {
	/** List of images, lowest first */
	std::list<XILImage> ximgs_;
	// typedef std::list<XILImage>::iterator XILImageItr;
	/** Return a ptr to image at x,y or nullptr if there isn't one */
	XILImage *img_at(int, int) noexcept;
	/** Tracking image (across events) */
 public:
	XWindow();
	~XWindow() noexcept;
	XWindow(XWindow const &) = delete;
	XWindow(XWindow &&) = default;
	XWindow &operator=(XWindow const &) = delete;
	XWindow &operator=(XWindow &&) = default;

	/** Make an image in this window */
	void mkimage(ImageFile const &);
	/** Process event */
	void exposeEvent(QExposeEvent *) override;
	void mousePressEvent(QMouseEvent *) override;

	/** handle expose */
	void expose(QRect const &);
	friend class XMain;
};


#endif

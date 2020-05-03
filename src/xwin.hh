/* X11 and devIL interfaces */

#ifndef __IMGEX_XWIN_H
#define __IMGEX_XWIN_H


#include <list>
#include <string>


#define ILUT_USE_X11

#include <X11/X.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>


#include "common.hh"
#include "session.hh"


class XWindow;
class XMain;
class Image;			// defined in image.hh
class ImageFile;		// defined in image.hh



/** Base class for X Window; all resources set up by XWindow */
class EventProcessor {
protected:
	/** Display, set up by XWindow, and the same for all instances of
		XWindow */
	static Display *disp_;

	std::string name_;
	/** X identifier of window (XWindow) or subwindow (XILImage).
	 * Note that in either case, the parent XWindow is responsible for
	 * bot creating and destroying the window as an X resource.
	 */
	Window win_;
	GC gc_;

public:
	EventProcessor(std::string const &name) : name_(name), win_(0), gc_{} { }
	EventProcessor(EventProcessor const &) = delete;
	EventProcessor(EventProcessor &&) = default;
	EventProcessor &operator=(EventProcessor const &) = delete;
	virtual ~EventProcessor() noexcept = default;

	/** Process event, returning bool if exiting */
	[[nodiscard]]
	virtual bool process(XEvent const &) = 0;

	/** Who, me? */
	constexpr bool who_me(XEvent const &ev) const noexcept
	{
		return ev.xany.window == win_;
	}

	/** Name, primarily for troubleshooting */
	virtual std::string const &get_name() const noexcept { return name_; }
};


/** XRoot
 * \brief All the classic X stuff doing RAII now
 */
class XRoot final {
 private:
	Display *disp_;
	int screen_;
	int scr_w_, scr_h_;
	Colormap cm_;
	/** Foreground, background, border */
	XColor fg_, bg_, bd_;
 public:
	XRoot(char const *display);
	~XRoot() noexcept;
	XRoot(XRoot const &) = delete;
	XRoot(XRoot &&) = delete;
	XRoot &operator=(XRoot const &) = delete;
	friend class XWindow;
	friend class XMain;
};



class XILImage final : public EventProcessor {
 private:
	/** placement on window; width and height equivalent to the original image size times scale */
	box wbox_;
	/** Pointer to X Window System's version of the image */
	XImage *img_;
	/** IL keeps the original image and a working copy */
	ILuint work_, orig_;

	/** Base coordinates for tracking movement, relative to root window */
	coord locX, locY;
	/** Whether this window is being moved, need to track movement */
	bool track_;

	/** scaling factor */
	float zoom_;
	/** (Re)copy orig to working copy */
	void workCopy();
	/** (Re)load original copy from Image */
	box loadOrig(Image const &);

	/** Move window */
	void moveto(coord x, coord y) noexcept
	{
		wbox_.x = x; wbox_.y = y;
		XMoveWindow(disp_, win_, x, y);
	}

	/** Resize window and image to size relative to original image */
	void resize();

public:
	XILImage(XWindow &, Image const &);
	~XILImage() noexcept;
	XILImage(XILImage const &) = delete;
	XILImage(XILImage &&) = default;
	XILImage &operator=(XILImage const &) = delete;

	/** Call clear */
	void clear(Display *d, Window w) const { XClearArea(d, w, wbox_.x, wbox_.y, wbox_.h, wbox_.y, 0); }
	/** Does this image contain point x,y (as mapped on window) */
	constexpr bool contains(coord x, coord y) const noexcept { return wbox_.contains(x,y); }
	/** Does this image intersect a given box? */
	constexpr bool intersects(box const &b) const noexcept { return wbox_.intersects(b); }
	
	/** Process event */
	bool process(XEvent const &) override;

	/** Application of transformation */
	void apply( transform const & );
	/** Or a list of transformations */
	void apply( workflow const & );
	/** Map or move window */
	void map();
};



/** XWindow - full screen window to draw stuff on.
 * A Window will be full size, the size of the screen, if possible.
 * Images are moved around inside windows, or between windows.
 */
class XWindow final : public EventProcessor {
	XRoot const &xroot_;
	/** List of images, lowest first */
	std::list<XILImage> ximgs_;
	// typedef std::list<XILImage>::iterator XILImageItr;
	/** Return a ptr to image at x,y or nullptr if there isn't one */
	XILImage *img_at(coord, coord) noexcept;
	/** Tracking image (across events) */
 public:
	XWindow(XRoot const &);
	~XWindow() noexcept;
	XWindow(XWindow const &) = delete;
	XWindow(XWindow &&) = default;
	XWindow &operator=(XWindow const &) = delete;
	Window window() const noexcept { return win_; }

	constexpr GC get_gc() noexcept { return gc_; }

	/** Process event */
	bool process(XEvent const &) override;
	/** Return the recipient of this event, or nullptr if not one of ours */
	EventProcessor *recipient(XEvent const &) noexcept;

	/** Make a subwindow for an image */
	Window mksubwin(box const &);
	/** handle expose */
	void expose(box const &);
	friend class XMain;
};



/** XMain
 * Manages creation and link between XRoot and XWindow(s)
 */
class XMain final {
 private:
	XRoot xroot_;
	/* List of Windows on this display/screen.  Needs to be a list because
	 * of restriced copy semantics of XWindow.
	 * Most of the time there will be only one window */
	std::list<XWindow> wins_;
	/* Provide at function for list; throws out_of_range */
	XWindow &at(int);
 public:
	XMain(char const *display);
	~XMain();
	/* Windows are numbered from 0 upwards */
	int createWindow() { wins_.emplace_back(xroot_); return wins_.size()-1; }
	void mkimage(int, ImageFile const &);
	void flush();
	void run();
};

#endif

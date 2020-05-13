#include "xwin.hh"
#include "image.hh"
#include <iterator>
#include <exception>
#include <algorithm>




/** This value will be set by XWindow (and will be the same for any
	XWindow) */
Display *EventProcessor::disp_ = nullptr;


XRoot::XRoot(char const *display): disp_(XOpenDisplay(display))
{
	if(!disp_)
		throw "couldn't open display";

	screen_ = DefaultScreen(disp_);
	cm_ = DefaultColormap(disp_, screen_);

	if( XParseColor(disp_, cm_, "Red", &bd_) == 0 \
		|| XParseColor(disp_, cm_, "Black", &fg_) == 0 \
		|| XParseColor(disp_, cm_, "Black", &bg_) == 0
		|| XAllocColor(disp_, cm_, &bd_) == 0 \
		|| XAllocColor(disp_, cm_, &fg_) == 0 \
		|| XAllocColor(disp_, cm_, &bg_) == 0) {
		throw "Failed to allocate colours";
	}
	
	scr_w_ = DisplayWidth(disp_, screen_);
	scr_h_ = DisplayHeight(disp_, screen_);
	
	/*
	scr_w_ = WidthOfScreen(screen_);
	scr_h_ = HeightOfScreen(screen_);
	*/
	ilInit();
	iluInit();
	ilutInit();
}


XRoot::~XRoot() {
	XCloseDisplay(disp_);
	disp_ = nullptr;
}


XILImage::XILImage(XWindow &xw, Image const &img) : EventProcessor(img.getFilename()),
							    wbox_(), img_(nullptr), work_(0),
							    orig_(0), locX(0), locY(0), track_(false),
							    zoom_(1.0)
{
	// disp_ is shared through base class
	gc_ = xw.get_gc();
	ILuint ids[2];		// guaranteed consecutive
	ilGenImages(2, ids);
	work_ = ids[0]; orig_ = ids[1];
	wbox_ = loadOrig(img);
	win_ = xw.mksubwin(wbox_);
	img_ = nullptr;
	// workCopy (re)sets wbox and sets img_
	workCopy();
}


XILImage::~XILImage()
{
	if(img_)
		XDestroyImage(img_);
	// The windows need to be destroyed by the parent
	//    XDestroyWindow(disp_, win_);
	img_ = nullptr;
}


box
XILImage::loadOrig(Image const &img)
{
	ilBindImage(orig_);
	if(!ilLoadImage(img.getFilename().c_str())) {
		throw "Failed to load " + img.getFilename();
	}
	// TODO: First set of transforms apply to orig
	// Then define the box
	box b;
	b.w = static_cast<decltype(b.w)>(ilGetInteger(IL_IMAGE_WIDTH));
	b.h = static_cast<decltype(b.h)>(ilGetInteger(IL_IMAGE_HEIGHT));
	return b;
}


void
XILImage::workCopy()
{
	ilBindImage(work_);
	ilCopyImage(orig_);
	wbox_.w = static_cast<decltype(wbox_.w)>(ilGetInteger(IL_IMAGE_WIDTH));
	wbox_.h = static_cast<decltype(wbox_.h)>(ilGetInteger(IL_IMAGE_HEIGHT));
	if(img_) XDestroyImage(img_);
	img_ = nullptr;
}


void
XILImage::map()
{
	if(!img_) {
		ilBindImage(work_);
		img_ = ilutXCreateImage(disp_);
		if(!img_)
		    throw "Cannot create X Image";
	}
	//    XClearArea(disp, win, wbox_.x, wbox_.y, wbox_.w, wbox_.h, 0);
	XPutImage(disp_, win_, gc_, img_,
		      0, 0,		// src x and y
		      0, 0,		// dst x and y relative to own subwindow
		      wbox_.w, wbox_.h);
	XFlush(disp_);
}


void
XILImage::resize()
{
	workCopy();			// deletes img_ and resets work copy and wbox
	int depth{ilGetInteger(IL_IMAGE_DEPTH)};
	decltype(wbox_.w) width{static_cast<decltype(wbox_.w)>(static_cast<float>(wbox_.w) * zoom_+0.999f)};
	decltype(wbox_.h) height{static_cast<decltype(wbox_.h)>(static_cast<float>(wbox_.h) * zoom_+0.999f)};
	if(	iluScale(width, height, depth) ) {
		XResizeWindow(disp_, win_, width, height);
		try {
		    map();
		} catch(char const *) {
		    // ignore for now, leaving window blank
		    XFillRectangle(disp_, win_, gc_, 0, 0, width, height);
		    return;
		}
		wbox_.w = width;
		wbox_.h = height;
	}
	return;
}


void
XILImage::apply(transform const &)
{
}


bool
XILImage::process(XEvent const &ev)
{
	//fprintf(stderr, "process %s, %d\n", name_.c_str(), ev.type);
	bool need_resize = false;
	switch(ev.type) {
	case MotionNotify:
		moveto(ev.xmotion.x_root - locX, ev.xmotion.y_root - locY);
		break;
	case Expose:
		XClearArea(disp_, win_,
			   ev.xexpose.x, ev.xexpose.y,
			   ev.xexpose.width, ev.xexpose.height, 0);
		XPutImage(disp_, win_, gc_, img_,
			  0, 0,		// src x and y
			  0, 0,		// dst x and y relative to own subwindow
			  wbox_.w, wbox_.h);
		break;
	case ButtonPress:
		switch(ev.xbutton.button) {
		case Button1:
		    locX = ev.xbutton.x_root - wbox_.x;
		    locY = ev.xbutton.y_root - wbox_.y;
		    break;
		case Button4:
		    /* Button 4 is mouse wheel forward - zoom in */
		    zoom_ *= 1.1;
		    need_resize = true;
		    break;
		case Button5:
		    /* Button 5 is mouse wheel backward - zoom out */
		    zoom_ /= 1.1;
		    need_resize = true;
		    break;
		case Button2:
		    /* Button 2 is pressing middle mouse button/wheel */
		    if(zoom_ != 1.0) {
			zoom_ = 1.0;
			need_resize = true;
		    }
		    break;
		case Button3:
		    /* Button 3 is right mouse button (or left if you have a left handed mouse) */
		    return true;
		}
		break;
	case ButtonRelease:
		track_ = false;
	}
	if(need_resize) {
		resize();
	}
	return false;
}



XWindow::XWindow(XRoot const &xr) : EventProcessor("XWindow"), xroot_(xr)
{
	disp_ = xr.disp_;		// this value is always the same
	win_ = XCreateSimpleWindow(xr.disp_,
				       DefaultRootWindow(xr.disp_),
				       0, 0,
				       xr.scr_w_, xr.scr_h_,
				       0, // default border width
				       xr.bg_.pixel, xr.bg_.pixel);
	if(!win_)
		throw "Failed to create window";
	// XSizeHints xsh;
	XGCValues xgcv;
	xgcv.foreground = xr.fg_.pixel;
	xgcv.background = xr.bg_.pixel;
	gc_ = XCreateGC( xr.disp_, win_, GCForeground | GCBackground, &xgcv );
	if(!gc_) {
		XDestroyWindow(xr.disp_, win_);
		throw "Failed to create graphics context";
	}
	XSetWindowAttributes xswa;
	xswa.colormap = xr.cm_;
	xswa.bit_gravity = NorthWestGravity;
	XChangeWindowAttributes(xr.disp_, win_, CWColormap|CWBitGravity, &xswa);

	//    XSelectInput(xr.disp_, win_, ExposureMask|ButtonPressMask|Button1MotionMask);
	XSelectInput(xr.disp_, win_, ExposureMask);
	XMapWindow(xr.disp_, win_);
	XFlush(xr.disp_);
}


XWindow::~XWindow()
{
	XFreeGC(xroot_.disp_, gc_);
	XDestroySubwindows(xroot_.disp_, win_);
	XDestroyWindow(xroot_.disp_, win_);
}


XILImage *
XWindow::img_at(coord x, coord y) noexcept
{
	// We must find the last image that contains the point
	std::reverse_iterator<std::list<XILImage>::iterator>
		p = ximgs_.rbegin(), q = ximgs_.rend();
	// Annoyingly, std::find_if refuses to work with reverse iterators
	while( p != q ) {
		if( p->contains(x,y) ) return &(*p);
		++p;
	}
	return nullptr;
}


Window
XWindow::mksubwin(box const &b)
{
	Window w = XCreateSimpleWindow(xroot_.disp_, win_,
					   b.x, b.y, b.w, b.h,
					   1, xroot_.bd_.pixel, xroot_.bg_.pixel);
	XSelectInput(xroot_.disp_, w, ExposureMask|ButtonPressMask|Button1MotionMask);
	XMapWindow(xroot_.disp_, w);
	return w;
}


bool
XWindow::process(XEvent const &ev)
{
	//fprintf(stderr, "process XWindow, %d\n", ev.type);

	/* Variables used for tracking (while moving an image within a
	   window) - there is at most one tracking active at any given
	   time */
	bool need_resize = false;
	XILImage *which = nullptr;
	
	switch(ev.type) {
	case Expose:
		if( ev.xany.window == win_ ) { // main window
		    XClearArea(xroot_.disp_, win_,
			       ev.xexpose.x, ev.xexpose.y,			\
			       ev.xexpose.width, ev.xexpose.height, 0);
		    XFillRectangle(xroot_.disp_, win_, gc_,
				   ev.xexpose.x, ev.xexpose.y,			\
				   ev.xexpose.width, ev.xexpose.height);
		}
		break;
	}
	return false;
}



/*
void
XWindow::expose(box const &b)
{
	XClearArea(xroot_.disp_, win_, b.x, b.y, \
		       static_cast<int>(b.w), static_cast<int>(b.h), 0);
	//XClearArea(xroot_.disp_, win_, 0, 0, 0, 0, 0);
	if constexpr(true)
			    map_all();
	else
	std::for_each(ximgs_.rbegin(), ximgs_.rend(),
			  [this,&b](XILImage &x) { if(x.intersects(b)) x.map(*this); });
}
*/


EventProcessor *
XWindow::recipient(XEvent const &ev) noexcept
{
	if( who_me(ev) )
		return this;
	auto k = std::find_if(ximgs_.begin(), ximgs_.end(),
				  [&ev](XILImage const &x) { return x.who_me(ev); });
	if( k != ximgs_.end() )
		return &(*k);
	return nullptr;
}


XMain::XMain(char const *display): xroot_(display), wins_()
{
}


 XMain::~XMain()
{
}


void
XMain::mkimage(int no, ImageFile const &fn)
{
	// can't happen?
	if(wins_.empty())
		throw "No windows defined";
	const Image img(fn);
	XWindow &w = at(no);
	// as of C++17, emplace_back returns the reference
	XILImage &xim = w.ximgs_.emplace_back(w, img);
	xim.map();
}


void
XMain::flush()
{
	XFlush(xroot_.disp_);
}


XWindow &
XMain::at(int k)
{
	std::list<XWindow>::iterator p = wins_.begin(), q = wins_.end();
	while(k-- && p++ != q);    // doesn't matter if we decrement below zero
	if(p == q)
		throw std::out_of_range("XMain::at");
	return *p;
}


void
XMain::run()
{
	// Start an event processing loop
	bool done = false;
	while(!done) {
		XEvent ev;
		XNextEvent(xroot_.disp_, &ev);
		for( EventProcessor *who; auto &win : wins_ ) {
		    who = win.recipient(ev);
		    if(who) {
			done |= who->process(ev);
			break;
		    }
		}
	}
}

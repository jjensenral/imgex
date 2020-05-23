#include "xwin.hh"
#include "image.hh"
#include <iterator>
#include <exception>
#include <algorithm>
#include <memory>

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QString>
#include <QSize>
#include <QMouseEvent>
#include <QWheelEvent>



XILImage::XILImage(XWindow &xw, Image const &img) : QWindow(&xw), Transformable(),
							    wbox_(), canvas_(this), work_(), orig_(img.getFilename()),
							    locX(0), locY(0), track_(false), focused_(false),
								resize_on_zoom_(true),
							    zoom_(1.0f)
{
	// TODO: Apply any processing to orig_
	wbox_ = orig_.rect();
	qWarning("Loaded %s geometry (%d,%d,%d,%d)", qPrintable(img.getFilename()),wbox_.x(),wbox_.y(),wbox_.width(),wbox_.height());
	setGeometry(orig_.rect());
	canvas_.resize(orig_.size());
	// workCopy (re)sets wbox and sets img_
	workCopy();
	show();
}


XILImage::~XILImage()
{
}


void
XILImage::workCopy()
{
	work_ = orig_.copy();
	wbox_ = work_.rect();
}


void
XILImage::render()
{
	if(!isExposed())
		return;
	canvas_.beginPaint(wbox_);
	QPaintDevice *pd = canvas_.paintDevice();
	if(!pd) {
		qWarning("Unable to get paint device");
		canvas_.endPaint();
		return;
	}
	// To call, we need an old fashioned C pointer
	QPainter p(pd);
	p.drawPixmap(wbox_.x(), wbox_.y(), work_);
	p.end();
	canvas_.endPaint();				// also frees pd
	canvas_.flush(wbox_, this);
}


void
XILImage::resize(bool resize_window)
{
	workCopy();			// resets work copy and wbox
	// Target size
	QSize size{ wbox_.size() };
	size.setHeight( size.height() * zoom_ + 0.99f );
	size.setWidth( size.width() * zoom_ + 0.99f );
	work_.scaled(wbox_.size(), Qt::KeepAspectRatio);
	wbox_.setSize( work_.size() );
	if( resize_window ) {
		QWindow::resize(wbox_.size());
		canvas_.resize(wbox_.size());
	}
	render();
}


void
XILImage::apply(transform const &)
{
}


void
XILImage::mousePressEvent(QMouseEvent *ev)
{
	switch(ev->button()) {
	case Qt::LeftButton:
	    locX = ev->globalX() - wbox_.x();
	    locY = ev->globalY() - wbox_.y();
		track_ = true;
	    break;
	case Qt::MiddleButton:
		zoom_ = 1.0f;
		resize(resize_on_zoom_);
		break;
	case Qt::RightButton:
		break;
	default:
		break;
	}
}


void
XILImage::mouseReleaseEvent(QMouseEvent *ev)
{
	switch(ev->button()) {
	case Qt::LeftButton:
		track_ = false;
	default:
		break;
	}
}


void
XILImage::mouseMoveEvent(QMouseEvent *)
{
	//if(track_) 
}


void
XILImage::wheelEvent(QWheelEvent *ev)
{
	if(focused_) {
		if(ev->angleDelta().y() > 0)
			zoom_ /= 1.1;
		else
			zoom_ *= 1.1;
		resize(resize_on_zoom_);
	}
}




XWindow::XWindow() : QWindow(static_cast<QWindow *>(nullptr))
{
	//showMaximized();
}


XWindow::~XWindow()
{
}


XILImage *
XWindow::img_at(int x, int y) noexcept
{
	// We must find the last (highest in stack) image that contains the point
	std::reverse_iterator<std::list<XILImage>::iterator>
		p = ximgs_.rbegin(), q = ximgs_.rend();
	// Annoyingly, std::find_if refuses to work with reverse iterators
	while( p != q ) {
		if( p->contains(x,y) ) return &(*p);
		++p;
	}
	return nullptr;
}


void
XWindow::exposeEvent(QExposeEvent *)
{
	for( XILImage &x : ximgs_ )
		x.render();
}


void
XWindow::mousePressEvent(QMouseEvent *)
{}


/*
bool
XWindow::process(XEvent const &ev)
{
	//fprintf(stderr, "process XWindow, %d\n", ev.type);

	// Variables used for tracking (while moving an image within a
	// window) - there is at most one tracking active at any given
	// time
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
*/


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



void
XWindow::mkimage(ImageFile const &fn)
{
	const Image img(fn);
	ximgs_.emplace_back(*this, img);
}

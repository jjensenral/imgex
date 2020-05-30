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



XILImage::XILImage(XWindow &xw, Image const &img, QString name) : QWindow(&xw), Transformable(),
							    wbox_(), canvas_(this), work_(), orig_(img.getFilename()),
							    parent_(&xw), loc(0,0), track_(false), focused_(false),
								resize_on_zoom_(true),
							    zoom_(1.0f), name_(name)
{
	// TODO: Apply any processing to orig_
	wbox_ = orig_.rect();
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
XILImage::render(QRect reg)
{
	if(!isExposed())
		return;
	if(reg.isNull())
		reg = wbox_;
	else if(!wbox_.intersects(reg))
		return;
	canvas_.beginPaint(reg);
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
	render(wbox_);
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
	// locX = ev.xbutton.x_root - wbox_.x; locY = ev.xbutton.y_root - wbox_.y;
	    loc = ev->globalPos() - wbox_.topLeft();
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
XILImage::mouseMoveEvent(QMouseEvent *ev)
{
	// moveto(ev.xmotion.x_root - locX, ev.xmotion.y_root - locY);
	// Only move if we are in track mode and docked with the main window
	if(track_ && !isTopLevel()) {
		auto q = ev->globalPos() - loc;
		setPosition(q);
		wbox_.setTopLeft(q);
	}
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


void
XILImage::exposeEvent(QExposeEvent *ev)
{
	render(ev->region().boundingRect());
}


XWindow::XWindow() : QWindow(static_cast<QWindow *>(nullptr))
{
	//showMaximized();
}


XWindow::~XWindow()
{
}


XILImage *
XWindow::img_at(auto args...) noexcept
{
	// We must find the last (highest in stack) image that contains the point
	std::reverse_iterator<std::list<XILImage>::iterator> p = ximgs_.rbegin(), q = ximgs_.rend();
	// Annoyingly, std::find_if refuses to work with reverse iterators
	while( p != q ) {
		if( p->contains(args) ) return &(*p);
		++p;
	}
	return nullptr;
}


void
XWindow::exposeEvent(QExposeEvent *ev)
{
	QRect bbox(ev->region().boundingRect());
	for( XILImage &x : ximgs_ )
		x.render(QRect());
}


void
XWindow::mousePressEvent(QMouseEvent *ev)
{
	XILImage *w = img_at(ev->globalPos());
	if(!w) return;
	w->mousePressEvent(ev);
}


void
XWindow::mouseReleaseEvent(QMouseEvent *ev)
{
	XILImage *w = img_at(ev->globalPos());
	if(!w) return;
	w->mouseReleaseEvent(ev);
}


void
XWindow::mouseMoveEvent(QMouseEvent *ev)
{
	XILImage *w = img_at(ev->globalPos());
	if(!w) return;
	w->mouseMoveEvent(ev);
}


void
XWindow::wheelEvent(QWheelEvent *ev)
{
	XILImage *w = img_at(ev->globalPos());
	if(!w) return;
	w->wheelEvent(ev);
}


void
XWindow::mkimage(ImageFile const &fn, QString name)
{
	const Image img(fn);
	ximgs_.emplace_back(*this, img, name);
}

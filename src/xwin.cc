#include "xwin.hh"
#include "image.hh"
#include "decor.hh"
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
#include <iostream>


XILImage::XILImage(XWindow &xw, std::unique_ptr<Image> img, QString const &name) : QWindow(&xw), Transformable(),
                                                                         img_(std::move(img)),
                                                                         // Note we take ownership of the Image and img is invalid from now on
                                                                         wbox_(), canvas_(this), work_(), orig_(img_->getFilename()),
                                                                         parent_(&xw), loc(0,0), track_(false), focused_(false),
                                                                         resize_on_zoom_(true),
                                                                         zoom_(1.0f), name_(name)
{
	// TODO: Apply any processing to orig
	wbox_ = orig_.rect();
	setGeometry(orig_.rect());
	canvas_.resize(orig_.size());
	// workCopy (re)sets wbox
	workCopy();
	show();
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
	QRect reg{0, 0, width(), height()};
	canvas_.beginPaint(reg);
	QPaintDevice *pd = canvas_.paintDevice();
	if(!pd) {
		qWarning("Unable to get paint device");
		canvas_.endPaint();
		return;
	}
	QPainter p(pd);
	p.drawPixmap(0, 0, work_);
	// shared painter properties for (presumably) all decorators
	p.setBrush(Qt::NoBrush);
	p.setBackgroundMode(Qt::TransparentMode);
	std::for_each(decors_.begin(), decors_.end(), [&p](XILDecorator *dr) { dr->render(p); });
	p.end();
	canvas_.endPaint();				// also frees pd
	canvas_.flush(reg, this);
}


void
XILImage::resize(QPoint const &, bool resize_window)
{
	workCopy();			// reset work copy
	// Resize around focus point, or centre?
//	QPoint focus{ c.isNull() ? work_.rect().center() : c };
	QPoint focus{ work_.rect().center() };

	// Target size
	QSize size{ wbox_.size() };
	size.setHeight( size.height() * zoom_ + 0.99f );
	size.setWidth( size.width() * zoom_ + 0.99f );
	QRect oldbox = wbox_;
	work_ = work_.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	wbox_.setSize( work_.size() );
	wbox_.moveCenter(focus);
	if( resize_window ) {
		QWindow::resize(wbox_.size());
		canvas_.resize(wbox_.size());
	}
	mkexpose( oldbox | wbox_ );
}


void
XILImage::mousePressEvent(QMouseEvent *ev)
{
    if(decor_event(*ev))
        return;
	// qWarning("XIL press %s %d", qPrintable(name_), ev->button());
	switch(ev->button()) {
	case Qt::LeftButton:
	// locX = ev.xbutton.x_root - wbox_.x; locY = ev.xbutton.y_root - wbox_.y;
		// Position is relative to the parent window, or the root if we have no parent
		loc = ev->globalPos();
		oldq_ = loc;
		loc -= wbox_.topLeft();
		if(QWindow *p = this->parent()) {
			loc -= p->position();
		}
		// Now find position relative to the top left corner
		loc -= wbox_.topLeft();
		track_ = true;
	    break;
	case Qt::MiddleButton:
		zoom_ = 1.0f;
		resize(ev->globalPos(), resize_on_zoom_);
		break;
	case Qt::RightButton:
        // XXX for now, just start or end the crop process
        if(decors_.empty()) {
            add_decorator(new XILCropDecorator());
        } else {
            // XXX assume it is the crop decorator and finalise it
            std::cerr << "RMB\n";
            XILDecorator *dec = decors_.front();
            Transformable::add_from_decorator(*dec);
            decors_.pop_front();
            delete dec;
        }
        mkexpose(wbox_);
		break;
	default:
		break;
	}
	QWindow::mousePressEvent(ev);
}


void
XILImage::mouseReleaseEvent(QMouseEvent *ev)
{
    if(decor_event(*ev))
        return;
	switch(ev->button()) {
	case Qt::LeftButton:
		track_ = false;
	default:
		break;
	}
	QWindow::mouseReleaseEvent(ev);
}


void
XILImage::mouseMoveEvent(QMouseEvent *ev)
{
    if(decor_event(*ev))
        return;
	// Only move if we are in track mode and docked with the main window
	if(track_) {
		xwParentBox from = parent_box();
		QPoint q{ev->globalPos()};
		QPoint delta{ q-oldq_ };
		oldq_ = q;
		QPoint newpos{ this->position() + delta };
		xwParentBox to{ newpos, from.size() };
		wbox_ = to;
		if(isTopLevel()) {
			// XXX untested
			setPosition(newpos);
		} else {
			// Current mouse position relative to parent window
			q -= this->parent()->position();
		}
		// Move the window to the new location
		setPosition(newpos);
		mkexpose( from | to );
	}
	QWindow::mouseMoveEvent(ev);
}


void
XILImage::wheelEvent(QWheelEvent *ev)
{
	// Area affected
	xwParentBox area = parent_box();

	// Wheel forward is zoom in (ie enlarge)
	if(ev->angleDelta().y() > 0)
		zoom_ *= 1.1;
	else
	// Wheel back is zoom out (ie shrink)
		zoom_ /= 1.1;
	// ev->globalPos() is deprecated in 5.15 at least
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    resize(ev->globalPosition().toPoint(), resize_on_zoom_);
#else
    resize(ev->globalPos(), resize_on_zoom_);
#endif
	xwParentBox new_area = parent_box();
	mkexpose( area | new_area );
	QWindow::wheelEvent(ev);
}


void
XILImage::exposeEvent(QExposeEvent *ev)
{
	//render(ev->region().boundingRect());
	//render();
	xwParentBox bbox = ev->region().boundingRect();
	mkexpose(bbox);
	QWindow::exposeEvent(ev);
}


void
XILImage::mkexpose(xwParentBox const &area) const
{
	QWindow *parent = this->parent();
	if(parent != nullptr) {
		// XXX can't fail
		dynamic_cast<XWindow *>(parent)->redraw(area);
	}
}


bool
XILImage::decor_event(QEvent &qev)
{
	// Ask decorators if they want the event with most recent first
	std::reverse_iterator<std::list<XILDecorator *>::iterator> p = decors_.rbegin(), q = decors_.rend();
	while(p != q) {
		switch((*p)->handleEvent(qev)) {
		case XILDecorator::event_status_t::EV_DELME:
			// Delete decorator and return
		case XILDecorator::event_status_t::EV_DONE:
			// Event handled
			return true;
		case XILDecorator::event_status_t::EV_NOP:
			break;
		case XILDecorator::event_status_t::EV_REDRAW:
			render();
			return true;
		}
        ++p;
	}
	return false;
}


XILImage::xwParentBox
XILImage::parent_box() const
{
	QSize sz(width(), height());
	xwParentBox bbox( geometry().topLeft(), sz);
	return bbox;
}


XWindow::XWindow() : QWindow(static_cast<QWindow *>(nullptr)), qbs_(this)
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
	// Annoyingly, std::find_if refuses to work with reverse iterator adaptors?
	while( p != q ) {
		if( p->contains(args) ) return &(*p);
		++p;
	}
	return nullptr;
}


void
XWindow::redraw(QRect area)
{
	QRect window(0, 0, width(), height());
	if(area.isNull())
		area = window;
	else
		area &= window;
	qbs_.beginPaint(area);
	QPaintDevice *dev = qbs_.paintDevice();
	if(dev) {
		QPainter qp(dev);
		qp.fillRect(area, QColor(0,0,0));
		qp.end();
		for( XILImage &x : ximgs_ )
			if(x.intersects(area))
				x.render();
	}
	qbs_.endPaint();
	qbs_.flush(area, this);
}


void
XWindow::exposeEvent(QExposeEvent *ev)
{
	if(!isExposed())
		return;
	QRect bbox(ev->region().boundingRect());
	redraw(bbox);
	QWindow::exposeEvent(ev);
}


void
XWindow::resizeEvent(QResizeEvent *ev)
{
	qbs_.resize(ev->size());
	QWindow::resizeEvent(ev);
}


void
XWindow::mousePressEvent(QMouseEvent *ev)
{
	XILImage *w = img_at(ev->globalPos());
	if(!w) return;
	w->mousePressEvent(ev);
	QWindow::mousePressEvent(ev);
}


void
XWindow::mouseReleaseEvent(QMouseEvent *ev)
{
	XILImage *w = img_at(ev->globalPos());
	if(!w) return;
	w->mouseReleaseEvent(ev);
	QWindow::mouseReleaseEvent(ev);
}


void
XWindow::mouseMoveEvent(QMouseEvent *ev)
{
	XILImage *w = img_at(ev->globalPos());
	if(!w) return;
	w->mouseMoveEvent(ev);
	QWindow::mouseMoveEvent(ev);
}


void
XWindow::wheelEvent(QWheelEvent *ev)
{
	XILImage *w =
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            img_at(ev->globalPosition().toPoint());
#else
            img_at(ev->globalPos());
#endif
	if(!w) return;
	w->wheelEvent(ev);
	QWindow::wheelEvent(ev);
}


void
XWindow::mkimage(ImageFile const &fn, QString name)
{
    auto img = std::make_unique<Image>(fn);
	ximgs_.emplace_back(*this, std::move(img), name);
	// test to add decorator to one image
	if(ximgs_.size() == 2)
	    ximgs_.back().add_decorator(new BorderDecorator(QRect(), Qt::red));
}

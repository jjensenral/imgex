#ifndef IMGEX_QBOX_HH
#define IMGEX_QBOX_HH

#include <QRect>


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
    qpoint<KIND>(): QPoint() {}
    explicit qpoint<KIND>(QPoint const &q): QPoint(q) {}
    // Treating it as the common base type requires explicit call
    // QPoint asQPoint() const noexcept { return *this; }
    void move(int dx, int dy) noexcept
    {
        setX(x()+dx);
        setY(y()+dy);
    }
};

// These are currently just dummy bookkeeping (typekeeping) types
struct xilimage {};
struct xwindow {};
struct screen {};
struct desktop {};

template<typename KIND>
struct qbox {
    qpoint<KIND> topl_;
    QSize size_;
public:
    qbox() noexcept: topl_(), size_() {}

    qbox(int x, int y, int w, int h) noexcept: topl_(x, y), size_(w, h) {}

    qbox(QPoint topl, QSize size) : topl_(topl), size_(size) {}

    qbox(qpoint<KIND> topl, QSize size) : topl_(topl), size_(size) {}

    qbox(QRect const &other) noexcept: topl_(other.topLeft()), size_(other.size()) {}

    qbox(qpoint<KIND> const &topl, QSize const &size) noexcept: topl_(topl), size_(size) {}

    decltype(auto) width() const noexcept { return size_.width(); }

    decltype(auto) height() const noexcept { return size_.height(); }

    qpoint<KIND> topLeft() const noexcept { return topl_; }

    QSize size() const noexcept { return size_; }

    void move(int dx, int dy) noexcept
    {
        topl_.move(dx, dy);
    }
    bool contains(qpoint<KIND> const &p) const noexcept
    {
        QRect qr(topl_, size_);
        return qr.contains(p);
    }
    bool intersects(qbox<KIND> const &q) const noexcept
    {
        QRect qra(topl_, size_), qrb(q.topl_, q.size_);
        return qra.intersects(qrb);
    }
    qbox<KIND> operator|(qbox<KIND> const &q)
    {
        QRect qra(topl_, size_), qrb(q.topl_, q.size_);
        QRect qrc{ qra | qrb };
        qbox<KIND> qb{qrc.topLeft(), qrc.size()};
        return qrc;
    }
    QRect asQRect() const noexcept
    {
        QRect q(topl_, size_);
        return q;
    }

    void setSize(QSize size) noexcept
    {
        size_ = size;
    }
};


#endif //IMGEX_QBOX_HH

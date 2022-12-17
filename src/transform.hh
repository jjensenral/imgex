#ifndef __IMGEX_COMMON_H
#define __IMGEX_COMMON_H


#include <iosfwd>
#include <list>
#include <QRect>
#include <QPixmap>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>

class QString;

class FileNotFound : std::exception
{
private:
    QString fn_;
public:
    FileNotFound(QString const &s) : fn_(s) {}
    ~FileNotFound() {}
    const char * what() const noexcept override { return "File not found"; }
    QString const &filename() const noexcept { return fn_; }
};


/** transform is basically a list of transforms.
 * We have to store pointer to transforms (of some description) or we will slice them.
 */
class transform final {
private:
    struct transform_t;
    struct transform_t *impl_;
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version);

public:
	transform();
	~transform();
    transform(transform const &) = delete;
    transform &operator=(transform const &);
    transform(transform &&) = default;
    transform &operator=(transform &&) = default;

    //** Set move transform to absolute (parent window) coordinate
    void move_to(QPoint) noexcept;
    //** Set (absolute) zoom level
    void zoom_to(float) noexcept;
    //** Crop takes relative coordinates (within local pixmap)
    void crop(QRect) noexcept;

    friend std::ostream &operator<<(std::ostream &, transform const &);

    float get_zoom() const noexcept;
};


// The XILDecorator base class is defined in xwin.hh
// derived classes in decor.hh
class XILDecorator;

/** Transformable defines (below) a type of image which can have transforms applied to it (using a Visitor pattern)
 * This class is the visitee, being visited by transform through apply.
 */
class ImageFile;
class XWindow;
class Transformable {
private:
    // XXX temporary hack
    friend class XWindow;
public:
    /** Alias for XILImage's box in parent's coordinates */
    typedef QRect xwParentBox;

    Transformable(ImageFile const &fn);
    // img is value copyable
    Transformable(QPixmap img) : img_(img), wbox_(img.rect()), txfs_() {}
	virtual ~Transformable() = default;
    // not inline functions defined in transform.cc
    void add_from_decorator(XILDecorator const &dec);

    /** Run the current transform on the current image */
    virtual void run();

    /** reset working copy of image to its Image */
    virtual void copy_from(Transformable const &orig);

    /** Move to new topleft global position */
    virtual QRect move_to(QPoint point);

    /** Zoom to (absolute value) */
    virtual QRect zoom_to(float);

    /** Crop to relative box (in local pixmap coordinates) */
    virtual QRect crop(QRect);


    /** Resize the image to wbox
     * Does not scale the image (the zoom transform does that)
     * \param oldbox pre-resize box, if null the current size is used */
    virtual void resize(QRect const &oldbox) {};

protected:
    /** The image to be transformed */
    QPixmap img_;
    /** placement on main window; width and height equivalent to the image size times scale */
    xwParentBox wbox_;

	/** Serialise */


    transform txfs_;
};




#endif

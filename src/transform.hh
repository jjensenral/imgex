#ifndef __IMGEX_TRANSFORM_H
#define __IMGEX_TRANSFORM_H


#include <iosfwd>
#include <list>
#include <QRect>
#include <QPixmap>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <cmath>

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

    /** Create a base Transformable
     * Pixmaps are value copyable
     * @param img base pixmap (not null)
     */
    Transformable(QPixmap img) : img_(img), cache_(), wbox_(img.rect()), txfs_() {}
	virtual ~Transformable() = default;
    // not inline functions defined in transform.cc
    void add_from_decorator(XILDecorator const &dec);

    /** Run the current transform on the current image */
    virtual void run();

    /** reset working copy of image to its Image */
    virtual void copy_from(Transformable const &orig);

    /** Move to new topleft global position
     * Returns the global rectangle to redraw */
    virtual QRect move_to(QPoint point);

    /** Zoom to (absolute value)
     * Returns the global rectangle to redraw */
    virtual QRect zoom_to(float);

    /** Crop to relative box (in local pixmap coordinates)
     * Returns the global rectangle to redraw
     * Note the return value is in global coordinates like the other transform functions */
    virtual QRect crop(QRect);

    struct transform {
        // Starting from a new image in upper left (0,0) transformations are applied in the following order
        // crop in pixmap-local coordinates (same as global as pre-transform images start top left (0,0)
        QRect crop_;
        // then zoom to absolute value keeping top left fixed
        float zoom_;
        // and finally move top left point to a new location
        QPoint move_;

        transform() : crop_(), zoom_(1.0), move_(0,0) {}

        bool has_zoom() const noexcept { return std::fabs(zoom_-1.0f) > 1e-4; }
    };

protected:
    /** The image to be transformed */
    QPixmap img_;
    /** Temporary cache of cropped image before zoom is applied
     * Needed tp ensure we don't lose zoom resolution and to avoid redoing the crop.
     * Note we can never uncrop from within this class itself
     */
     QPixmap cache_;
    /** placement on main window; width and height equivalent to the image size times scale */
    xwParentBox wbox_;

    QSize zoom_box(float g)
    {
        QSize target{cache_.isNull() ? img_.size() : cache_.size()};
        target.setHeight(target.height() * g + 0.99f );
        target.setWidth(target.width() * g + 0.99f );
        return target;
    }

    struct transform txfs_;

    friend std::ostream &operator<<(std::ostream &, transform const &);
    /** Serialise */


};




#endif

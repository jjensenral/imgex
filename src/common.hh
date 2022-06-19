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

// XXX should not be needed
class bad_transform : std::exception
{
public:
    const char *what() const noexcept override { return "bad transfprm"; }
};

/** Transformable defines (below) a type of image which can have transforms applied to it
 * (using a Visitor pattern)
*/
class Transformable;


/** Image processing workflow step; the base class being a no op.
 * Every transform contains the instructions for a transform of an (Transformable) image. */
class QPixmap;

class transform {
 public:
    /** Default transform is a NOP (No Operation) */
	transform() = default;
	virtual ~transform() {}
    transform(transform const &) = default;
    virtual transform *clone() const { return new transform(); }
    enum class transform_id { TX_NOP, TX_CROP, TX_ZOOM, TX_MOVE  };
	/** Process a XILImage (defined in xwin.hh) or an Image (defined in image.hh)
	 * The intention is that XILImage is transformed per-window and can differ for each window that the image appears in,
	 * whereas Image is transformed identically each time the same image is loaded.
	 */
	virtual void process(Transformable &) {}
    /** Return whether this transform should be stored with the image.
     * The alternative is that it is session specific and is stored with the session.
     */
    [[nodiscard]] virtual bool image() const noexcept { return true; }
    /** Apply the transform to a box and an image, returning the updated box
     * @param owner for transform to call back to modify other owner parameters
     * @param bbox Global coordinates of image
     * @param img Image; will be updated in place
     * @return Updated global coordinates
     */
    [[nodiscard]] virtual QRect apply(Transformable &owner, QRect bbox, QPixmap &img) const
    {
        /* NOP transform does nothing */
        return bbox;
    }

	/** An arbitrary identifier for each type */
	[[nodiscard]] virtual transform_id type() const noexcept { return transform_id::TX_NOP; }

    /** operator+ is used to merge transforms into each other */
    virtual transform &operator+=(transform const &other)
    {
        // The default action is that the latter supersedes the former
        copy_from(other);
        return *this;
    }
private:
    /** Merge another transform of the same type into this one
     * The default is that the last supersedes the prior
     * */
    virtual void merge(transform *other);
    /** Copy another transform into this one (if they are of the same type */
    virtual void copy_from(transform const &other) {}
    int x_;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & x_;
    }
};



/** workflow is basically a list of transforms.
 * We have to store pointer to transforms (of some description) or we will slice them.
 */
class workflow final {
public:
    typedef std::list<transform *> workflow_t;
private:
	 workflow_t w_;
     friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & w_;
    }
public:
	workflow() noexcept : w_() {}
	~workflow();
    // Since workflow owns its pointers, we must prevent copying
    workflow(workflow const &) = delete;
    workflow &operator=(workflow const &) = delete;
    // but moving is OK
    workflow(workflow &&) = default;
    workflow &operator=(workflow &&) = default;

    /** Add a transform to a workflow, taking ownership of it */
	void add(transform *t);

	[[nodiscard]] workflow_t::iterator begin() noexcept { return w_.begin(); }
    [[nodiscard]] workflow_t::iterator end() noexcept { return w_.end(); }
	/* Iterators are constant so it's safe to pass them on */
	[[nodiscard]] workflow_t::const_iterator cbegin() const noexcept { return w_.cbegin(); }
	[[nodiscard]] workflow_t::const_iterator cend() const noexcept { return w_.cend(); }
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
    workflow txfs_;
    // XXX temporary hack
    friend class XWindow;
public:
    /** Alias for XILImage's box in parent's coordinates */
    typedef QRect xwParentBox;

    Transformable(ImageFile const &fn);
    // img is value copyable
    Transformable(QPixmap img) : txfs_(), img_(img), wbox_(img.rect()) {}
	virtual ~Transformable() = default;
    // not inline functions defined in transform.cc
    void add_from_decorator(XILDecorator const &dec);
    /** Apply a single transform */
    virtual void apply(transform *);

    /** reset working copy of image to its Image */
    virtual void workCopy() {}

    /** Move to new topleft global position */
    virtual void moveto(QPoint point);

    /** Resize the image to wbox
     * Does not scale the image (the zoom transform does that) */
    virtual void resize() {};

    /** Add a transform, taking ownership of it */
    virtual void add_transform(transform *tf);
protected:
    /** The image to be transformed */
    QPixmap img_;
    /** placement on main window; width and height equivalent to the image size times scale */
    xwParentBox wbox_;

	/** Serialise */


};




#endif

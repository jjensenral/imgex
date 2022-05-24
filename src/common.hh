#ifndef __IMGEX_COMMON_H
#define __IMGEX_COMMON_H


#include <iosfwd>
#include <list>
#include <QRect>


/** Transformable defines (below) a type of image which can have transforms applied to it
 * (using a Visitor pattern)
*/
class Transformable;


/** Image processing workflow step; the base class being a no op.
 * Every transform contains the instructions for a transform of an (Transformable) image. */
class QPixmap;

class transform {
 public:
	transform() = default;
	virtual ~transform() {}
    transform(transform const &) = default;
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
     * @param bbox Bounding box of image (can be smaller than image size)
     * @param img Image; will be updated in place
     */
    virtual QRect apply(QRect bbox, QPixmap &img) const { /* do nothing */ return bbox; }
protected:
	/** An arbitrary identifier for each type */
	[[nodiscard]] virtual transform_id type() const noexcept { return transform_id::TX_NOP; }
};



/** workflow is basically a list of transforms.
 * We have to store pointer to transforms (of some description) or we will slice them.
 */
class workflow final {
public:
    typedef std::list<transform const *> workflow_t;
private:
	 workflow_t w_;
public:
	workflow() noexcept : w_() {}
	~workflow();

    /** Add a transform to a workflow, taking ownership */
	void add(transform const *t) { w_.push_back(t); }

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

class QPixmap;

class Transformable {
private:
    workflow txfs_;
public:
	Transformable() noexcept = default;
	virtual ~Transformable() = default;
    // not inline functions defined in transform.cc
    void add_from_decorator(XILDecorator const &dec);
protected:
	/** Actual implementation of transforming an image.
	 * @param img Pixmap which will be modified in-place
	 * @return new bounding box in parent window (screen) coordinates (aka wbox_)
	 */
    QRect apply(QPixmap &img) const;
	/** Serialise */
	
};




#endif

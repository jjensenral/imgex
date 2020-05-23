#ifndef __IMGEX_COMMON_H
#define __IMGEX_COMMON_H


#include <iosfwd>
#include <list>



/** Transformable defines (below) a type of image which can have transforms applied to it 
 * (using a Visitor pattern)
*/
class Transformable;


/** Enum of identifier for each type of transform */
enum transform_id { TX_NOP, TX_ZOOM, TX_PAN };


/** Image processing workflow step; the base class being a no op.
 * Every transform contains the instructions for a transform of an (Transformable) image. */

class transform {
 public:
	transform() {}
	virtual ~transform() {}
	/** Process a XILImage (defined in xwin.hh) or an Image (defined in image.hh)
	 * The intention is that XILImage is transformed per-window and can differ for each window that the image appears in,
	 * whereas Image is transformed identically each time the same image is loaded.
	 */
	virtual void process(Transformable &) {}
protected:
	/** An arbitrary identifier for each type */
	virtual transform_id type() const noexcept { return TX_NOP; }
};



/** workflow is basically a list of transforms */
class workflow final {
private:
	std::list<transform> w_;
public:
	workflow() noexcept : w_() {}
	~workflow() noexcept {}

	void add(transform &&t) { w_.emplace_back(t); }
	void add(transform const &t) { w_.emplace_back(t); }

	std::list<transform>::iterator begin() noexcept { return w_.begin(); }
	std::list<transform>::iterator end() noexcept { return w_.end(); }
	/* Iterators are constant so it's safe to pass them on */
	std::list<transform>::const_iterator cbegin() const noexcept { return w_.cbegin(); }
	std::list<transform>::const_iterator cend() const noexcept { return w_.cend(); }
};


/** Transformable defines (below) a type of image which can have transforms applied to it (using a Visitor pattern)
 * This class is the visitee, being visited by transform through apply.
 */
class Transformable {
public:
	Transformable() noexcept {}
	virtual ~Transformable() noexcept {}
	/** Apply a workflow - which applies one transform at a time */
	virtual void apply(workflow &);
	/** Each class of Transformable must implement every transform */
	virtual void apply(transform const &) = 0;
protected:
	/** Actual implementation of transform on an ILImage; assumes the correct IL image has been selected */

	/** Serialise */
	
};




#endif

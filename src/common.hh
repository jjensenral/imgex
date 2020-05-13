#ifndef __IMGEX_COMMON_H
#define __IMGEX_COMMON_H


#include <iosfwd>
#include <list>


/** Box rectangle for all sorts of rectangles */

typedef signed int coord;
typedef unsigned int dims;

struct box {
	// x,y is top left corner (NW gravity)
	coord x, y;
	dims w, h;
	constexpr box() noexcept : x(0), y(0), w(1), h(1) {}
	constexpr box(coord x1, coord y1, dims w1, dims h1) : x(x1), y(y1), w(w1), h(h1) { }
	constexpr box(box const &) = default;
	constexpr box(box &&) = default;
	constexpr box &operator=(box const &) = default;

	constexpr bool contains(coord x1, coord y1) const noexcept
	{
		return x <= x1 && x1 <= x+w && y <= y1 && y1 <= y+h;
	}

	/* Pedestrian implementation... */
	constexpr bool intersects(box const &other) const noexcept
	{
		return other.contains(x,y)		\
		    || other.contains(x+w,y)		\
		    || other.contains(x,y+h)		\
		    || other.contains(x+w,y+h);
	}
};


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

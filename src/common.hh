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


/** Image processing workflow step; the base class being a no op */

class transform {
 public:
	transform() {}
	virtual ~transform() {}
	virtual void process() {}
};

/*
std::ostream &operator<<(std::ostream &os, transform const &)
{
	os << "NOP()";
	return os;
}
*/
typedef std::list<transform> workflow;

#endif

	

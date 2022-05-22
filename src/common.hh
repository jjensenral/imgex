#ifndef __IMGEX_COMMON_H
#define __IMGEX_COMMON_H


#include <iosfwd>
#include <list>


/** Transformable defines (below) a type of image which can have transforms applied to it
 * (using a Visitor pattern)
*/
class Transformable;


/** Image processing workflow step; the base class being a no op.
 * Every transform contains the instructions for a transform of an (Transformable) image. */

class transform {
 public:
	transform() {}
	virtual ~transform() {}
    enum class transform_id { TX_NOP, TX_CROP, TX_ZOOM, TX_PAN  };
	/** Process a XILImage (defined in xwin.hh) or an Image (defined in image.hh)
	 * The intention is that XILImage is transformed per-window and can differ for each window that the image appears in,
	 * whereas Image is transformed identically each time the same image is loaded.
	 */
	virtual void process(Transformable &) {}
    /** Return whether this transform should be stored with the image.
     * The alternative is that it is session specific and is stored with the session.
     */
    [[nodiscard]] virtual bool image() const noexcept { return true; }
protected:
	/** An arbitrary identifier for each type */
	[[nodiscard]] virtual transform_id type() const noexcept { return transform_id::TX_NOP; }
};



/** workflow is basically a list of transforms */
class workflow final {
private:
	std::list<transform> w_;
public:
	workflow() noexcept : w_() {}
	~workflow() noexcept = default;

	void add(transform &&t) { w_.push_back(std::forward<transform>(t)); }
	void add(transform const &t) { w_.push_back(t); }

	std::list<transform>::iterator begin() noexcept { return w_.begin(); }
	std::list<transform>::iterator end() noexcept { return w_.end(); }
	/* Iterators are constant so it's safe to pass them on */
	[[nodiscard]] std::list<transform>::const_iterator cbegin() const noexcept { return w_.cbegin(); }
	[[nodiscard]] std::list<transform>::const_iterator cend() const noexcept { return w_.cend(); }
};


/** Transformable defines (below) a type of image which can have transforms applied to it (using a Visitor pattern)
 * This class is the visitee, being visited by transform through apply.
 */
class Transformable {
private:
    /** Temporary placeholder for transformation under construction */
    transform *tf_;
public:
	Transformable() noexcept : tf_(nullptr) {}
	virtual ~Transformable() {
        delete tf_;
    }
	/** Apply a workflow - which applies one transform at a time */
	virtual void apply(workflow &);
	/** Each class of Transformable must implement every transform */
	virtual void apply(transform const &) = 0;
    /** Are we creating a transform at the moment? */
    bool constexpr transforming() const noexcept { return tf_ != nullptr; }
    /** Start constructing a transformation */
    void begin_transform(transform::transform_id);
    [[nodiscard]] transform end_transform()
    {
        if(tf_ != nullptr) {
            transform tf{*tf_}; // copy will do for now
            delete tf_;
            tf_ = nullptr;
            return tf;
        }
        return transform(); // NOP transform
    }
    void abort_transform() noexcept
    {
        delete tf_;
        tf_ = nullptr;
    }
protected:
	/** Actual implementation of transform on an ILImage; assumes the correct IL image has been selected */

	/** Serialise */
	
};




#endif

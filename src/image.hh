#ifndef __IMGEX_IMAGE_H
#define __IMGEX_IMAGE_H


#include <set>
#include <QString>
#include <QPixmap>
#include "common.hh"


/** ImageFile represents a file as stored on physical media;
 * it contains a drive identifier to allow for the same file being
 * stored on more than one drive.
 * */

class ImageFile final {
	QString path_;
	// status
	/** Drives/locations this file is located on, each drive is identified by a single letter */
	std::set<char> drives_;
public:
	// can throw std::ios_base::failure
	ImageFile(const QString &path);
	~ImageFile() noexcept;
	QString getPath() const noexcept { return path_; }
};


/** Image is kind of an in-betweem version of image, abstracted from its X
	renderable (in XILImage) and from the file it is stored in; the same
	image may be in multiple files and may have processes done to it
	independently of its placement in a window. */

class Image final : public Transformable {
	// TODO: checksum
	ImageFile const imgf_;
    workflow wf_;
public:
	Image( ImageFile const &imgf );
	virtual ~Image();
    Image(Image &) = delete;
    Image(Image &&) = delete;
    Image &operator=(Image const &) = delete;
    Image &operator=(Image &&) = delete;
    /** The image we're holding (Transformable::img_)
     * Qt docs say QPixmap can be passed by value */
    [[nodiscard]] QPixmap getImage() const { return img_; }
	/** Returns a filename (basename) identifying the file */
	[[nodiscard]] QString getFilename() const noexcept;
    /** Add a transform to the workflow for this image, taking ownership */
    virtual void apply(transform *tf) {
        wf_.add(tf);
        Transformable::apply(tf);
    }
    // XXX temporary hack: nothing gets added to the Image
    void add_transform(transform *tf) override {}
};


#endif

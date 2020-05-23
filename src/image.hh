#ifndef __IMGEX_IMAGE_H
#define __IMGEX_IMAGE_H


#include <set>
#include <QString>
#include "common.hh"


/** ImageFile represents a file as stored on physical media; 
 * it contains a drive identifier to allow for the same file being
 * stored on more than one drive.
 * */

class ImageFile final {
	QString path_;
	// status
	/** Drives this file is located on */
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
	independent of its rendering in a window. */

class Image final : public Transformable {
	// TODO: checksum
	ImageFile const imgf_;
public:
	Image( ImageFile const &imgf );
	~Image();
	/** Returns a filename (basename) identifying the file */
	QString getFilename() const noexcept;
	/** Transformable visitor entry point for being visited by a transform */
	void apply(transform const &) override;
};


#endif

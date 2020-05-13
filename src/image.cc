#include "image.hh"
#include <iostream>
#include <fstream>
#include <cstdlib>



/** Images are loaded from removable media - we aim to remember which
 * drive each image came from.  The short term solution for this is to
 * store a one-byte file with the drive letter. */
static std::string driveletter("drive");

ImageFile::ImageFile( std::string const &fn )
{
	/** This location is used only for test/development */
	std::string mount{"/Pictures/"};
	char const *home = getenv("HOME");
	if( home )
		    mount = home + mount;
	if(!mount.ends_with('/'))
	    mount += '/';

	path_ = mount + fn;
	// raise exception unless file exists and is readable (we open it as an image later)
	std::ifstream img(path_);
	img.exceptions(img.failbit);
	// TODO: checksum the file
	// read the drive letter which should be in a fixed location
	std::ifstream drive(mount + driveletter);
	if( drive.fail() ) {
		    std::cerr << "No drive indicator\n";
	} else {
		    char d;
		    drive >> d;
		    drives_.insert(d);
	}
}


ImageFile::~ImageFile() noexcept
{
}



Image::Image(ImageFile const &imgf) : imgf_(imgf)
{
}


Image::~Image() noexcept
{
}


/* Transformable implementation */
void
Image::apply(transform const &)
{
}


std::string
Image::getFilename() const noexcept
{
	return imgf_.getPath();
}

#include "image.hh"
#include <cstdlib>
#include <QFile>
#include <QString>


/** This class is an abstract representation of the file.
 * It represents the right filename and checksum, but independent of storage location.
 * 
 * Images are loaded from removable media - we aim to remember which
 * drive each image came from.  The short term solution for this is to
 * store a one-byte file with the drive letter. */
static QString driveletter("drive");

ImageFile::ImageFile( QString const &fn )
{
	/** This location is used only for test/development */
	QString mount{"/Pictures/"};
	char const *home = getenv("HOME");
	if( home )
		    mount = home + mount;
	if(!mount.endsWith('/'))
	    mount += '/';

	path_ = mount + fn;
/*
	QString filename(path_.c_str());

	// raise exception unless file exists and is readable (we open it as an image later)
	QFile img(filename);
*/
	// TODO: checksum the file
	// read the drive letter which should be in a fixed location
/*
	QFile drive(filename);
	if( !drive.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		    std::cerr << "No drive indicator\n";
	} else {
		    char d;
			// XXX
		    ;
		    drives_.insert(d);
	}
*/
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


QString
Image::getFilename() const noexcept
{
	return imgf_.getPath();
}

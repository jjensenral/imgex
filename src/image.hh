#ifndef __IMGEX_IMAGE_H
#define __IMGEX_IMAGE_H


#include <set>
#include <string>
#include "common.hh"


/** ImageFile represents a file as stored on physical media; 
 * it contains a drive identifier to allow for the same file being
 * stored on more than one drive.
 * */

class ImageFile final {
    std::string path_;
    // status
    /** Drives this file is located on */
    std::set<char> drives_;
public:
    // can throw std::ios_base::failure
    ImageFile(const std::string &path);
    ~ImageFile() noexcept;
    std::string const &getPath() const noexcept { return path_; }
};


/** Image is kind of an in-betweem version of image, abstracted from its X
    renderable (in XILImage) and from the file it is stored in; the same
    image may be in multiple files and may have processes done to it
    independent of its rendering in a window. */

class Image final {
    // TODO: checksum
    ImageFile const imgf_;
public:
    Image( ImageFile const &imgf );
    ~Image();
    /** Returns a filename (basename) identifying the file */
    std::string getFilename() const noexcept;
};


#endif

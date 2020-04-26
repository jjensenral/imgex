#include <string>
#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "xwin.hh"
#include "image.hh"

/**
 * NOTE this is just a test main program, not a production version
 */


int
main([[maybe_unused]] int argc, [[maybe_unused]] char const *argv[])
{
    char const *display = getenv("DISPLAY");
    if(!display)
	display = ":0";

    /* Test images to load - find your own and put them in  ~/Pictures (the test mount point) */
    std::list<std::string> files{ "testimg0.jpeg",
				  "testimg1.png",
				  "testimg2.jpeg" };
    XMain xm(display);
    size_t win = xm.createWindow();
    for( auto const &fn : files ) {
	try {
	    ImageFile imf(fn);
	    xm.mkimage(win, imf);
	} catch( std::exception const &e ) {
	    std::cerr << fn << " failed " << e.what() << " \n";
	} catch( std::string const &s ) {
	    std::cerr << fn << " failed " << s << " \n";
	}
    }
    xm.flush();
    xm.run();
    return 0;
}

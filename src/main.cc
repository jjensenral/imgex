#include <QString>
#include <QScreen>
#include <algorithm>
#include <string>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <fmt/core.h>
#include "xwin.hh"
#include "image.hh"
#include "session.hh"

/**
 * NOTE this is just a test main program, not a production version
 */


int
main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	/* Test images to load - find your own and put them in  ~/Pictures (the test mount point) */
	std::list<QString> files{ "testimg0.jpeg",
							  "testimg1.png",
							  "testimg2.jpeg",
                              "testimg3.jpg"};

    Session s(argc, argv);

	for( auto const &fn : files ) {
		try {
            ImageFile imf(fn);
            s.mkimage(imf, fn);
        } catch(FileNotFound const &f) {
            std::cerr << f.what() << f.filename().toStdString() << std::endl;
		} catch( std::exception const &e ) {
		    qWarning("Exception %s: %s", qPrintable(fn), e.what());
		} catch( char const *msg ) {
		    qWarning("Exception %s", msg);
		}
	}

	s.exec();
	return 0;
}

#include <QString>
#include <algorithm>
#include <string>
#include <unistd.h>
#include <iostream>

#include "xwin.hh"
#include "image.hh"

/**
 * NOTE this is just a test main program, not a production version
 */


int
main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	/* Test images to load - find your own and put them in  ~/Pictures (the test mount point) */
	std::list<QString> files{ "testimg0.jpeg",
							  "testimg1.png",
							  "testimg2.jpeg" };
	QGuiApplication app(argc, argv);
	XWindow w;
	w.show();

	for( auto const &fn : files ) {
		try {
		    ImageFile imf(fn);
			w.mkimage(imf, fn);

		} catch( std::exception const &e ) {
		    qWarning("Exception %s: %s", qPrintable(fn), e.what());
		} catch( char const *msg ) {
		    qWarning("Exception %s", msg);
		}
	}


	app.exec();
	return 0;
}

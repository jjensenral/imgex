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
	QGuiApplication app(argc, argv);

    std::vector<XWindow> windows;
    windows.reserve(3); // try to prevent moving elements
    // create a window but don't show it yet
    windows.emplace_back(nullptr);
    // Compatibility
    XWindow &w = windows[0];

    QScreen *s = w.screen();
    QRect geom{s->virtualGeometry()};
    auto y = s->virtualSiblings();
    // create windows for the other screens
    auto c{0};
    for( auto z : y ) {
        if( z != s ) {
            fmt::print(stderr, "Creating window on screen {}\n", c);
            windows.emplace_back(z);
        } else {
            fmt::print(stderr, "No window needed for screen {}\n", c);
        }
        ++c;
    }

    fmt::print(stderr, "{}x{}+{}+{}\n", geom.width(), geom.height(), geom.x(), geom.y());
    // Now show them all
    for( auto &m : windows ) m.show();

	for( auto const &fn : files ) {
		try {
            ImageFile imf(fn);
            w.mkimage(imf, fn);
        } catch(FileNotFound const &f) {
            std::cerr << f.what() << f.filename().toStdString() << std::endl;
		} catch( std::exception const &e ) {
		    qWarning("Exception %s: %s", qPrintable(fn), e.what());
		} catch( char const *msg ) {
		    qWarning("Exception %s", msg);
		}
	}

	app.exec();
	return 0;
}

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

    std::vector<std::unique_ptr<XWindow>> windows;
    // create a window but don't show it yet
    windows.emplace_back(std::make_unique<XWindow>());
    // Compatibility
    XWindow &w = *(windows[0].get());

    QScreen *s = w.screen();

    auto y = s->virtualSiblings();
    // create windows for the other screens
    auto c{0};
    for( auto z : y ) {
        auto geom = z->geometry();
        if( z != s ) {
            fmt::print(stderr, "Creating window on screen {}\n", c);
            windows.emplace_back(std::make_unique<XWindow>(z));
        } else {
            fmt::print(stderr, "No window needed for screen {}\n", c);
        }
        auto &win = windows.back();
        geom.adjust(20, 20, -20, -20);
        fmt::print(stderr, "Placing window {} at {}x{}+{}+{}\n", c, geom.width(), geom.height(),
                   geom.x(), geom.y());
        win->setGeometry(geom);
        ++c;
    }

    // Now show them all
    for( auto &m : windows ) m->showMaximized();

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

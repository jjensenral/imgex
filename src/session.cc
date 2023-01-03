#include "session.hh"
#include <QScreen>
#include <fmt/core.h>



Session::Session(int argc, char **argv) : app_(argc, argv),
                                          id_(0),
                                          time_(std::chrono::system_clock::now()),
                                          windows_()
{
    // Create a window somewhere
    windows_.emplace_back(std::make_unique<XWindow>(nullptr));
    XWindow *w = windows_[0].get();
    QScreen *s = w->screen();

    auto y = s->virtualSiblings();
    // create windows for the other screens
    auto c{0};
    for( auto z : y ) {
        auto geom = z->geometry();
        if( z != s ) {
            fmt::print(stderr, "Creating window on screen {}\n", c);
            windows_.emplace_back(std::make_unique<XWindow>(z));
        } else {
            fmt::print(stderr, "No window needed for screen {}\n", c);
        }
        auto &win = windows_.back();
        geom.adjust(20, 20, -20, -20);
        fmt::print(stderr, "Placing window {} at {}x{}+{}+{}\n", c, geom.width(), geom.height(),
                   geom.x(), geom.y());
        win->setGeometry(geom);
        ++c;
    }
    // Now show them all
    for( auto &m : windows_ ) m->showMaximized();

}

Session::~Session()
{
}

void
Session::persist(class Persist &)
{
}

void
Session::lock()
{
}

void
Session::unlock()
{
}

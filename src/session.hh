#ifndef __IMGEX_SESSION_H
#define __IMGEX_SESSION_H

#include <chrono>
#include <memory>
#include <QGuiApplication>
#include <vector>
#include "xwin.hh"

typedef std::chrono::duration<int64_t> ses_time;

/* Persistence class defined in persist.hh */
class Persist;

class Session final {
 private:
    QGuiApplication app_;
    uint64_t id_;
    std::chrono::time_point<std::chrono::system_clock> time_;
    std::vector<std::unique_ptr<XWindow>> windows_;


 public:
    /** Initial creation */
    Session(int argc, char **argv);
    ~Session();
    Session(Session const &) = delete;
    Session(Session &&) = delete;
    Session &operator=(Session const &) = delete;
    Session &operator=(Session &&) = delete;

    /** Reinitialise a session */
    void reset();

    // XXX temporary
    void mkimage(ImageFile const &img, QString fn)
    {
        windows_[0]->mkimage(img, fn);
    }

    /** Persist all session data */
    void persist(class Persist &);

    /** Lock session - all images - from mousing etc */
    void lock();

    /** Unlock, opening all images to manipulation */
    void unlock();

    void exec() { app_.exec(); }
};


#endif

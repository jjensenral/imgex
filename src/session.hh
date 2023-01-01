#ifndef __IMGEX_SESSION_H
#define __IMGEX_SESSION_H

#include <chrono>
#include <memory>

typedef std::chrono::duration<int64_t> ses_time;

class SessionImpl;

class Session final {
 private:
    uint64_t id_;
    std::chrono::time_point<std::chrono::system_clock> time_;
    std::unique_ptr<SessionImpl> impl_;

 public:
    Session();
    ~Session();

    /** Persist all session data into file
     * \param filename - location to write session (filename, directory).
     * location defaults to current working directory if writeable, and /tmp if not */
    void persist(char const *filename = nullptr);

    /** Lock session - all images - from mousing etc */
    void lock();

    /** Unlock, opening all images to manipulation */
    void unlock();
};


#endif

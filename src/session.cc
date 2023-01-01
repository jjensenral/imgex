#include "session.hh"

/** The base class is a null implementation (does nothing)
 *
 */
class SessionImpl
{

};




Session::Session() : id_(0), time_(std::chrono::system_clock::now())
{
}

Session::~Session()
{
}

void
Session::persist(char const *filename)
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

#include "transform.hh"

#include <algorithm>

/* Defined in commmon.hh
 * A workflow is implemented by, quite simply, applying one transform at a time, to *this
 */

void
Transformable::apply(workflow &w)
{
    for( transform const &t : w)
        this->apply(t);
}

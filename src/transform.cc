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


void
Transformable::begin_transform(transform::transform_id tid) {
    // new rule: can only define one transform at a time
    // XXX for now we just abort the previous transform
    abort_transform();
    switch(tid) {
        case transform::transform_id::TX_NOP:
            tf_ = new transform();
            break;
        case transform::transform_id::TX_CROP:
            tf_ = new tf_crop();
            break;
    }
}
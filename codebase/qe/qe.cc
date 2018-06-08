
#include "qe.h"

Filter::Filter(Iterator* input, const Condition &condition) {
}

RC Filter::getNextTuple(void * data)
{
    return QE_EOF;
}
// ... the rest of your implementations go here

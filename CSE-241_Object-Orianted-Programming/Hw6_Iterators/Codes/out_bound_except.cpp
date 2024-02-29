#include "out_bound_except.h"

namespace std {

    out_bound::out_bound(const char* sa)
    {
        excep = sa;
    }

    const char* out_bound::what () const noexcept{
        return excep;
    }

    out_bound::~out_bound() {
        // if(excep != nullptr) delete excep;
    }
}

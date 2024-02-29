#include "Location.h"

namespace Tetris{
#pragma region Location
// is other in 1 under
bool Location::operator>=(const Location& o) const{
    return ((x == (o.x - 1)) && (y == o.y));
}

// is other in 1 above
bool Location::operator<=(const Location&o) const{
    return ((x == o.x + 1) && (y == o.y));
}

// is other in 1 right
bool Location::operator>(const Location&o) const{
    return ((x == o.x) && (y == o.y - 1));
}  

// is other in 1 left
bool Location::operator<(const Location&o) const{
    return ((x == o.x) && (y == o.y + 1));
}  


#pragma endregion
}
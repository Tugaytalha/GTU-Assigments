#ifndef out_bound_exc
#define out_bound_exc

#include <exception>

namespace std {
    class out_bound : public std::exception
    {
    private:
        const char* excep; 
    public:
        out_bound(const char* sa);
        ~out_bound();
        const char* what() const noexcept override;
    };
}



#endif



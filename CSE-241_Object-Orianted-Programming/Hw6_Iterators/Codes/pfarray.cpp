
#include "pfarray.h"

using std::out_bound;  // my own exception

#pragma region debug

bool debug_cout(bool sa) {  // function that using for debug
    #ifndef NDEBUG
    if(sa) std::cout << "true";
    else std::cout << "false";
    debug_cout();
    #endif
    return sa;
}

bool debug_cout(char *sa) {  // function that using for debug
    static int counter = 1;
    #ifndef NDEBUG
    std::cout << sa << "\033[1;31m" << counter << "\033[0m\n" << std::endl;
    ++counter;
    #endif
    return true;
}

bool debug_cout(std::string sa) {  // function that using for debug
    static int counter = 1;
    #ifndef NDEBUG
    std::cout << sa << "\033[1;31m" << counter << "\033[0m\n" << std::endl;
    ++counter;
    #endif
    return true;
}

bool debug_cout(int sa) {  // function that using for debug
    #ifndef NDEBUG
    std::cout << sa;
    debug_cout();
    #endif
    return true;
}

#pragma endregion

namespace PFArraySavitch
{

    template<class T>
    PFArray<T>::PFArray(int size) :capacity(size)
    {
        a.reset(new T[capacity]);
        element_size = sizeof(T);
    }

    template<class T>
    PFArray<T>::PFArray(const PFArray<T>& pfaObject) : capacity(pfaObject.size( ))
    {
        a.reset(new T[capacity]);
        for (int i =0; i < capacity; i++)
            a.get()[i] = pfaObject.a.get()[i];
    }

    template<class T>
    PFArray<T>::PFArray(const PFArray<T>&& pfaObject) : capacity(pfaObject.size( ))
    {
        a = pfaObject.a;
        element_size = pfaObject.element_size;
    }

    template<class T>
    void PFArray<T>::add(const T& element)
    {
        std::shared_ptr<T> temp(new T[capacity + 1]);
        for(int i = 0; i < capacity; ++i) temp.get()[i] = a.get()[i];
        temp.get()[capacity++] = element;

        a = temp;
    }

    template<class T>
    int PFArray<T>::size( ) const
    {
        return capacity;
    }

    template<class T>
    void PFArray<T>::clear( ) {
        a.reset(); 
        capacity = 0;
    }

    template<class T>
    typename PFArray<T>::iterator PFArray<T>::erase(const PFArray<T>::iterator& index) {
        std::shared_ptr<T> temp(new T[capacity - 1]);
        int tsize = 0, it;

        for(auto i = this->cbegin(); i != this->cend(); ++i) {
            if(i != index)
                temp.get()[tsize++] = *i;
            else it = tsize;

        }
        a = temp;
        capacity = tsize;
        if (it == this->size()) --it;
        return this->begin() + it;
    }

    template<class T>
    typename PFArray<T>::iterator PFArray<T>::begin() {
        return iterator(a.get());
    }

    template<class T>
    typename PFArray<T>::iterator PFArray<T>::end() {
        return iterator(a.get() + capacity);
    }

    template<class T>
    const typename PFArray<T>::iterator PFArray<T>::cbegin() const {
        return iterator(a.get());
    }

    template<class T>
    const typename PFArray<T>::iterator PFArray<T>::cend() const {
        return iterator(a.get() + capacity);
    }

    template<class T>
    T& PFArray<T>::operator[](int index)
    {
        if (index >= capacity || index < 0)
            throw out_bound("Illegal index in PFArray.\n");


       return a.get()[index];
    }

    template<class T>
    const T& PFArray<T>::operator[](int index) const 
    {
        if (index >= capacity || index < 0)
            throw out_bound("Illegal index in PFArray.\n");


       return a.get()[index];
    }

    template<class T>
    PFArray<T>& PFArray<T>::operator =(const PFArray<T>& rightSide)
    {
        if (size() != rightSide.size())
        {
            a.reset(new T[rightSide.size()]);
        }

        capacity = rightSide.size();
        for (int i = 0; i < size(); i++)
            a.get()[i] = rightSide.a.get()[i];

        return *this;
    }

    template<class T>
    PFArray<T>& PFArray<T>::operator =(const PFArray<T>&& rightSide) {
        a = rightSide.a;
        element_size = rightSide.element_size;
        capacity = rightSide.size();
    }
    
    template<class T>
    bool PFArray<T>::operator ==(const PFArray<T>& rightSide) const {
        return (a == rightSide.a); 
    }

    template<class T>
    bool PFArray<T>::operator !=(const PFArray<T>& rightSide) const {
        return (a != rightSide.a); 
    }

    template<class T>
    std::ostream &operator<<(std::ostream &out, const PFArray<T>& object){
        out << "{ ";
        for(int i = 0; i < object.size() - 1; ++i) {
            out << object[i] << " ,";
        }
        out << object[object.size() - 1] << "}";

        return out;
    }

    template<class T>
    void print(const T& element) {
        std::cout << element << " ";
    }
    
}// PFArraySavitch
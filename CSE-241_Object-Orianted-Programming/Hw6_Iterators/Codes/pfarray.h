#ifndef PFARRAY_H
#define PFARRAY_H

#include <memory>
#include "out_bound_except.h"
#include <iostream>

bool debug_cout(std::string sa ="");

bool debug_cout(int);

bool debug_cout(bool);

bool debug_cout(char *sa);

// using 
namespace PFArraySavitch
{
    template<class T>
    void print(const T&);
    
    template<class T>
    class PFArray
    {
    public:
        PFArray(int capacityValue = 0);

        PFArray(const PFArray<T>& pfaObject);

        PFArray(const PFArray<T>&& pfaObject);

        virtual ~PFArray() {/* do nothing, shared_ptr handle delete itself*/};

        void add(const T& element);
        //Precondition: The array is not full.
        //Postcondition: The element has been added.

        //Test whether container is empty
        bool empty() {return ((a == nullptr) || (size() == 0));};

        int size( ) const;


        // clear all content
        void clear();

        void emptyArray( );
        //Resets the number used to zero, effectively emptying the array.

        //bool operator<();

        T& operator[](int index);
        //Read and change access to elements 0 through numberUsed - 1.
        const T& operator[](int index) const ;
        //Read and change access to elements 0 through numberUsed - 1.

        PFArray<T>& operator =(const PFArray<T>& rightSide);

        bool operator ==(const PFArray<T>& rightSide) const;

        bool operator !=(const PFArray<T>& rightSide) const;

        PFArray<T>& operator =(const PFArray<T>&& rightSide);

        // Iterator class
        class iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            iterator(T* o) : ptr(o) {}

            iterator operator+(difference_type n) const { return iterator(ptr + n); }
            iterator operator-(difference_type n) const { return iterator(ptr - n); }
            iterator& operator++() { ++ptr; return *this; }
            iterator operator++(int) { iterator temp = *this; ++ptr; return temp; }
            iterator operator+(int o) {return iterator(ptr + o); }
            iterator& operator--() { --ptr; return *this; }
            iterator operator--(int) { iterator temp = *this; --ptr; return temp; }
            iterator operator-(int o) {return iterator(ptr - o); }
            iterator& operator+=(difference_type n) { ptr += n; return *this; }
            iterator& operator-=(difference_type n) { ptr -= n; return *this; }
            T& operator*() const { return *ptr; }
            T* operator->() const { return ptr; }
            bool operator==(const iterator& other) const { return ptr == other.ptr; }
            bool operator!=(const iterator& other) const { return ptr != other.ptr; }
            difference_type operator-(const iterator& other) const { return ptr - other.ptr; }
            bool operator<(const iterator& other) const { return ptr < other.ptr; }
            bool operator>(const iterator& other) const { return ptr > other.ptr; }
            bool operator<=(const iterator& other) const { return ptr <= other.ptr; }
            bool operator>=(const iterator& other) const { return ptr >= other.ptr; }
            T& operator[](int i) { return *(ptr + i); }
            const T& operator[](int i) const { return *(ptr + i); }
        private:
            T* ptr;
        };

        //Erase element pointed by the given iterator
        iterator erase(const iterator&index);

        iterator begin();
        const iterator cbegin() const;
        iterator end();
        const iterator cend() const;

        void print(const T& element);

        template<class T1>
        friend std::ostream &operator<<(std::ostream &out, const PFArray<T1>& object);

    private:
        std::shared_ptr<T> a; //for an array of T.
        int capacity; //for the size of the array.
        int element_size;  // size of an element
    };

    
}// PFArraySavitch

#include "pfarray.cpp"

#endif //PFARRAY_H
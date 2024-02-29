
#ifndef DAYOFYEAR_H
#define DAYOFYEAR_H

#include<iostream>


namespace DayOfYear
{
    

    const int DAYS_IN_YEAR = 365;

    // Public inner class representing a day of the year
    class DayofYear
    {
    public:
        // Constructor
        DayofYear( int month=1, int day=1);
        DayofYear(const DayofYear&other);

        // Getters
        int day() const { return day_; }
        int month() const{ return month_; }

        // Setters
        void set(int newMonth, int newDay);
        //Precondition: newMonth and newDay form a possible date.
        void set_month(int month);
        //Precondition: 1 <= newMonth <= 12
        //Postcondition: The date is set to the first day of the given month.

        // print
        friend std::ostream &operator<<(std::ostream &out, const DayofYear &day) ;

        //coppy assigment operator
        const DayofYear& operator=(const DayofYear&other);


        // comparasion operator overloadings
        bool operator <(const DayofYear&) const;
        bool operator >(const DayofYear&) const;
        bool operator >=(const DayofYear&) const;
        bool operator <=(const DayofYear&) const;
        bool operator ==(const DayofYear&) const;
        bool operator !=(const DayofYear&) const;

    private:
        // The day of the year, with 1 representing the first day of the year
        int day_;
        int month_;
    };

    // Public class representing a set of DayofYear objects
    class DayofYearSet
    {
    public:
        // Constructors
        DayofYearSet();
        DayofYearSet(const DayofYearSet&other);
        DayofYearSet(const std::initializer_list<DayofYear> &list);

        // Destructor
        ~DayofYearSet();

        // Overloaded stream insertion operator
        friend std::ostream &operator<<(std::ostream &out, const DayofYearSet &set);

        // Overloaded comparison operators
        bool operator==(const DayofYearSet &other) const;
        bool operator!=(const DayofYearSet &other) const;

        // Member function to remove an element from the set, if an error accured function be terminated
        void remove(const DayofYear &day);  

        // Recursive binary search, takes  elemnt for find, 0 and size -1
        int find(const DayofYear& key, int low, int high) const;

        // Member function to return the number of elements in the set
        int size() const {return size_;}

        // Overloaded binary & unary operators
        const DayofYearSet& operator=(const DayofYearSet& other);  // basicly coppy assigment
        DayofYearSet& operator+(const DayofYear &day);   // add element to set; non-const return beacuse of I can add again and again
        const DayofYearSet operator+(const DayofYearSet other) const;  // return union of the sets, call by value
        DayofYearSet& operator-(const DayofYear &day);  // removes an elemnt rom the set
        const DayofYearSet operator-(const DayofYearSet &other) const;  // returns difference of the sets
        const DayofYearSet operator^(const DayofYearSet &other) const;  // returns the intersection set.
        DayofYear& operator[](const int i){ return this->days[i];}
        const DayofYear& operator[](const int i) const { return this->days[i];}
        const DayofYearSet operator!() const;  // return complement of the set

    private:
        int size_;
        DayofYear *days;
    };

}

#endif
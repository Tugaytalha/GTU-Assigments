#include "DayofYear.h"

namespace DayOfYear
{
#pragma region DayofYear
    // DayofYear class implementation
    // constructors
    DayofYear::DayofYear(int month, int day) 
    {
        set(month, day);
    }
    DayofYear::DayofYear(const DayofYear&other) {
        set(other.month(), other.day());
    }

    // day and month setter
    void DayofYear::set(int newMonth, int newDay)
    {
        // Validate input
        if (newMonth < 1 || newMonth > 12)
        {
            throw std::invalid_argument("newMonth must be between 1 and 12");
        }

        // Adjust newDay and newMonth values to account for months with different number of days
        int daysInMonth = 31; // Most months have 31 days
        if (newMonth == 2)
        {
            daysInMonth = 28; // for 365 days in a year february has to has 28 days
        }
        else if (newMonth == 4 || newMonth == 6 || newMonth == 9 || newMonth == 11)
        {
            daysInMonth = 30; // April, June, September, and November have 30 days
        }
        if (newDay < 1 || newDay > daysInMonth)
        {
            throw std::invalid_argument("newDay must be between 1 and 365");
        }

        month_ = newMonth;
        day_ = newDay;
    }

    //month setter
    void DayofYear::set_month(int month)
    {
        // Validate input
        if (month < 1 || month > 12)
        {
            throw std::invalid_argument("day must be between 1 and 365");
        }
        month_ = month;
    }

    // print
    std::ostream &operator<<(std::ostream &out, const DayofYear &day) 
{
    switch (day.month())
    {
        case 1:
            out << "January "; break;
        case 2:
            out << "February "; break;
        case 3:
            out << "March "; break;
        case 4:
            out << "April "; break;
        case 5:
            out << "May "; break;
        case 6:
            out << "June "; break;
        case 7:
            out << "July "; break;
        case 8:
            out << "August "; break;
        case 9:
            out << "September "; break;
        case 10:
            out << "October "; break;
        case 11:
            out << "November "; break;
        case 12:
            out << "December "; break;
        default:
            out << "Error in operator<< for DayofYear. Contact software vendor.";
    }
    out << day.day() << std::endl;

    return out;
}

    // comparasion operator overloadings
    bool DayofYear::operator<(const DayofYear& b) const{
	if ((month() > b.month())|| ((month() == b.month()) && (day() > b.day()))){
		return false;
	} 
		return true;
    }

    bool DayofYear::operator>(const DayofYear& b) const{
	if ((month() < b.month())|| ((month() == b.month()) && (day() < b.day()))){
		return false;
	} 
		return true;
    }

    bool DayofYear::operator<=(const DayofYear&b) const{
        return !(*this > b);
    }

    bool DayofYear::operator >=(const DayofYear& b) const{
        return !(*this < b);
    }

    bool DayofYear::operator ==(const DayofYear& b) const{
	if (month() == b.month() && day() == b.day()){
		return true;
	} 
		return false;
    }
    bool DayofYear::operator !=(const DayofYear& b) const{
        return !(*this == b);
    }
    
    // probably same as default but guaranteed
    const DayofYear& DayofYear::operator=(const DayofYear&other) {
        day_ = other.day_;
        month_ = other.month_;
        return *this;
    }


#pragma endregion

#pragma region DayofYearSet
    // Constructors
    DayofYearSet::DayofYearSet() :size_(0) {days = nullptr;}
    DayofYearSet::DayofYearSet(const DayofYearSet&other) :size_(0) {
        days = nullptr;
        *(this) = other; 
    }
    DayofYearSet::DayofYearSet(const std::initializer_list<DayofYear> &liste) : size_(0){
        days = nullptr;
        for (auto day : liste) {
            *this + day;
        }
    }

    // Destructor
    DayofYearSet::~DayofYearSet() {
        if(days != nullptr) {
            size_ = 0;
            delete[] days;
            days = nullptr;
        }
    }

    // comprasion  operator overloads
    bool DayofYearSet::operator==(const DayofYearSet &other) const {
        bool flag = true; 

        if (size() != other.size()) return false; 
        for(auto i = 0; i < size(); ++i) {
            if(this->days[i] != other.days[i]) return false;
        }
        return true;
    }
    bool DayofYearSet::operator!=(const DayofYearSet &other) const {
        return !(*this == other);
    }

    // Binary search, takes element for search, 0 and size - 1
    int DayofYearSet::find(const DayofYear& key, int low, int high) const
    {
        // Base case
        if (low > high)
            return -1;

        // Calculate mid index
        int mid = low + (high - low) / 2;

        // Check if key is present at mid
        if ((*this)[mid] == key)
            return mid;

        // If key is greater, ignore left half
        if ((*this)[mid] < key)
            return find(key, mid + 1, high);

        // If key is smaller, ignore right half
        return find(key, low, mid - 1);
    }

    // Remove an element from the set, if an error accured function be terminated
    void DayofYearSet::remove(const DayofYear &day) {
        if (days == nullptr) return;
        auto index = find(day, 0, size()-1);
        DayofYearSet temp;

        if (index != -1) {
            for (auto i = 0; i < this->size(); ++i) {
                if(i != index) {
                    temp + (*this)[i];
                }
            }
            delete[] this->days;
            this->days = temp.days;
            --size_;
            temp.days = nullptr;
            temp.size_ = 0;
        }
    }

    
    const DayofYearSet& DayofYearSet::operator=(const DayofYearSet& other) {

        if (this->days != nullptr) delete[] this->days;
        this->days = nullptr;
        size_ = 0;
        for( auto i = 0; i < other.size(); ++i) {
            *this + other.days[i];
        }

        return *this;
    }

    // add element to set not const beacuse of I can add again and again
    DayofYearSet& DayofYearSet::operator+(const DayofYear &day) {
        if (this->find(day, 0 , size() - 1) == -1)
        {    
            decltype(this->days) temp;
            decltype(0) i;

            temp = new DayofYear[size() + 1];
            for (i = 0; i < size() && day > this->days[i]; ++i) {
                temp[i] = this->days[i];
            }
            temp[i] = day;
            for(; i < size(); ++i) {
                temp[i + 1] = this->days[i];
            }
            
            delete[] this->days;
            this->days = temp;
            ++size_;
            temp = nullptr;
        }

        return *this;
    }

    // return union of the sets
    const DayofYearSet DayofYearSet::operator+(const DayofYearSet other) const {
        DayofYearSet temp;
 
        for(auto i = 0; i < this->size(); ++i) {
            temp + this->days[i];
        }
        for(auto i = 0; i < other.size(); ++i) {
            temp + other.days[i];
        }

        return temp;
    }

    // removes an elemnt rom the set
    DayofYearSet& DayofYearSet::operator-(const DayofYear &day) {
        this->remove(day);

        return *this;
    }  

    // returns difference of the sets
    const DayofYearSet DayofYearSet::operator-(const DayofYearSet &other) const {
        DayofYearSet temp;

        for(auto i = 0; i < size(); ++i) {
            if(other.find(this->days[i], 0, other.size() - 1) == -1) {
                temp + this->days[i];
            }
        }

        return temp;
    }
    
    // returns the intersection set.
    const DayofYearSet DayofYearSet::operator^(const DayofYearSet &other) const {
        DayofYearSet temp;

        for(auto i = 0; i < this->size(); ++i) {
            if(other.find(this->days[i], 0, other.size() -1) != -1) temp + this->days[i];
        }

        return temp;
    }

    // return complement of the set
    const DayofYearSet DayofYearSet::operator!() const {
        DayofYearSet temp;
        int dayof = 0, monthof = 1, daysInMonth = 31;

        for(auto i = 0; i < DAYS_IN_YEAR; ++i) {
            ++dayof;
            if(dayof > daysInMonth) {
                dayof = 1;
                ++monthof;
                daysInMonth = 31; // Most months have 31 days
                if (monthof == 2)
                {
                    daysInMonth = 28; // For a year to have 365 days, February must have 28 days
                }
                else if (monthof == 4 || monthof == 6 || monthof == 9 || monthof == 11)
                {
                    daysInMonth = 30; // April, June, September, and November have 30 days
                }
            }
            temp + DayofYear(monthof, dayof);
        }

        for(auto i = 0; i < this->size(); ++i) {
            temp - this->days[i];
        }

        return temp;
    }  

    // Overloaded stream insertion operator
    std::ostream &operator<<(std::ostream &out, const DayofYearSet &set) {
        for(auto i = 0; i < set.size(); ++i) {
            out << i+1 << "." << set[i];
        }
        out << std::endl;

        return out;
    }


#pragma endregion
}
#include "DayofYear.h"
#include <iostream>

//namespace std {string std::getline(std::istream&, std::string);}

int main() { 
    DayOfYear::DayofYear d1(3,5), d2(7,9), d3(11, 13), d4, d5(1,12), d6(12,31);
    DayOfYear::DayofYearSet year_set1 = {d1, d2, d3}, year_set2 = {d2, d1, d4, d5, d3, d4}, year_set3(year_set1), year_set4 ={d3, d6};

    std::cout << "\033[0;31m                   list initializer and coppy constructor, operator<<, operator+ that add elemnt to set\033[0m" << std::endl;
    std::cout << "d1:" << std::endl << d1;
    std::cout << "d2:" << std::endl << d2;
    std::cout << "d3:" << std::endl << d3;
    std::cout << "d4:" << std::endl << d4;
    std::cout << "d5:" << std::endl << d5;
    std::cout << "d6:" << std::endl << d6 << std::endl;
    std::cout << "DayofYear set that initialized with list initializer constructor: {d1, d2 ,d3}" << std::endl;
    std::cout << year_set1;
    std::cout << "DayofYear set that initialized with list initializer constructor: {d2, d1, d4, d5, d3, d4}" << std::endl;
    std::cout << year_set2;
    std::cout << "DayofYear set that initialized with copy constructor: DayofYearSet(year_set1)" << std::endl;
    std::cout << year_set3;
    std::cout << "Those have sorted and unique elements therefore operator+(add elemrnt to the set) and list initializer constructor works very well" << std::endl;
    std::cout << "In addition, the overloaded stream insertion operator was used while they were being printed and works very well" << std::endl << std::endl;

    std::cout << "Press enter to next Page" << std::endl;
    std::string str;
    std::getline(std::cin, str);
    std::cout << "\033[48;2;" << 70 << ';' << 70 << ';' << 70 << 'm'<< std::endl << std::endl << std::endl << std::endl << std::endl << std::endl <<   "\033[0m" << std::endl;

    std::cout << "\033[0;31m                           operator==, operator!=\033[0m" << std::endl;
    std::cout << "year_set1:" << std::endl;
    std::cout << year_set1;
    std::cout << "year_set3:" << std::endl;    
    std::cout << year_set3;
    std::cout << "year_set4:" << std::endl;    
    std::cout << year_set4;
    std::cout << "year_set1 == year_set4: " << (year_set1 == year_set4 ? "\033[0;32m True \033[0m" : "\033[0;31m False \033[0m") << std::endl;    
    std::cout << "year_set4 == year_set3: " << (year_set4 == year_set3 ? "\033[0;32m True \033[0m" : "\033[0;31m False \033[0m") << std::endl;    
    std::cout << "year_set1 == year_set3: " << (year_set1 == year_set3 ? "\033[0;32m True \033[0m" : "\033[0;31m False \033[0m") << std::endl << std::endl;    
    std::cout << "year_set4 != year_set3: " << (year_set4 != year_set3 ? "\033[0;32m True \033[0m" : "\033[0;31m False \033[0m") << std::endl;    
    std::cout << "year_set1 != year_set3: " << (year_set1 != year_set3 ? "\033[0;32m True \033[0m" : "\033[0;31m False \033[0m") << std::endl << std::endl;    

    std::cout << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    std::cout << "\033[48;2;" << 70 << ';' << 70 << ';' << 70 << 'm'<< std::endl << std::endl << std::endl << std::endl << std::endl << std::endl <<   "\033[0m" << std::endl;

    std::cout << "\033[0;31m                size, remove, operator-(removes element from the set) operator+(add element to set)\033[0m" << std::endl;
    std::cout << "d1:" << d1;
    std::cout << "d2:" << d2;
    std::cout << "d3:" << d3;
    std::cout << "d5:" << d5;
    std::cout << "d6:" << d6 << std::endl;
    std::cout << "year_set2:" << std::endl;    
    std::cout << year_set2;
    std::cout << "year_set2.size():" << "\033[0;31m" << year_set2.size() << "\033[0m" << std::endl << std::endl;
    year_set2.remove(d5);    
    std::cout << "year_set2.remove(d5):" << std::endl;    
    std::cout << year_set2;
    std::cout << "year_set2.size():" << "\033[0;31m" << year_set2.size() << "\033[0m" << std::endl << std::endl;
    year_set2.remove(d1);    
    std::cout << "year_set2.remove(d1):" << std::endl;    
    std::cout << year_set2;
    std::cout << "year_set2.size():" << "\033[0;31m" << year_set2.size() << "\033[0m" << std::endl << std::endl;
    year_set2 - d2;
    std::cout << "year_set2 - d2:" << std::endl;    
    std::cout << year_set2;
    std::cout << "year_set2.size():" << "\033[0;31m" << year_set2.size() << "\033[0m" << std::endl << std::endl;
    year_set2 - d3;
    std::cout << "year_set2 - d3:" << std::endl;    
    std::cout << year_set2;
    std::cout << "year_set2.size():" << "\033[0;31m" << year_set2.size() << "\033[0m" << std::endl << std::endl;
    year_set2 - d3;
    std::cout << "year_set2 - d3:" << std::endl;    
    std::cout << year_set2;
    std::cout << "year_set2.size():" << "\033[0;31m" << year_set2.size() << "\033[0m" << std::endl << std::endl;
    year_set2 + d6;
    std::cout << "year_set2 + d6:" << std::endl;    
    std::cout << year_set2;
    std::cout << "year_set2.size():" << "\033[0;31m" << year_set2.size() << "\033[0m" << std::endl << std::endl;
    year_set2 + d6;
    std::cout << "year_set2 + d6:" << std::endl;    
    std::cout << year_set2;
    std::cout << "year_set2.size():" << "\033[0;31m" << year_set2.size() << "\033[0m" << std::endl << std::endl;
    year_set2 + d3;
    std::cout << "year_set2 + d3:" << std::endl;    
    std::cout << year_set2;
    std::cout << "year_set2.size():" << "\033[0;31m" << year_set2.size() << "\033[0m" << std::endl << std::endl;

    std::cout << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    std::cout << "\033[48;2;" << 70 << ';' << 70 << ';' << 70 << 'm'<< std::endl << std::endl << std::endl << std::endl << std::endl << std::endl <<   "\033[0m" << std::endl;

    std::cout << "\033[0;31m                 operator+(returns union set), operator-(returns diffrence set), operator^, operator[]\033[0m" << std::endl;
    std::cout << "year_set1:" << std::endl;
    std::cout << year_set1;
    std::cout << "year_set2:" << std::endl;
    std::cout << year_set2;
    std::cout << "year_set3:" << std::endl;
    std::cout << year_set3;
    std::cout << "year_set4:" << std::endl;    
    std::cout << year_set4;
    std::cout << "year_set3[0]:" << year_set3[0] << std::endl;
    std::cout << "year_set3[2]:" << year_set3[2] << std::endl;
    std::cout << "year_set1 + year_set4: " << std::endl << year_set1 + year_set4 << std::endl;    
    std::cout << "year_set1 + year_set4 + year_set3 + year_set2: " << std::endl << year_set1 + year_set4 + year_set3 + year_set2<< std::endl;    
    std::cout << "year_set1 - year_set4: " << std::endl << year_set1 - year_set4 << std::endl;    
    std::cout << "year_set1 - year_set3: " << std::endl << year_set1 - year_set3 << std::endl;    
    std::cout << "year_set1 ^ year_set2: " << std::endl << (year_set1 ^ year_set2) << std::endl;    
    std::cout << "year_set1 ^ year_set3: " << std::endl << (year_set1 ^ year_set3) << std::endl;    

    std::cout << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    std::cout << "\033[48;2;" << 70 << ';' << 70 << ';' << 70 << 'm'<< std::endl << std::endl << std::endl << std::endl << std::endl << std::endl <<   "\033[0m" << std::endl;

    std::cout << "\033[0;31m                                operator! \033[0m" << std::endl;
    year_set1 + d1 + d2 + d3 + d4 + d5 + d6;
    std::cout << "year_set1:" << std::endl;
    std::cout << year_set1;
    std::cout << "!year_set1(complement of the set):" << std::endl;
    std::cout << !year_set1;

    std::cout << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    std::cout << "\033[48;2;" << 70 << ';' << 70 << ';' << 70 << 'm'<< std::endl << std::endl << std::endl << std::endl << std::endl << std::endl <<   "\033[0m" << std::endl;

    std::cout << "\033[0;31m                           more operator! \033[0m" << std::endl;
    year_set4 - d1 - d2 - d3 - d4 - d5 - d6;
    std::cout << "year_set4:" << std::endl;
    std::cout << year_set4;
    std::cout << "year_set4:" << std::endl;
    std::cout << year_set4;
    std::cout << "!year_set4(complement of an empty set):" << std::endl;
    std::cout << !year_set4;

    std::cout << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    std::cout << "\033[48;2;" << 70 << ';' << 70 << ';' << 70 << 'm'<< std::endl << std::endl << std::endl << std::endl << std::endl << std::endl <<   "\033[0m" << std::endl;

    std::cout << "\033[0;31m                           more operator! \033[0m" << std::endl;
    std::cout << "year_set1:" << std::endl;
    std::cout << year_set1;
    std::cout << "!!year_set1(complement of the complement of the set ):" << std::endl;
    std::cout << !!year_set1;

    return 0;
}
#include "DayofYear.h"
#include <iostream>
#include <fstream>

//namespace std {string std::getline(std::istream&, std::string);}

int main() { 
    std::fstream Output; 
    Output.open("Output.txt", std::ios::out);  
    DayOfYear::DayofYear d1(3,5), d2(7,9), d3(11, 13), d4, d5(1,12), d6(12,31);
    DayOfYear::DayofYearSet year_set1 = {d1, d2, d3}, year_set2 = {d2, d1, d4, d5, d3, d4}, year_set3(year_set1), year_set4 ={d3, d6};

    Output << "                   list initializer and coppy constructor, operator<<, operator+ that add elemnt to set" << std::endl;
    Output << "d1:" << std::endl << d1;
    Output << "d2:" << std::endl << d2;
    Output << "d3:" << std::endl << d3;
    Output << "d4:" << std::endl << d4;
    Output << "d5:" << std::endl << d5;
    Output << "d6:" << std::endl << d6 << std::endl;
    Output << "DayofYear set that initialized with list initializer constructor: {d1, d2 ,d3}" << std::endl;
    Output << year_set1;
    Output << "DayofYear set that initialized with list initializer constructor: {d2, d1, d4, d5, d3, d4}" << std::endl;
    Output << year_set2;
    Output << "DayofYear set that initialized with copy constructor: DayofYearSet(year_set1)" << std::endl;
    Output << year_set3;
    Output << "Those have sorted and unique elements therefore operator+(add elemrnt to the set) and list initializer constructor works very well" << std::endl;
    Output << "In addition, the overloaded stream insertion operator was used while they were being printed and works very well" << std::endl << std::endl;

    Output << "Press enter to next Page" << std::endl;
    std::string str;
    std::getline(std::cin, str);
    Output  << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl  << std::endl;

    Output << "                           operator==, operator!=" << std::endl;
    Output << "year_set1:" << std::endl;
    Output << year_set1;
    Output << "year_set3:" << std::endl;    
    Output << year_set3;
    Output << "year_set4:" << std::endl;    
    Output << year_set4;
    Output << "year_set1 == year_set4: " << (year_set1 == year_set4 ? " True " : " False ") << std::endl;    
    Output << "year_set4 == year_set3: " << (year_set4 == year_set3 ? " True " : " False ") << std::endl;    
    Output << "year_set1 == year_set3: " << (year_set1 == year_set3 ? " True " : " False ") << std::endl << std::endl;    
    Output << "year_set4 != year_set3: " << (year_set4 != year_set3 ? " True " : " False ") << std::endl;    
    Output << "year_set1 != year_set3: " << (year_set1 != year_set3 ? " True " : " False ") << std::endl << std::endl;    

    Output << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    Output  << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl  << std::endl;

    Output << "                size, remove, operator-(removes element from the set) operator+(add element to set)" << std::endl;
    Output << "d1:" << d1;
    Output << "d2:" << d2;
    Output << "d3:" << d3;
    Output << "d5:" << d5;
    Output << "d6:" << d6 << std::endl;
    Output << "year_set2:" << std::endl;    
    Output << year_set2;
    Output << "year_set2.size():" << "" << year_set2.size() << "" << std::endl << std::endl;
    year_set2.remove(d5);    
    Output << "year_set2.remove(d5):" << std::endl;    
    Output << year_set2;
    Output << "year_set2.size():" << "" << year_set2.size() << "" << std::endl << std::endl;
    year_set2.remove(d1);    
    Output << "year_set2.remove(d1):" << std::endl;    
    Output << year_set2;
    Output << "year_set2.size():" << "" << year_set2.size() << "" << std::endl << std::endl;
    year_set2 - d2;
    Output << "year_set2 - d2:" << std::endl;    
    Output << year_set2;
    Output << "year_set2.size():" << "" << year_set2.size() << "" << std::endl << std::endl;
    year_set2 - d3;
    Output << "year_set2 - d3:" << std::endl;    
    Output << year_set2;
    Output << "year_set2.size():" << "" << year_set2.size() << "" << std::endl << std::endl;
    year_set2 - d3;
    Output << "year_set2 - d3:" << std::endl;    
    Output << year_set2;
    Output << "year_set2.size():" << "" << year_set2.size() << "" << std::endl << std::endl;
    year_set2 + d6;
    Output << "year_set2 + d6:" << std::endl;    
    Output << year_set2;
    Output << "year_set2.size():" << "" << year_set2.size() << "" << std::endl << std::endl;
    year_set2 + d6;
    Output << "year_set2 + d6:" << std::endl;    
    Output << year_set2;
    Output << "year_set2.size():" << "" << year_set2.size() << "" << std::endl << std::endl;
    year_set2 + d3;
    Output << "year_set2 + d3:" << std::endl;    
    Output << year_set2;
    Output << "year_set2.size():" << "" << year_set2.size() << "" << std::endl << std::endl;

    Output << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    Output  << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl  << std::endl;

    Output << "                 operator+(returns union set), operator-(returns diffrence set), operator^, operator[]" << std::endl;
    Output << "year_set1:" << std::endl;
    Output << year_set1;
    Output << "year_set2:" << std::endl;
    Output << year_set2;
    Output << "year_set3:" << std::endl;
    Output << year_set3;
    Output << "year_set4:" << std::endl;    
    Output << year_set4;
    Output << "year_set3[0]:" << year_set3[0] << std::endl;
    Output << "year_set3[2]:" << year_set3[2] << std::endl;
    Output << "year_set1 + year_set4: " << std::endl << year_set1 + year_set4 << std::endl;    
    Output << "year_set1 + year_set4 + year_set3 + year_set2: " << std::endl << year_set1 + year_set4 + year_set3 + year_set2<< std::endl;    
    Output << "year_set1 - year_set4: " << std::endl << year_set1 - year_set4 << std::endl;    
    Output << "year_set1 - year_set3: " << std::endl << year_set1 - year_set3 << std::endl;    
    Output << "year_set1 ^ year_set2: " << std::endl << (year_set1 ^ year_set2) << std::endl;    
    Output << "year_set1 ^ year_set3: " << std::endl << (year_set1 ^ year_set3) << std::endl;    

    Output << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    Output  << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl  << std::endl;

    Output << "                                operator! " << std::endl;
    year_set1 + d1 + d2 + d3 + d4 + d5 + d6;
    Output << "year_set1:" << std::endl;
    Output << year_set1;
    Output << "!year_set1(complement of the set):" << std::endl;
    Output << !year_set1;

    Output << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    Output  << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl  << std::endl;

    Output << "                           more operator! " << std::endl;
    year_set4 - d1 - d2 - d3 - d4 - d5 - d6;
    Output << "year_set4:" << std::endl;
    Output << year_set4;
    Output << "year_set4:" << std::endl;
    Output << year_set4;
    Output << "!year_set4(complement of an empty set):" << std::endl;
    Output << !year_set4;

    Output << "Press enter to next Page" << std::endl;
    std::getline(std::cin, str);
    Output  << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl  << std::endl;

    Output << "                           more operator! " << std::endl;
    Output << "year_set1:" << std::endl;
    Output << year_set1;
    Output << "!!year_set1(complement of the complement of the set ):" << std::endl;
    Output << !!year_set1;

    Output.close();

    return 0;
}
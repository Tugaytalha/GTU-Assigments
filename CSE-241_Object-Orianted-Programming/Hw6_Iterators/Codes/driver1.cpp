#include "pfarray.h"
#include <algorithm>


int main() {
    typedef PFArraySavitch::PFArray<int> pint;
    typedef PFArraySavitch::PFArray<char> pchar;
    pint pf_int;
    pchar pf_char;
    int i = 0;

    std::cout << "Empty: " << (pf_int.empty() ? "True" : "False") << "  Size: " << pf_int.size() <<  std::endl;
    pf_int.add(110);
    pf_int.add(115);
    pf_int.add(97);
    pf_int.add(111);
    pf_int.add(109);
    pf_int.add(105);
    std::cout << "PFArray: " << pf_int << std::endl;
    std::sort(pf_int.begin(), pf_int.end());
    std::cout << "PFArray after sort: " << pf_int << std::endl;
    std::cout << "*(find(begin(), end(), 111)): " << *(std::find(pf_int.begin(), pf_int.end(), 111)) << std::endl;
    std::cout << "*(++find(begin(), end(), 111)): " << *(++std::find(pf_int.begin(), pf_int.end(), 111)) << std::endl;
    std::cout << "Before erase  " << "Empty: " << (pf_int.empty() ? "True" : "False") << "  Size: " << pf_int.size() <<  std::endl;
    pf_int.erase(std::find(pf_int.begin(), pf_int.end(), 111));  // uses cbegin and cend
    std::cout << "After erase  " << "Empty: " << (pf_int.empty() ? "True" : "False") << "  Size: " << pf_int.size() <<  std::endl;
    std::cout << "PFArray: " << pf_int << std::endl;
    std::cout << "for_each based PFArray print: {";
    std::for_each(pf_int.begin(), pf_int.end(), PFArraySavitch::print<int>); std::cout << "}" << std::endl;

    //  Try except part
    try
    {
        pf_int[70];
    }
    catch(const std::out_bound& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }


    std::cout << "Empty: " << (pf_char.empty() ? "True" : "False") << "  Size: " << pf_char.size() <<  std::endl;
    pf_char.add(110);
    pf_char.add(115);
    pf_char.add(97);
    pf_char.add(111);
    pf_char.add(109);
    pf_char.add(105);
    std::cout << "PFArray: " << pf_char << std::endl;
    std::stable_sort(pf_char.begin(), pf_char.end());
    std::cout << "PFArray after sort: " << pf_char << std::endl;
    std::cout << "*(find(begin(), end(), 111)): " << *(std::find(pf_char.begin(), pf_char.end(), 111)) << std::endl;
    std::cout << "*(++find(begin(), end(), 111)): " << *(++std::find(pf_char.begin(), pf_char.end(), 111)) << std::endl;
    std::cout << "Before erase  " << "Empty: " << (pf_char.empty() ? "True" : "False") << "  Size: " << pf_char.size() <<  std::endl;
    pf_char.erase(std::find(pf_char.begin(), pf_char.end(), 111));  // uses cbegin and cend
    std::cout << "After erase  " << "Empty: " << (pf_char.empty() ? "True" : "False") << "  Size: " << pf_char.size() <<  std::endl;
    std::cout << "PFArray: " << pf_char << std::endl;
    std::cout << "for_each based PFArray print: {";
    std::for_each(pf_char.begin(), pf_char.end(), PFArraySavitch::print<char>); std::cout << "}" << std::endl;

    //  Try except part
    try
    {
        pf_char[70];
    }
    catch(const std::out_bound& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }


}
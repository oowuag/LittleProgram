// istream constructor
#include <iostream>     // std::ios, std::istream, std::cout
#include <fstream>      // std::filebuf

int test1()
{
    std::istream *pIs = NULL;

    std::filebuf fb;
    if (fb.open ("text.txt",std::ios::in))
    {
        std::istream is(&fb);
        pIs = new std::istream(&fb);
        while (*pIs)
            std::cout << char(is.get());
        fb.close();
    }
    if (pIs) {
        delete pIs;
    }
    return 0;
}
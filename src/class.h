#include <iostream>
#include <array>

template <typename T>
T myMax(T x, T y)
{
    return (x > y) ? x : y;
}


template <class T, class U> class A {
    T x;
    U y;
 
public:
    A() { std::cout << "Constructor Called" << std::endl; }
};
 
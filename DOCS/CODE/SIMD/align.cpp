#include <iostream>

char D[2] __attribute__((aligned(8)));

struct A
{
    short s[3];
} __attribute__((aligned(8)));

struct B
{
    short s[3];
};

struct C
{
    short s[3];
} __attribute__((aligned(1)));

struct E
{
    char a;
    int b;
} __attribute__((packed));

int main()
{
    std::cout << "sizeof(A) = " << sizeof(A) << std::endl;
    std::cout << "sizeof(B) = " << sizeof(B) << std::endl;
    std::cout << "sizeof(C) = " << sizeof(C) << std::endl;
    std::cout << "sizeof(D) = " << sizeof(D) << std::endl;
    std::cout << "sizeof(E) = " << sizeof(E) << std::endl;
    return 0;
}

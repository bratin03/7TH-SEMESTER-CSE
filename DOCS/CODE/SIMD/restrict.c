

void foo(int *restrict a, int *restrict b, int *c)
{
    *a = 42;
    *b = 23;
    c = a;
}

int main()
{
    int a = 1;
    int b = 2;
    int c = 3;
    foo(&a, &b, &c);
    return 0;
}
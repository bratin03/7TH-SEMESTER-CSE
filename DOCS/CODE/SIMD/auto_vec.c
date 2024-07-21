#define SIZE (1 << 16)
void simpleLoop(double *a, double *b)
{
    for (int i = 0; i < SIZE; i++)
    {
        a[i] += b[i];
    }
}

int main()
{
    double *a = (double *)malloc(SIZE * sizeof(double));
    double *b = (double *)malloc(SIZE * sizeof(double));
    simpleLoop(a, b);
    free(a);
    free(b);
    return 0;
}
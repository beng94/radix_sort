#include <omp.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;

    if(end.tv_sec - start.tv_sec == 0)
    {
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_nsec = ((end.tv_sec - start.tv_sec)*1000000000) + end.tv_nsec - start.tv_nsec;
    }

    return temp;
}

int max_item(int* array, const size_t size)
{
    int max = array[0];
    for(int i = 1; i<size; i++)
    {
        if(array[i] > max) max = array[i];
    }

    return max;
}

void radix_sort(int* array, const size_t size)
{
    const int base = 10;

    int *tmp_array = (int*)malloc(sizeof(int)*size);
    int *bucket = (int*)malloc(sizeof(int)*base);
    for(int i = 0; i<base; i++) bucket[i] = 0;

    int max = max_item(array, size);
    int pow = 1;

    while(max / pow > 0)
    {
        for(int i = 0; i<size; i++)
        {
            int digit = (array[i] / pow) % base;
            bucket[digit]++;
        }

        for(int i = 1; i<base; i++) bucket[i] += bucket[i-1];

        for(int i = size-1; i>=0; i--)
        {
            int digit = (array[i] / pow) % base;
            tmp_array[--bucket[digit]] = array[i];
        }

        for(int i = 0; i<size; i++) array[i] = tmp_array[i];

        pow *= base;
    }

    free(tmp_array);
    free(bucket);
}

void radix_sort_parallel(int* array, const size_t size)
{
    const int base = 10;

    int *tmp_array = (int*)malloc(sizeof(int)*size);
    int *bucket = (int*)malloc(sizeof(int)*base);

    #pragma omp parallel for
    for(int i = 0; i<base; i++) bucket[i] = 0;

    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val)
    for(unsigned int i= 0; i < size; i++)
    {
        if(array[i] > max_val) max_val = array[i];
    }

    int pow = 1;
    while(max_val / pow > 0)
    {
        #pragma omp parallel for
        for(int i = 0; i<size; i++)
        {
            //int digit = (array[i] / pow) % base;
            //bucket[digit]++;
            bucket[(array[i] / pow) % base]++;
        }

        for(int i = 0; i<base; i++) bucket[i] += bucket[i-1];

        #pragma omp parallel for
        for(int i = size-1; i>=0; i--)
        {
            //int digit = (array[i] / pow) % base;
            //tmp_array[--bucket[digit]] = array[i];
            tmp_array[--bucket[(array[i] / pow) % base]] = array[i];
        }

        #pragma omp parallel for
        for(int i = 0; i<size; i++) array[i] = tmp_array[i];

        pow *= base;
    }

    free(tmp_array);
    free(bucket);
}

int* generate_rand_array(size_t size)
{
    srand(time(NULL));

    int* array = (int*)malloc(sizeof(int)*size);
    if(array == NULL) return NULL;

    for(int i = 0; i<size; i++) array[i] = rand() % 10;

    return array;
}

int main()
{
    FILE *f = fopen("outtiny.txt", "w");
    if(f == NULL)
    {
        printf("Could not initialize file\n");
        return -1;
    }

    fprintf(f, "\t1 core\t4 core\n");

    for(int N = 1; N < 10000; N += 1)
    {
        if(N % 10000 == 0) printf ("%d\n", N);
        struct timespec start, end;

        int* array = generate_rand_array(N);
        if(array == NULL)
        {
            printf("Not enough memory\n");
            return -1;
        }

        clock_gettime(CLOCK_MONOTONIC, &start);
        radix_sort(array, N);
        clock_gettime(CLOCK_MONOTONIC, &end);

        fprintf(f, "%d\t", N);
        fprintf(f, "%lf\t",
                (double)(diff(start, end).tv_nsec) / 1000000000.0);

        clock_gettime(CLOCK_MONOTONIC, &start);
        radix_sort_parallel(array, N);
        clock_gettime(CLOCK_MONOTONIC, &end);

        fprintf(f, "%lf\n",
                (double)(diff(start, end).tv_nsec) / 1000000000.0);

        free(array);
    }

    fclose(f);

    return 0;
}

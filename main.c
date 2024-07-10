#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "FW_1.3.1_Lin64/fwBase.h"
#include "FW_1.3.1_Lin64/fwSignal.h"
#include "FW_1.3.1_Lin64/fwImage.h"

#define VER_X
#undef VER_X

#ifndef RAND_MAX
#define RAND_MAX ((int)((unsigned)~0 >> 1))
#endif

/* A = 11 * 5 * 10 = 550 */
#define A 550

#define IS_WHOLE_PART_EVEN(num) (((((int)(num)) % 2) == 0))
#define SET_SEED_POINT(pSeed, seed) ((pSeed) = (seed))

#define RANDOM_DOUBLE(low, high, pSeed) ((low) + ((Fw32f)rand_r(pSeed) / ((Fw32f)(RAND_MAX) + 1)) * ((high) - (low)))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void insertion_sort(Fw32f *array, unsigned int size)
{
    unsigned int i, j;
    Fw32f x;

    for (i = 1; i < size; i++)
    {
        x = array[i];
        j = i;

        while (j > 0 && array[j - 1] > x)
        {
            array[j] = array[j - 1];
            j -= 1;
        }

        array[j] = x;
    }
}

/* X(B) = 1 + ((550 mod 47) mod B) = 1 + (33 mod B)
 * X(7) = 1 + (33 mod 7) = 6
 * X(8) = 1 + (33 mod 8) = 2
 * X(6) = 1 + (33 mod 6) = 4 */

int main(int argc, char *argv[])
{
    unsigned int i, j, pSeed, N, N2, numThreads;
    long deltaMS;
    Fw32f minimum, X, *M1, *M2, *M2Copy;
    struct timeval T1, T2;

    N = atoi(argv[1]);          // N равен 1-му параметру командной строки
    numThreads = atoi(argv[2]); // numThreads равен 2-му параметру командной строки

    fwSetNumThreads(numThreads);

    N2 = N / 2;
    M2Copy = fwsMalloc_32f(N2 + 1);
    M1 = fwsMalloc_32f(N);
    M2 = fwsMalloc_32f(N2);
    M2Copy[0] = 0.0;

    gettimeofday(&T1, NULL); // запомнить текущее время T1 */

#ifndef VER_X
    for (i = 0; i < 100; i++) /* 100 экспериментов */
#else
    for (i = 0; i < 5; i++)
#endif
    {
        SET_SEED_POINT(pSeed, i);

        for (j = 0; j < N; j++) // [1; A]
            M1[j] = RANDOM_DOUBLE(1, A, &pSeed);

        for (j = 0; j < N2; j++) // [A; 10*A]
            M2[j] = RANDOM_DOUBLE(A, 10 * A, &pSeed);

        fwsDivC_32f_I((Fw32f)M_E, M1, N);
        fwsCbrt_32f_A24(M1, M1, N);

        fwsCopy_32f(M2, M2Copy, N2); // Копия массива M2

        fwsAdd_32f(M2 + 1, M2Copy, M2 + 1, N2); // 2: Элементы нового массива равны модулю косинуса от суммы текущего и прошлого элементов массива
        fwsCos_32f_A24(M2, M2, N2);
        fwsAbs_32f_I(M2, N2);

        fwsMaxEvery_32f_I(M1, M2, N2); // 3) Merge. 4: Выбор большего

        insertion_sort(M2, N2); // 4) Sort. 6: Сортировка вставками

        X = 0;
        minimum = M2[0];
        for (j = 0; j < N2; j++) // 5) Reduce. Сумма синусов элементов, которые при делении на минимум дают чётное число в целой части
            if (IS_WHOLE_PART_EVEN(M2[j] / minimum))
                X += sin(M2[j]);

#ifdef VER_X
        printf("X = %f\n", X);
#endif
    }
    gettimeofday(&T2, NULL); // запомнить текущее время T2
    deltaMS = 1000 * (T2.tv_sec - T1.tv_sec) + (T2.tv_usec - T1.tv_usec) / 1000;
    printf("\nN=%d. Milliseconds passed: %ld\n", N, deltaMS); // T2 - T1

    fwsFree(M2Copy);
    fwsFree(M2);
    fwsFree(M1);

    return 0;
}
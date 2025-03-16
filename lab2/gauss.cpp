#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <locale.h>
#include <tlhelp32.h>

#define MAX_THREADS 10

int max_threads = 1;
double **matrix;
int n;

CRITICAL_SECTION criticalSection;

void print_thread_count() {
    HANDLE hThreadSnap;
    THREADENTRY32 te32;
    int threadCount = 0;
    DWORD currentProcessId = GetCurrentProcessId();

    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        printf("Error. couldnt create snapshot.\n");
        return;
    }

    te32.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hThreadSnap, &te32)) {
        do {
            if (te32.th32OwnerProcessID == currentProcessId) {
                threadCount++;
            }
        } while (Thread32Next(hThreadSnap, &te32));
    } else {
        printf("error. couldnt fetch thread information.\n");
    }

    CloseHandle(hThreadSnap);

    printf("number of threads in current process: %d\n", threadCount);
}

void gaussian_elimination(int thread_id) {
    int i, j, k;
    double factor;

    for (i = 0; i < n; i++) {
        if (i % max_threads == thread_id) {
            for (j = i + 1; j < n; j++) {
                factor = matrix[j][i] / matrix[i][i];
                for (k = i; k < n + 1; k++) {
                    matrix[j][k] -= factor * matrix[i][k];
                }
            }
        }
    }
}

DWORD WINAPI thread_function(LPVOID lpParam) {
    int thread_id = *(int *)lpParam;
    gaussian_elimination(thread_id);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Using: %s <Maximum threads>\n", argv[0]);
        return 1;
    }

    max_threads = atoi(argv[1]);
    if (max_threads < 1 || max_threads > MAX_THREADS) {
        printf("maximum threads should be 1 -  %d\n", MAX_THREADS);
        return 1;
    }

    print_thread_count();

    printf("enter number of equasions: ");
    scanf("%d", &n);

    matrix = (double **)malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        matrix[i] = (double *)malloc((n + 1) * sizeof(double));
    }

    printf("enter matrix coefficients:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n + 1; j++) {
            scanf("%lf", &matrix[i][j]);
        }
    }

    InitializeCriticalSection(&criticalSection);

    HANDLE threads[MAX_THREADS];
    int thread_ids[MAX_THREADS];

    for (int i = 0; i < max_threads; i++) {
        thread_ids[i] = i;
        threads[i] = CreateThread(NULL, 0, thread_function, &thread_ids[i], 0, NULL);
        if (threads[i] == NULL) {
            printf("Thread creation error %d\n", i);
            return 1;
        }
    }

    WaitForMultipleObjects(max_threads, threads, TRUE, INFINITE);

    for (int i = 0; i < max_threads; i++) {
        CloseHandle(threads[i]);
    }

    double *x = (double *)malloc(n * sizeof(double));
    for (int i = n - 1; i >= 0; i--) {
        x[i] = matrix[i][n];
        for (int j = i + 1; j < n; j++) {
            x[i] -= matrix[i][j] * x[j];
        }
        x[i] /= matrix[i][i];
    }

    printf("Решение системы:\n");
    for (int i = 0; i < n; i++) {
        printf("x[%d] = %lf\n", i, x[i]);
    }

    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(x);

    DeleteCriticalSection(&criticalSection);

    return 0;
}
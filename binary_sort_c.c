#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Binary insertion sort: binary-search for position, then shift one by one.
   This matches binary_sort_py.py; no library sort or memory-move shortcuts. */
static void binary_insertion_sort(int *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        int key = a[i];
        size_t lo = 0;
        size_t hi = i;

        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2;
            if (key < a[mid]) {
                hi = mid;
            } else {
                lo = mid + 1; /* Equal values remain in original order. */
            }
        }

        size_t j = i;
        while (j > lo) {
            a[j] = a[j - 1];
            j--;
        }
        a[lo] = key;
    }
}

/* Same 32-bit pseudo-random generator as the Python implementation. */
static void fill_data(int *a, size_t n, uint32_t seed) {
    uint32_t state = seed;
    for (size_t i = 0; i < n; i++) {
        state = state * UINT32_C(1664525) + UINT32_C(1013904223);
        a[i] = (int)((state >> 8) % 1000000u);
    }
}

/* An order-sensitive checksum to validate that C and Python match. */
static uint64_t checksum(const int *a, size_t n) {
    uint64_t h = UINT64_C(14695981039346656037);
    for (size_t i = 0; i < n; i++) {
        h ^= (uint64_t)a[i];
        h *= UINT64_C(1099511628211);
    }
    return h;
}

static int is_sorted(const int *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (a[i - 1] > a[i]) return 0;
    }
    return 1;
}

static double seconds_between(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) +
           (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <size> <seed>\n", argv[0]);
        return 2;
    }

    char *end1 = NULL;
    char *end2 = NULL;
    unsigned long long count = strtoull(argv[1], &end1, 10);
    unsigned long seed_value = strtoul(argv[2], &end2, 10);
    if (*argv[1] == '\0' || *argv[2] == '\0' || *end1 != '\0' ||
        *end2 != '\0' || count == 0 || count > 1000000ULL ||
        seed_value > UINT32_MAX) {
        fprintf(stderr, "Invalid inputs (size: 1..1000000, seed: 0..4294967295).\n");
        return 2;
    }

    size_t n = (size_t)count;
    int *a = malloc(n * sizeof(*a));
    if (a == NULL) {
        fprintf(stderr, "Could not allocate array.\n");
        return 1;
    }

    fill_data(a, n, (uint32_t)seed_value);  /* Not timed. */
    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("clock_gettime");
        free(a);
        return 1;
    }
    binary_insertion_sort(a, n);            /* Only this is timed. */
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        perror("clock_gettime");
        free(a);
        return 1;
    }

    /* CSV-like output: milliseconds, 64-bit checksum (hex), sorted flag. */
    printf("%.6f,%016" PRIx64 ",%d\n",
           1000.0 * seconds_between(start, end), checksum(a, n),
           is_sorted(a, n));
    free(a);
    return 0;
}

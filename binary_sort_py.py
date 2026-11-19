"""Python implementation of exactly the same binary insertion sort as C.

Usage: python3 binary_sort_py.py 4000 12345
Output: <elapsed_milliseconds>,<64-bit_output_checksum>,<sorted_flag>
"""

import argparse
import time

MASK32 = 0xFFFFFFFF
MASK64 = 0xFFFFFFFFFFFFFFFF


def make_data(n: int, seed: int) -> list[int]:
    """Match C's unsigned 32-bit linear congruential generator."""
    state = seed
    values = []
    for _ in range(n):
        state = (state * 1664525 + 1013904223) & MASK32
        values.append((state >> 8) % 1_000_000)
    return values


def binary_insertion_sort(a: list[int]) -> None:
    """Stable in-place sort using binary search and individual shifts."""
    for i in range(1, len(a)):
        key = a[i]
        lo, hi = 0, i

        while lo < hi:
            mid = lo + (hi - lo) // 2
            if key < a[mid]:
                hi = mid
            else:
                lo = mid + 1  # Equal values stay in original order.

        j = i
        while j > lo:
            a[j] = a[j - 1]
            j -= 1
        a[lo] = key


def checksum(a: list[int]) -> str:
    """Same order-sensitive 64-bit checksum as in C."""
    h = 14695981039346656037
    for value in a:
        h = ((h ^ value) * 1099511628211) & MASK64
    return f"{h:016x}"


def main() -> None:
    parser = argparse.ArgumentParser(description="Time Python binary insertion sort")
    parser.add_argument("size", type=int)
    parser.add_argument("seed", type=int)
    args = parser.parse_args()
    if not 1 <= args.size <= 1_000_000 or not 0 <= args.seed <= MASK32:
        parser.error("size must be 1..1000000; seed must be 0..4294967295")

    a = make_data(args.size, args.seed)  # Not timed.
    start_ns = time.perf_counter_ns()
    binary_insertion_sort(a)             # Only this is timed.
    elapsed_ms = (time.perf_counter_ns() - start_ns) / 1_000_000
    ok = int(all(a[i - 1] <= a[i] for i in range(1, len(a))))
    print(f"{elapsed_ms:.6f},{checksum(a)},{ok}")


if __name__ == "__main__":
    main()

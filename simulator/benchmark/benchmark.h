#ifndef QSORT_BENCH_H
#define QSORT_BENCH_H

#define MAXARRAY 8000

struct myStringStruct {
  char qstring[128];
};

extern struct myStringStruct array[MAXARRAY];
void qsort_bench();

#endif
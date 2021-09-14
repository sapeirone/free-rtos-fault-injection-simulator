#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "benchmark.h"

#pragma warning(disable : 4996) // _CRT_SECURE_NO_WARNINGS
#define UNLIMIT
#define MAXARRAY 8000

struct myStringStruct {
  char qstring[128];
};

static int compare(const void *elem1, const void *elem2);

void qsort_bench() {
  struct myStringStruct array[MAXARRAY];
  FILE *fp;
  int count=0;

  fp = fopen("simulator/input_data/input_small.dat", "r");
  if(fp == NULL){
    fprintf(stderr, "Error fopen in qsort_bench\n");
  }

  while((count < MAXARRAY) && (fscanf(fp, "%s", &array[count].qstring) == 1)) {
    count++;
  }

  fclose(fp);

  fprintf(stdout, "Sorting %d elements.\n", count);
  qsort(array, count, sizeof(struct myStringStruct), compare);
  
  for(int i = 0; i < count; i++){
    if((i%1000) == 0 || i == count-1)
      fprintf(stdout, "array[%d] = %s\n", i, array[i].qstring);
  }

  return;
}

static int compare(const void *elem1, const void *elem2){
  int result;
  
  result = strcmp((*((struct myStringStruct *)elem1)).qstring, (*((struct myStringStruct *)elem2)).qstring);

  return (result < 0) ? 1 : ((result == 0) ? 0 : -1);
}

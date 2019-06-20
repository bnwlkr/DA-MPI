#include "dampi.h"
#include "profile.h"

void DAMPI_Init(int proc, int n) {
  profile(proc, n);
}


void DAMPI_Run(void (*f)(void*), int suitcase_sz) {
  
}

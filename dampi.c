#include "dampi.h"
#include "profile.h"

void DAMPI_Start(int proc, int n, dampi_func f, int sc_size) {
  profile(proc, n, f, sc_size);
}

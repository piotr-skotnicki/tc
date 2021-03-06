int main()
{
#if 0
# define _PB_N 4000
#else
  int _PB_N;
#endif
  
  double A[_PB_N][_PB_N];
  double b[_PB_N];
  double x[_PB_N];
  double y[_PB_N];
  double w;

#pragma scop
  for (int i = 0; i < _PB_N; i++) {
    for (int j = 0; j < i; j++) {
S1:   w = A[i][j];
      for (int k = 0; k < j; k++) {
S2:     w -= A[i][k] * A[k][j];
      }
S3:   A[i][j] = w / A[j][j];
    }
    for (int j = i; j < _PB_N; j++) {
S4:   w = A[i][j];
      for (int k = 0; k < i; k++) {
S5:     w -= A[i][k] * A[k][j];
      }
S6:   A[i][j] = w;
    }
  }

  for (int i = 0; i < _PB_N; i++) {
S7: w = b[i];
    for (int j = 0; j < i; j++)
S8:   w -= A[i][j] * y[j];
S9: y[i] = w;
  }

  for (int i = _PB_N-1; i >= 0; i--) {
S10:w = y[i];
    for (int j = i+1; j < _PB_N; j++) {
S11:  w -= A[i][j] * x[j];
    }
S12:x[i] = w / A[i][i];
  }
#pragma endscop
}

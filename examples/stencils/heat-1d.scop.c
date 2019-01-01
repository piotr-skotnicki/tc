int main()
{
#if 0
# define N 3600000
# define T 1000
#else
  int T;
  int N;
#endif
  
  int A[2][N+2];
  
#pragma scop
  for (int t = 0; t < T; ++t) {
    for (int i = 1; i < N+1; ++i) {
S1:   A[(t+1)%2][i] = 0.125 * (A[t%2][i+1] - 2.0 * A[t%2][i] + A[t%2][i-1]);
    }
  }
#pragma endscop
}

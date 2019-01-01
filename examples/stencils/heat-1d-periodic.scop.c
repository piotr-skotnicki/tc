int main()
{
#if 0
# define N 3600000
# define T 1000
#else
  int T;
  int N;
#endif
  
  int A[T+1][N];
  
#pragma scop
  for (int t = 0; t < T; ++t) {
    for (int i = 0; i < N; ++i) {
S1:   A[t+1][i] = 0.125 * (A[t][i == N-1 ? 0 : i+1] - 2.0 * A[t][i] + A[t][i == 0 ? N-1 : i-1]);
    }
  }
#pragma endscop
}

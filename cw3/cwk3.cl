// Kernel for matrix multiplication.
__kernel
void matrix_vector_mult(__global const float *M, __global const float *x, __global float *y, const int N) {
    int gid = get_global_id(0);
    float sum = 0.0f;
    for (int j = 0; j < N; j++) {
        sum += M[gid*N+j] * x[j];
    }
    y[gid] = sum;
}

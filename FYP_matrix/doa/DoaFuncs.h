#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_sf_expint.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_complex_math.h>

#include <fftw3.h>

struct COMPLEX{
	double re;
	double im;
};

#define MAXINTVALUE	1000000
#define epsilon6	(float)1/1000000
#ifdef  min
#undef  min
#endif
#define min(a, b)       ((a) < (b) ? (a) : (b))

#ifdef  max
#undef  max
#endif
#define max(a, b)       ((a) < (b) ? (b) : (a))

void array_zeros(float* data, int size);
void short_to_float_array(float *des, short *src, int dessize, int srcsize);
void float_to_double_array(double *des, float *src, int dessize, int srcsize);
void double_to_float_array(float *des, double *src, int dessize, int srcsize);
void real2complex(float *data, int N, COMPLEX *complex);
void double2complex(double *data, int N, COMPLEX *complex);
int complex2fftwcomplex(COMPLEX *complexvec, fftw_complex *realvec, int length);
COMPLEX number_multiplication(COMPLEX complex1, COMPLEX complex2);
COMPLEX number_conj_multiplication(COMPLEX complex1, COMPLEX complex2);
float number_abs(COMPLEX complex);
COMPLEX conj(COMPLEX ComplexVector);
void conj_N(COMPLEX* ComplexVector, int Npts, COMPLEX* result);
void array_multiplication(COMPLEX *arr1, COMPLEX *arr2, int size, COMPLEX* result);
void conj_array_multiplication(COMPLEX *arr1, COMPLEX *arr2, int size, COMPLEX* result);
void denormalize(COMPLEX *G12, int size, COMPLEX *G);
void denormalize_conj_array_multiplication(COMPLEX *arr1, COMPLEX *arr2, int size, COMPLEX *G,float *pSpecWeights);
//void getrealafterIFFT(COMPLEX *G, int size, COMPLEX *IFFT, float* g);
//void getrealafterIFFT(COMPLEX *G, int size,fftw_complex *fftwin, double *IFFT, float* g);
void fftshift(float *x, int size, float *y);
void flipud(float *x, int size, float *y);
float GetMaxValueIndex(float *g, int first, int last);
void GetArrayValue(float *wave, int first, int last, float* alignedWave);
float *array_prepadding(float *wave, int size, int padding);
float *array_postpadding(float *wave, int size, int padding);
void array_plus(float *x1, float *x2, float *plus, int size);
int sign(float U);

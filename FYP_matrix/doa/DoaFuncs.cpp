#include "DoaFuncs.h"


void array_zeros(float* data, int size)
{
	memset(data, 0, size*sizeof(float));
}

void short_to_float_array(float *des, short *src, int dessize, int srcsize)
{
	if (srcsize < dessize)
	{
		for(int i = 0; i < srcsize; i++)
		{
			des[i] = (float)src[i]/32767;
		}
		for(int i = srcsize; i < dessize; i++)
		{
			des[i] = 0;
		}
	}
	else
	{
		for(int i = 0; i < dessize; i++)
		{
			des[i] = (float)src[i]/32767;
		}
	}
}

void float_to_double_array(double *des, float *src, int dessize, int srcsize)
{
	if (srcsize < dessize)
	{
		for(int i = 0; i < srcsize; i++)
		{
			des[i] = (double)src[i];
		}
		for(int i = srcsize; i < dessize; i++)
		{
			des[i] = 0;
		}
	}
	else
	{
		for(int i = 0; i < dessize; i++)
		{
			des[i] = (double)src[i];
		}
	}
}

void double_to_float_array(float *des, double *src, int dessize, int srcsize)
{
	if (srcsize < dessize)
	{
		for(int i = 0; i < srcsize; i++)
		{
			des[i] = (float)src[i];
		}
		for(int i = srcsize; i < dessize; i++)
		{
			des[i] = 0;
		}
	}
	else
	{
		for(int i = 0; i < dessize; i++)
		{
			des[i] = (float)src[i];
		}
	}
}

void real2complex(float *data, int N, COMPLEX *complex)
{
	for (int k=0; k<N-1; k++) {
        if (k==0 || k== N/2) {
            complex[k].re = data[k];
			complex[k].im = 0;			
        }
        else if (k < N/2) {
			complex[k].re = data[k];
			complex[k].im = data[N-k];
        }
        else {
            complex[k].re = data[N-k];
			complex[k].im = -data[k];		
        }		
    }
	complex[N-1].re = data[1];
	complex[N-1].im = -data[N-1];
}

void double2complex(double *data, int N, COMPLEX *complex)
{
	for (int k=0; k<N-1; k++) {
        if (k==0 || k== N/2) {
            complex[k].re = data[k];
			complex[k].im = 0;			
        }
        else if (k < N/2) {
			complex[k].re = data[k];
			complex[k].im = data[N-k];
        }
        else {
            complex[k].re = data[N-k];
			complex[k].im = -data[k];		
        }		
    }
	complex[N-1].re = data[1];
	complex[N-1].im = -data[N-1];
}

int complex2fftwcomplex(COMPLEX *complexvec, fftw_complex *realvec, int length)
{
	int i;
	
	for (i=0;i<length;i++) {
		realvec[i][0] = complexvec[i].re;
		realvec[i][1] = complexvec[i].im;
	}  
	return 0;
}

COMPLEX number_multiplication(COMPLEX complex1, COMPLEX complex2)
{
	COMPLEX result;
	result.re = complex1.re*complex2.re - complex1.im*complex2.im;
	result.im = complex1.re*complex2.im + complex1.im*complex2.re;
	return result;
}

COMPLEX number_conj_multiplication(COMPLEX complex1, COMPLEX complex2)
{
	COMPLEX result;
	result.re = complex1.re*complex2.re - complex1.im*(-complex2.im);
	result.im = complex1.re*(-complex2.im) + complex1.im*complex2.re;
	return result;
}

float number_abs(COMPLEX complex)
{
	return sqrt(complex.re*complex.re + complex.im*complex.im);
}

COMPLEX conj(COMPLEX ComplexVector)
{
	COMPLEX result;
	result.re = ComplexVector.re;
	result.im = -ComplexVector.im;
	return result;
}


void conj_N(COMPLEX* ComplexVector, int Npts, COMPLEX* result)
{
	int i;
	for (i=0;i<Npts;i++) 
	{
		result[i].re = ComplexVector[i].re;
		result[i].im = -ComplexVector[i].im;
	}
}

void array_multiplication(COMPLEX *arr1, COMPLEX *arr2, int size, COMPLEX* result)
{
	for(int i = 0; i < size; i++)
	{
		result[i] = number_multiplication(arr1[i], conj(arr2[i]));
	}
}

void conj_array_multiplication(COMPLEX *arr1, COMPLEX *arr2, int size, COMPLEX* result)
{
	for(int i = 0; i < size; i++)
	{
		//result[i] = number_multiplication(arr1[i], arr2[i]);
		result[i] = number_conj_multiplication(arr1[i], arr2[i]);
	}
}

void denormalize(COMPLEX *G12, int size, COMPLEX *G)
{
	float dorm;
	for(int i = 0; i < size; i++)
	{
		dorm = max(number_abs(G12[i]), epsilon6);
		G[i].re = G12[i].re/dorm;
		G[i].im = G12[i].im/dorm;
	}
}

void denormalize_conj_array_multiplication(COMPLEX *arr1, COMPLEX *arr2, int size, COMPLEX *G, float *pSpecWeights)
{
	float dorm;
	for(int i = 0; i < size; i++)
	{
		G[i] = number_conj_multiplication(arr1[i], arr2[i]);
		dorm = max(number_abs(G[i]), epsilon6);
		G[i].re = G[i].re/dorm;
		G[i].im = G[i].im/dorm;
		//G[i].re = pSpecWeights[i]*pSpecWeights[i]* G[i].re;
		//G[i].im = pSpecWeights[i]*pSpecWeights[i]*G[i].im;
	}
}

/*
void getrealafterIFFT(COMPLEX *G, int size, COMPLEX *IFFT, float* g)
{
	ifft_C(G, size, IFFT, size);
	complex2real(IFFT, g, size);
}
*/
void getrealafterIFFT(COMPLEX *G, int size,fftw_complex *fftwin, double *IFFT, float* g)
{
	complex2fftwcomplex(G, fftwin, size);
	fftw_plan p = fftw_plan_dft_c2r_1d(size, fftwin, IFFT, FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);
	double_to_float_array(g, IFFT, size, size);
}

void fftshift(float *x, int size, float *y)
{
	int size2 = size/2;
	memcpy(y, x + size2, size2*sizeof(float)); 
	memcpy(y + size2, x, size2*sizeof(float));
}

void flipud(float *x, int size, float *y)
{
	for(int i = 0; i < size; i++)
		y[size - i - 1] = x[i];
}

float GetMaxValueIndex(float *g, int first, int last)
{
	float max = 0;
	int index = 0;
	for(int i = first; i <= last; i++)
	{
		if (g[i] > max)
		{
			max = g[i];
			index = i;
		}
	}
	if (index == 0)
		return 0;
	return index - first;
}

void GetArrayValue(float *wave, int first, int last, float* alignedWave)
{
	for(int i = first; i <= last; i++)
	{
		alignedWave[i - first] = wave[i];
	}
}

float *array_prepadding(float *wave, int size, int padding)
{
	float *newwave = new float[size + padding];
	memset(newwave, 0, padding*sizeof(float));
	memcpy(newwave + padding, wave, size*sizeof(float));
	return newwave;
}

float *array_postpadding(float *wave, int size, int padding)
{
	float *newwave = new float[size + padding];
	memcpy(newwave, wave, size*sizeof(float));
	memset(newwave + size, 0, padding*sizeof(float));
	return newwave;
}

void array_plus(float *x1, float *x2, float *plus, int size)
{
	for(int i = 0; i < size; i++)
	{
		plus[i] = x1[i] + x2[i];
	}
}

int sign(float U)
{
	if (U > 0)
		return 1;
	if (U < 0)
		return -1;
	return 0;
}

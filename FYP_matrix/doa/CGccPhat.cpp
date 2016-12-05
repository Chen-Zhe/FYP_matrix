#include "stdafx.h"
#include "CGccPhat.h"
#include <memory.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define DEBUG 1

void fn_crossCorrComplexNumerator(const fftwf_complex *sig1, const fftwf_complex *sig2, fftwf_complex *xcorr_sig12, int N_2p1);

void fn_crossCorrComplexNumerator_withScale(const fftwf_complex *sig1, const fftwf_complex *sig2, fftwf_complex *xcorr_sig12, int N_2p1, const float *Scale2);

CGccPhat::CGccPhat()
{
	initFlag = 0;
	extract_gcc = NULL;
}


CGccPhat::~CGccPhat()
{
	fftwf_free(extract_gcc);
}

int CGccPhat::init_N(int v_N, int v_range)
{
	assert(initFlag == 0);
	N = v_N;
	N_2p1 = N/2+1;
	range = v_range;
	extract_gcc = (float *) fftwf_malloc(sizeof(float) * ((range*2)+1));   
	gcc = (float *) fftwf_malloc(sizeof(float) * N);
	sig_12_c2r.init_N_c2r(N);
	initFlag = 1;

   smooth_extract_gcc = (float*) malloc(sizeof(float) * ((range*2)+1));
   for (int i=0; i<2*range+1; i++)
      smooth_extract_gcc[i] = 0;

	return 0;
}


void CGccPhat::evaluate_FFT_gccPhat(const fftwf_complex  *sig1_r2c, const fftwf_complex  *sig2_r2c, int N)
{
#ifdef DEBUG_GCCPHAT
   fftw3_display_complex( "sig1 fft", sig1_r2c, N_2p1);
   fftw3_display_complex( "sig2 fft", sig2_r2c, N_2p1);
#endif 

	fn_crossCorrComplexNumerator(sig1_r2c, sig2_r2c, sig_12_c2r.get_pInComplex(),	N_2p1);


	sig_12_c2r.evaluate_plan();

#ifdef DEBUG_GCCPHAT
   fftw3_display_real("sig12_ifft", sig_12_c2r.get_pOutReal(), sig_12_c2r.get_N());
#endif 

	sig_12_c2r.fftshift();
	gcc = sig_12_c2r.get_pOutReal();
	float *p =sig_12_c2r.get_pOutReal();

#ifdef DEBUG_GCCPHAT
   fftw3_display_real("sig12_ifft_fftshifted", p, sig_12_c2r.get_N());
#endif

	memcpy(&extract_gcc[0], &p[N_2p1-1-range], sizeof(float)*range);
	memcpy(&extract_gcc[range], &p[N_2p1-1], sizeof(float)*(range+1));

#ifdef DEBUG_GCCPHAT
	fftw3_display_real("sig12_extracted", extract_gcc, range*2+1);
#endif
	
	return;
}

void CGccPhat::fn_extractGccStats(float *pMaxVal, int *pMaxIdx, float *pMaxMeanRatio)
{
   // extracting maxVal, maxIdx, ratioMaxMean

	gccMaxVal = -1e9;
	gccMaxIdx = -range-1;
	gccRatioMaxMean = 0;
	float tmpSum   = 0;

	int r2 = range*2+1;
	for (int i=0; i < r2; i++)
 	{   
		tmpSum = tmpSum+fabs(extract_gcc[i]);

		if (extract_gcc[i] > gccMaxVal)
		{
			gccMaxVal = extract_gcc[i];
			gccMaxIdx = i;
		}
	}
	
	gccMaxIdx = gccMaxIdx - range;
	gccRatioMaxMean = gccMaxVal/tmpSum;

	*pMaxVal  = gccMaxVal;
	*pMaxIdx  = gccMaxIdx;
	*pMaxMeanRatio = gccRatioMaxMean;
}

void CGccPhat::fn_extractGccStats(float **GCC_range, int delay_range, int pairs)
{
   // extracting maxVal, maxIdx, ratioMaxMean

	int r2 = delay_range*2+1;
	for (int i=0; i < r2; i++)
 	{   
		GCC_range[pairs][i] = extract_gcc[i];
	}		
}

void CGccPhat::fn_extractGccStats_Long(float *pMaxVal, int *pMaxIdx, float *pMaxMeanRatio)
{
   // extracting maxVal, maxIdx, ratioMaxMean

	gccMaxVal = -1e9;
	gccMaxIdx = -range-1;
	gccRatioMaxMean = 0;
	float tmpSum   = 0;

	int r2 = N;
	for (int i=0; i < r2; i++)
 	{   
		tmpSum = tmpSum+fabs(gcc[i]);

		if (gcc[i] > gccMaxVal)
		{
			gccMaxVal = gcc[i];
			gccMaxIdx = i;
		}
	}
	
	gccMaxIdx = gccMaxIdx - N/2;
	gccRatioMaxMean = gccMaxVal/tmpSum;

	*pMaxVal  = gccMaxVal;
	*pMaxIdx  = gccMaxIdx;
	*pMaxMeanRatio = gccRatioMaxMean;
}


void fn_crossCorrComplexNumerator(const fftwf_complex *sig1, const fftwf_complex *sig2, fftwf_complex *xcorr_sig12, int N_2p1)
{ 
   float r,c,scale;

   // (a+jb)conj(c+jd) = (a+jb)*(c-jd) = ac +bd  + jbc -jad
	//                                 = (ac+bd) + j(bc -ad)
	// a = sig1[i][0], b = sig1[i][1];
	// c = sig2[i][0], d = sig2[i][1];

	for (int i=0; i < N_2p1; i++)
	{
		r = sig1[i][0]*sig2[i][0] + sig1[i][1]*sig2[i][1];
		c = sig1[i][1]*sig2[i][0] - sig1[i][0]*sig2[i][1];

		if (i<96 || i>N_2p1-96)
		{
			xcorr_sig12[i][0] = 0;
			xcorr_sig12[i][1] = 0;
		}
		else if (i>1280 && i<N_2p1-1280)
		{
			xcorr_sig12[i][0] = 0;
			xcorr_sig12[i][1] = 0;
		}
		else
		{
			scale = (float)(1.0/sqrt(r*r + c*c));
			xcorr_sig12[i][0] = r*scale;
			xcorr_sig12[i][1] = c*scale;
		}
	} // end of for i
}


void fn_crossCorrComplexNumerator_withScale(const fftwf_complex *sig1, const fftwf_complex *sig2, fftwf_complex *xcorr_sig12, int N_2p1, const float *Scale2)
{ 
   float r,c,scale;

	for (int i=0; i < N_2p1; i++)
	{
		r = sig1[i][0]*sig2[i][0] + sig1[i][1]*sig2[i][1];
		c = sig1[i][1]*sig2[i][0] - sig1[i][0]*sig2[i][1];

		scale = (float)(1.0/sqrt(r*r + c*c))*Scale2[i];
		xcorr_sig12[i][0] = r*scale;
		xcorr_sig12[i][1] = c*scale;
	} // end of for i
}




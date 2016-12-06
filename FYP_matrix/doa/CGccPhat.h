// FileName: CGccPhat.h
// Author: Chng Eng Siong
// 15 Oct 2009


#ifndef INC_CGccPhat
#define INC_CGccPhat

#include "CFFTW.h"


class CGccPhat
{
private:
	CFFTW			   sig_12_c2r;
	int				N;
	int				N_2p1;
	int				initFlag;
	int				range;
	float*         extract_gcc;            // size = 2*range+1
   float*         smooth_extract_gcc;     // low pass filter of extract_gcc values
	float*         gcc;

	float          gccMaxVal;   // the op of the gcc cross correlation
	int            gccMaxIdx;
	float          gccRatioMaxMean;

private:


public:

	CGccPhat();
	~CGccPhat();
	int init_N(int v_N, int range);

	// The input to our GCC is the FFT of the signal!!!
	void evaluate_FFT_gccPhat(const fftwf_complex *sig1, const fftwf_complex *sig2, int N);
	void fn_extractGccStats(float *pMaxVal, int *pMaxIdx, float *pMaxMeanRatio);
	void fn_extractGccStats_Long(float *pMaxVal, int *pMaxIdx, float *pMaxMeanRatio);
	void fn_extractGccStats(float **GCC_range, int delay_range, int pairs);
};

#endif
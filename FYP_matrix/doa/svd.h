// FileName: modification of NR3's SVD
// Author : Chng ENg Siong
// Date: 20 Oct 2009
/*
	SVD mySVD43, mySVD32;
	mySVD43.solve(A,b,x);  
	solving x = pinv(A)*b;  
	// create an instantiation of mySVD for various sizes of A
	// to be efficient, since internally mySVD43 will allocate
	// internal variables for the particular size

*/
#ifndef INC_SVD
#define INC_SVD

#include "nr3.h"

struct SVD 
{
	Int m,n;
	MatDoub u,v;
	VecDoub w;
	Doub eps, tsh;
	SVD() { m = 0; n = 0;};
	void initSVD(int m_, int n_);
	void assignA(MatDoub_I &a);

	void solve(MatDoub_I &a, VecDoub_I &b, VecDoub_O &x);
	void solve(VecDoub_I &b, VecDoub_O &x, Doub thresh);
	void solve(MatDoub_I &b, MatDoub_O &x, Doub thresh);

	Int rank(Doub thresh);
	Int nullity(Doub thresh);
	MatDoub range(Doub thresh);
	MatDoub nullspace(Doub thresh);

	Doub inv_condition() {
		return (w[0] <= 0. || w[n-1] <= 0.) ? 0. : w[n-1]/w[0];
	}

	void decompose();
	void reorder();
	Doub pythag(const Doub a, const Doub b);
};

#endif
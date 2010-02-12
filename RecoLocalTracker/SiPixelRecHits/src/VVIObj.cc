//
//  VVIObj.cc  Version 1.2 
//
//  Port of CERNLIB G116 Functions vviden/vvidis
//
// Created by Morris Swartz on 1/14/2010.
// Copyright 2010 __TheJohnsHopkinsUniversity__. All rights reserved.
//
// V1.1 - make dzero call both fcns with a switch
// V1.2 - remove inappriate initializers and add methods to return non-zero/normalized region
//

#include <math.h>
#include <algorithm>

#ifndef SI_PIXEL_TEMPLATE_STANDALONE
// put CMSSW location of SimpleHelix.h here
#include "RecoLocalTracker/SiPixelRecHits/interface/VVIObj.h"
#else
#include "VVIObj.h"
#endif

// ***************************************************************************************************************************************
//! Constructor
//! Set Vavilov parameters kappa and beta2 amd define whether to calculate density fcn or distribution fcn
//! \param kappa - (input) Vavilov kappa parameter [0.01 (Landau-like) < kappa < 10. (Gaussian-like)]
//! \param beta2 - (input) Vavilov beta2 parameter (square of particle speed in v/c units)
//! \param mode  - (input) set to 0 to calculate the density function and to 1 to calculate the distribution function
// *************************************************************************************************************************************** 

VVIObj::VVIObj(double kappa, double beta2, double mode) : kappa_(kappa), beta2_(beta2), mode_(mode) {
	
	const double xp[9] = { 9.29,2.47,.89,.36,.15,.07,.03,.02,0.0 };
	const double xq[7] = { .012,.03,.08,.26,.87,3.83,11.0 };
	double  q, u, x, c1, c2, c3, c4, d1, h4, h5, h6, q2, x1, d, ll, ul, xf1, xf2, rv;
	int lp, lq, k, l, n;

// Make sure that the inputs are reasonable
	
	if(kappa_ < 0.01) kappa_ = 0.01;
	if(kappa_ > 10.) kappa_ = 10.;
	if(beta2_ < 0.) beta2_ = 0.;
	if(beta2_ > 1.) beta2_ = 1.;

	h_[4] = 1. - beta2_*0.42278433999999998 + 7.6/kappa_;
	h_[5] = beta2_;
	h_[6] = 1. - beta2_;
	h4 = -7.6/kappa_ - (beta2_ * .57721566 + 1);
	h5 = log(kappa_);
	h6 = 1./kappa_;
	t0_ = (h4 - h_[4]*h5 - (h_[4] + beta2_)*(log(h_[4]) + VVIObj::expint(h_[4])) + exp(-h_[4]))/h_[4];
	
// Set up limits for the root search
	
	for (lp = 0; lp < 9; ++lp) {
		if (kappa_ >= xp[lp]) break;
	}
	ll = -lp - 1.5;
	for (lq = 0; lq < 7; ++lq) {
		if (kappa_ <= xq[lq]) break;
	}
	ul = lq - 6.5;
//	double (*fp2)(double) = reinterpret_cast<double(*)(double)>(&VVIObj::f2);
	VVIObj::dzero(ll, ul, u, rv, 1.e-5, 1000, 2);
	q = 1./u;
	t1_ = h4 * q - h5 - (beta2_ * q + 1) * (log((fabs(u))) + VVIObj::expint(u)) + exp(-u) * q;
	t_ = t1_ - t0_;
	omega_ = 6.2831853000000004/t_;
	h_[0] = kappa_ * (beta2_ * .57721566 + 2.) + 9.9166128600000008;
	if (kappa_ >= .07) {h_[0] += 6.90775527;}
	h_[1] = beta2_ * kappa_;
	h_[2] = h6 * omega_;
	h_[3] = omega_ * 1.5707963250000001;
//	double (*fp1)(double) = reinterpret_cast<double(*)(double)>(&VVIObj::f1);
	VVIObj::dzero(5., 155., x0_, rv, 1.e-5, 1000, 1);
	n = x0_ + 1.;
	d = exp(kappa_ * (beta2_ * (.57721566 - h5) + 1.)) * .31830988654751274;
	a_[n - 1] = 0.;
	if (mode_ == 0) {
		a_[n - 1] = omega_ * .31830988654751274;
	}
	q = -1.;
	q2 = 2.;
	for (k = 1; k < n; ++k) {
		l = n - k;
		x = omega_ * k;
		x1 = h6 * x;
		c1 = log(x) - VVIObj::cosint(x1);
		c2 = VVIObj::sinint(x1);
		c3 = sin(x1);
		c4 = cos(x1);
		xf1 = kappa_ * (beta2_ * c1 - c4) - x * c2;
		xf2 = x * c1 + kappa_ * (c3 + beta2_ * c2) + t0_ * x;
		if (mode_ == 0) {
			d1 = q * d * omega_ * exp(xf1);
			a_[l - 1] = d1 * cos(xf2);
			b_[l - 1] = -d1 * sin(xf2);
		} else {
			d1 = q * d * exp(xf1)/k;
			a_[l - 1] = d1 * sin(xf2);
			b_[l - 1] = d1 * cos(xf2);
			a_[n - 1] += q2 * a_[l - 1];
		}
		q = -q;
		q2 = -q2;
	}
	 
} // VVIObj

// *************************************************************************************************************************************
//! Vavilov function method
//! Returns density fcn (mode=0) or distribution fcn (mode=1)
//! \param x  - (input) Argument of function [typically defined as (Q-mpv)/sigma]
// ************************************************************************************************************************************* 


double VVIObj::fcn(double x) {
	
	// Local variables
	
	double f, u, y, a0, a1;
	double a2 = 0.;
	double b1, b0, b2, cof;
	int k, n, n1;
	
	n = x0_;
	if (x < t0_) {
		f = 0.;
	} else if (x <= t1_) {
		y = x - t0_;
		u = omega_ * y - 3.141592653589793;
		cof = cos(u) * 2.;
		a1 = 0.;
		a0 = a_[0];
		n1=n+1;
		for (k = 2; k <= n1; ++k) {
			a2 = a1;
			a1 = a0;
			a0 = a_[k - 1] + cof * a1 - a2;
		}
		b1 = 0.;
		b0 = b_[0];
		for (k = 2; k <= n; ++k) {
			b2 = b1;
			b1 = b0;
			b0 = b_[k - 1] + cof * b1 - b2;
		}
		f = (a0 - a2) * .5 + b0 * sin(u);
		if (mode_ != 0) {f += y / t_;}
	} else {
		f = 0.;
		if (mode_ != 0) {f = 1.;}
	}
	return f;
} // fcn



// *************************************************************************************************************************************
//! Vavilov limits method
//! \param xl - (output) Smallest value of the argument for the density and the beginning of the normalized region for the distribution
//! \param xu - (output) Largest value of the argument for the density and the end of the normalized region for the distribution
// ************************************************************************************************************************************* 


void VVIObj::limits(double& xl, double& xu) {
	
   xl = t0_;
   xu = t1_;
	return;
} // limits


double VVIObj::f1(double x) { return h_[0]+h_[1]*log(h_[2]*x)-h_[3]*x;}

double VVIObj::f2(double x) { return h_[4]-x+h_[5]*(log(fabs(x))+VVIObj::expint(x))-h_[6]*exp(-x);}

double VVIObj::cosint(double x)
{
	// Initialized data
	
	const double zero = 0.;
	const double one = 1.;
	const double two = 2.;
	const double eight = 8.;
	const double ce = .57721566490153;
	const double c__[14] = { 1.9405491464836,.9413409132865,
		-.579845034293,.3091572011159,-.0916101792208,.0164437407515,
		-.0019713091952,1.692538851e-4,-1.09393296e-5,5.522386e-7,
	-2.23995e-8,7.465e-10,-2.08e-11,5e-13 };
	const double p[23] = { .96074783975204,-.0371138962124,
		.00194143988899,-1.7165988425e-4,2.112637753e-5,-3.27163257e-6,
		6.0069212e-7,-1.2586794e-7,2.932563e-8,-7.45696e-9,2.04105e-9,
		-5.9502e-10,1.8323e-10,-5.921e-11,1.997e-11,-7e-12,2.54e-12,
	-9.5e-13,3.7e-13,-1.4e-13,6e-14,-2e-14,1e-14 };
	const double q[20] = { .98604065696238,-.0134717382083,
		4.5329284117e-4,-3.067288652e-5,3.13199198e-6,-4.2110196e-7,
		6.907245e-8,-1.318321e-8,2.83697e-9,-6.7329e-10,1.734e-10,
		-4.787e-11,1.403e-11,-4.33e-12,1.4e-12,-4.7e-13,1.7e-13,-6e-14,
	2e-14,-1e-14 };
	
	// System generated locals
	double d__1;
	
	// Local variables
	double h__;
	int i__;
	double r__, y, b0, b1, b2, pp, qq, alfa;
	
	// If x==0, return same
	
	if (x == zero) {
	   return zero;
	}
	if (fabs(x) <= eight) {
	   y = x / eight;
		// Computing 2nd power
	   d__1 = y;
	   h__ = two * (d__1 * d__1) - one;
	   alfa = -two * h__;
	   b1 = zero;
	   b2 = zero;
	   for (i__ = 13; i__ >= 0; --i__) {
	      b0 = c__[i__] - alfa * b1 - b2;
	      b2 = b1;
	      b1 = b0;
	   }
	   b1 = ce + log((fabs(x))) - b0 + h__ * b2;
	} else {
	   r__ = one / x;
	   y = eight * r__;
		// Computing 2nd power
	   d__1 = y;
	   h__ = two * (d__1 * d__1) - one;
	   alfa = -two * h__;
	   b1 = zero;
	   b2 = zero;
	   for (i__ = 22; i__ >= 0; --i__) {
			b0 = p[i__] - alfa * b1 - b2;
			b2 = b1;
			b1 = b0;
	   }
	   pp = b0 - h__ * b2;
	   b1 = zero;
	   b2 = zero;
	   for (i__ = 19; i__ >= 0; --i__) {
			b0 = q[i__] - alfa * b1 - b2;
			b2 = b1;
			b1 = b0;
	   }
	   qq = b0 - h__ * b2;
	   b1 = r__ * (qq * sin(x) - r__ * pp * cos(x));
	}
	return b1;
} // cosint

double VVIObj::sinint(double x)
{
	// Initialized data
	
	const double zero = 0.;
	const double one = 1.;
	const double two = 2.;
	const double eight = 8.;
	const double pih = 1.5707963267949;
	const double s[14] = { 1.9522209759531,-.6884042321257,
		.4551855132256,-.1804571236838,.0410422133759,-.0059586169556,
		6.001427414e-4,-4.44708329e-5,2.5300782e-6,-1.141308e-7,4.1858e-9,
	-1.273e-10,3.3e-12,-1e-13 };
	const double p[23] = { .96074783975204,-.0371138962124,
		.00194143988899,-1.7165988425e-4,2.112637753e-5,-3.27163257e-6,
		6.0069212e-7,-1.2586794e-7,2.932563e-8,-7.45696e-9,2.04105e-9,
		-5.9502e-10,1.8323e-10,-5.921e-11,1.997e-11,-7e-12,2.54e-12,
	-9.5e-13,3.7e-13,-1.4e-13,6e-14,-2e-14,1e-14 };
	const double q[20] = { .98604065696238,-.0134717382083,
		4.5329284117e-4,-3.067288652e-5,3.13199198e-6,-4.2110196e-7,
		6.907245e-8,-1.318321e-8,2.83697e-9,-6.7329e-10,1.734e-10,
		-4.787e-11,1.403e-11,-4.33e-12,1.4e-12,-4.7e-13,1.7e-13,-6e-14,
	2e-14,-1e-14 };
	
	// System generated locals
	double d__1;
	
	// Local variables
	double h__;
	int i__;
	double r__, y, b0, b1, b2, pp, qq, alfa;
	
	if (fabs(x) <= eight) {
		y = x / eight;
		d__1 = y;
		h__ = two * (d__1 * d__1) - one;
		alfa = -two * h__;
		b1 = zero;
		b2 = zero;
		for (i__ = 13; i__ >= 0; --i__) {
			b0 = s[i__] - alfa * b1 - b2;
			b2 = b1;
			b1 = b0;
		}
		b1 = y * (b0 - b2);
	} else {
		r__ = one / x;
		y = eight * r__;
		d__1 = y;
		h__ = two * (d__1 * d__1) - one;
		alfa = -two * h__;
		b1 = zero;
		b2 = zero;
		for (i__ = 22; i__ >= 0; --i__) {
			b0 = p[i__] - alfa * b1 - b2;
			b2 = b1;
         b1 = b0;
		}
		pp = b0 - h__ * b2;
		b1 = zero;
		b2 = zero;
		for (i__ = 19; i__ >= 0; --i__) {
			b0 = q[i__] - alfa * b1 - b2;
			b2 = b1;
			b1 = b0;
		}
		qq = b0 - h__ * b2;
		d__1 = fabs(pih);
		if(x < 0.) d__1 = -d__1;
		b1 = d__1 - r__ * (r__ * pp * sin(x) + qq * cos(x));
	}
	
	return b1;
} // sinint


int VVIObj::dzero(double a, double b, double& x0, 
									double& rv, double eps, int mxf, int fsel)
{
	/* System generated locals */
	double d__1, d__2, d__3, d__4;
	
	// Local variables
	double f1, f2, f3, u1, u2, x1, x2, u3, u4, x3, ca, cb, cc, fa, fb, ee, ff;
	int mc;
	double xa, xb, fx, xx, su4;
	
	xa = std::min(a,b);
	xb = std::max(a,b);
	if(fsel == 1) {fa = VVIObj::f1(xa);} else {fa = VVIObj::f2(xa);}
	if(fsel == 1) {fb = VVIObj::f1(xb);} else {fb = VVIObj::f2(xb);}
	if (fa * fb > 0.) {
	   rv = (xb - xa) * -2;
	   x0 = 0.;
	   return 1;
	}
	mc = 0;
L1:
	x0 = (xa + xb) * .5;
	rv = x0 - xa;
	ee = eps * (fabs(x0) + 1);
	if (rv <= ee) {
	   rv = ee;
		if(fsel == 1) {ff = VVIObj::f1(x0);} else {ff = VVIObj::f2(x0);}
	   return 0;
	}
	f1 = fa;
	x1 = xa;
	f2 = fb;
	x2 = xb;
L2:
	if(fsel == 1) {fx = VVIObj::f1(x0);} else {fx = VVIObj::f2(x0);}
	++mc;
	if (mc > mxf) {
	   rv = (d__1 = xb - xa, fabs(d__1)) * -.5;
	   x0 = 0.;
	   return 0;
	}
	if (fx * fa > 0.) {
	   xa = x0;
	   fa = fx;
	} else {
	   xb = x0;
	   fb = fx;
	}
L3:
	u1 = f1 - f2;
	u2 = x1 - x2;
	u3 = f2 - fx;
	u4 = x2 - x0;
	if (u2 == 0. || u4 == 0.) {goto L1;}
	f3 = fx;
	x3 = x0;
	u1 /= u2;
	u2 = u3 / u4;
	ca = u1 - u2;
	cb = (x1 + x2) * u2 - (x2 + x0) * u1;
	cc = (x1 - x0) * f1 - x1 * (ca * x1 + cb);
	if (ca == 0.) {
	   if (cb == 0.) {goto L1;}
	   x0 = -cc / cb;
	} else {
	   u3 = cb / (ca * 2);
	   u4 = u3 * u3 - cc / ca;
	   if (u4 < 0.) {goto L1;}
	   su4 = fabs(u4);
	   if (x0 + u3 < 0.f) {su4 = -su4;}
	   x0 = -u3 + su4;
	}
	if (x0 < xa || x0 > xb) {goto L1;}
	// Computing MIN
	d__3 = (d__1 = x0 - x3, fabs(d__1)), d__4 = (d__2 = x0 - x2, fabs(d__2));
	rv = std::min(d__3,d__4);
	ee = eps * (fabs(x0) + 1);
	if (rv > ee) {
	   f1 = f2;
	   x1 = x2;
	   f2 = f3;
	   x2 = x3;
	   goto L2;
	}
	if(fsel == 1) {fx = VVIObj::f1(x0);} else {fx = VVIObj::f2(x0);}
	if (fx == 0.) {
	   rv = ee;
	   if(fsel == 1) {ff = VVIObj::f1(x0);} else {ff = VVIObj::f2(x0);}
	   return 0;
	}
	if (fx * fa < 0.) {
	   xx = x0 - ee;
	   if (xx <= xa) {
			rv = ee;
			if(fsel == 1) {ff = VVIObj::f1(x0);} else {ff = VVIObj::f2(x0);}
			return 0;
	   }
	   if(fsel == 1) {ff = VVIObj::f1(xx);} else {ff = VVIObj::f2(xx);}
	   fb = ff;
	   xb = xx;
	} else {
	   xx = x0 + ee;
	   if (xx >= xb) {
	      rv = ee;
         if(fsel == 1) {ff = VVIObj::f1(x0);} else {ff = VVIObj::f2(x0);}
	      return 0;
	   }
	   if(fsel == 1) {ff = VVIObj::f1(xx);} else {ff = VVIObj::f2(xx);}
	   fa = ff;
	   xa = xx;
	}
	if (fx * ff > 0.) {
	   mc += 2;
		if (mc > mxf) {
	      rv = (d__1 = xb - xa, fabs(d__1)) * -.5;
	      x0 = 0.;
	      return 0;
	   }
	   f1 = f3;
	   x1 = x3;
	   f2 = fx;
	   x2 = x0;
	   x0 = xx;
	   fx = ff;
	   goto L3;
	}
	/* L4: */
	rv = ee;
	if(fsel == 1) {ff = VVIObj::f1(x0);} else {ff = VVIObj::f2(x0);}
	return 0;
} // dzero


double VVIObj::expint(double x)

{
	
	// Initialized data
	
	const double zero = 0.;
	const double q2[7] = { .10340013040487,3.319092135933,
		20.449478501379,41.280784189142,32.426421069514,10.041164382905,
		1. };
	const double p3[6] = { -2.3909964453136,-147.98219500504,
		-254.3763397689,-119.55761038372,-19.630408535939,-.9999999999036 
	};
	const double q3[6] = { 177.60070940351,530.68509610812,
		462.23027156148,156.81843364539,21.630408494238,1. };
	const double p4[8] = { -8.6693733995107,-549.14226552109,
		-4210.0161535707,-249301.39345865,-119623.66934925,
		-22174462.775885,3892804.213112,-391546073.8091 };
	const double q4[8] = { 34.171875,-1607.0892658722,35730.029805851,
		-483547.43616216,4285596.2461175,-24903337.574054,89192576.757561,
		-165254299.72521 };
	const double a1[8] = { -2.1808638152072,-21.901023385488,
		9.3081638566217,25.076281129356,-33.184253199722,60.121799083008,
		-43.253113287813,1.0044310922808 };
	const double b1[8] = { 0.,3.9370770185272,300.89264837292,
		-6.2504116167188,1003.6743951673,14.325673812194,2736.2411988933,
		.52746885196291 };
	const double a2[8] = { -3.4833465360285,-18.65454548834,
		-8.2856199414064,-32.34673303054,17.960168876925,1.7565631546961,
		-1.9502232128966,.99999429607471 };
	const double b2[8] = { 0.,69.500065588743,57.283719383732,
		25.777638423844,760.76114800773,28.951672792514,-3.4394226689987,
		1.0008386740264 };
	const double a3[6] = { -27.780928934438,-10.10479081576,
		-9.1483008216736,-5.0223317461851,-3.0000077799358,
		1.0000000000704 };
	const double one = 1.;
	const double b3[6] = { 0.,122.39993926823,2.7276100778779,
		-7.1897518395045,-2.9990118065262,1.999999942826 };
	const double two = 2.;
	const double three = 3.;
	const double x0 = .37250741078137;
	const double xl[6] = { -24.,-12.,-6.,0.,1.,4. };
	const double p1[5] = { 4.293125234321,39.894153870321,
		292.52518866921,425.69682638592,-434.98143832952 };
	const double q1[5] = { 1.,18.899288395003,150.95038744251,
		568.05252718987,753.58564359843 };
	const double p2[7] = { .43096783946939,6.9052252278444,
		23.019255939133,24.378408879132,9.0416155694633,.99997957705159,
		4.656271079751e-7 };
	
	/* Local variables */
	static int i__;
	double v, y, ap, bp, aq, dp, bq, dq;
	
	if (x <= xl[0]) {
		ap = a3[0] - x;
		for (i__ = 2; i__ <= 5; ++i__) {
			/* L1: */
			ap = a3[i__ - 1] - x + b3[i__ - 1] / ap;
		}
		y = exp(-x) / x * (one - (a3[5] + b3[5] / ap) / x);
	} else if (x <= xl[1]) {
		ap = a2[0] - x;
		for (i__ = 2; i__ <= 7; ++i__) {
			ap = a2[i__ - 1] - x + b2[i__ - 1] / ap;
		}
		y = exp(-x) / x * (a2[7] + b2[7] / ap);
	} else if (x <= xl[2]) {
		ap = a1[0] - x;
		for (i__ = 2; i__ <= 7; ++i__) {
			ap = a1[i__ - 1] - x + b1[i__ - 1] / ap;
		}
		y = exp(-x) / x * (a1[7] + b1[7] / ap);
	} else if (x < xl[3]) {
		v = -two * (x / three + one);
		bp = zero;
		dp = p4[0];
		for (i__ = 2; i__ <= 8; ++i__) {
			ap = bp;
			bp = dp;
			dp = p4[i__ - 1] - ap + v * bp;
		}
		bq = zero;
		dq = q4[0];
		for (i__ = 2; i__ <= 8; ++i__) {
			aq = bq;
			bq = dq;
			dq = q4[i__ - 1] - aq + v * bq;
		}
		y = -log(-x / x0) + (x + x0) * (dp - ap) / (dq - aq);
	} else if (x == xl[3]) {
		return zero;
	} else if (x < xl[4]) {
		ap = p1[0];
		aq = q1[0];
		for (i__ = 2; i__ <= 5; ++i__) {
			ap = p1[i__ - 1] + x * ap;
			aq = q1[i__ - 1] + x * aq;
		}
		y = -log(x) + ap / aq;
	} else if (x <= xl[5]) {
		y = one / x;
		ap = p2[0];
		aq = q2[0];
		for (i__ = 2; i__ <= 7; ++i__) {
			ap = p2[i__ - 1] + y * ap;
			aq = q2[i__ - 1] + y * aq;
		}
		y = exp(-x) * ap / aq;
	} else {
		y = one / x;
		ap = p3[0];
		aq = q3[0];
		for (i__ = 2; i__ <= 6; ++i__) {
			ap = p3[i__ - 1] + y * ap;
			aq = q3[i__ - 1] + y * aq;
		}
		y = exp(-x) * y * (one + y * ap / aq);
	}
	return y;
} // expint




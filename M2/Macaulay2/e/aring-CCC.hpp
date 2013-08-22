// Copyright 2012 Michael E. Stillman

#ifndef _aring_CCC_hpp_
#define _aring_CCC_hpp_

#include <iostream>
#include "aring.hpp"
#include "buffer.hpp"
#include "ringelem.hpp"
#include "ringmap.hpp"

#include "aring-RRR.hpp"

class RingMap;

namespace M2 {
/**
\ingroup rings
*/
  class ARingCCC : public RingInterface
  {
    // complex numbers represented as pairs of MPFRs.

  public:
    static const RingID ringID = ring_CCC;

    struct mpfc_struct {__mpfr_struct re; __mpfr_struct im;};
    typedef mpfc_struct* mpfc_ptr;
    typedef mpfc_struct elem; // ???
    typedef elem ElementType;

    ARingCCC(unsigned long precision) : mPrecision(precision) {}
    ARingCCC(){} // ???

    // ring informational
    size_t characteristic() const { return 0; }
    unsigned long get_precision() const { return mPrecision; }
    void text_out(buffer &o) const;

    unsigned long computeHashValue(const ElementType& a) const  
    { 
      double d = mpfr_get_d(&a.re, GMP_RNDN); // redo???
      return static_cast<unsigned long>(d);
    }

    /////////////////////////////////
    // ElementType informational ////
    /////////////////////////////////

    bool is_unit(const ElementType& f) const { 
      return !is_zero(f); 
    }

    bool is_zero(const ElementType& f) const {
      return mpfr_cmp_si(&f.re, 0) == 0 && mpfr_cmp_si(&f.im, 0) == 0;
    }
    
    bool is_equal(const ElementType& f, const ElementType& g) const { 
      return mpfr_cmp(&f.re,&g.re) == 0 && mpfr_cmp(&f.im,&g.im) == 0;
    }

    int compare_elems(const ElementType& f, const ElementType& g) const {
      int cmp_re = mpfr_cmp(&f.re,&g.re);
      if (cmp_re < 0) return -1;
      if (cmp_re > 0) return 1;
      int cmp_im = mpfr_cmp(&f.im,&g.im);
      if (cmp_im < 0) return -1;
      if (cmp_im > 0) return 1;      
      return 0;
    }

    ////////////////////////////
    // to/from ringelem ////////
    ////////////////////////////
    // These simply repackage the element as either a ringelem or an 'ElementType'.
    // No reinitialization is done.
    // Do not take the same element and store it as two different ring_elem's!!
    void to_ring_elem(ring_elem &result, const ElementType &a) const
    {
      mpfc_ptr res = getmemstructtype(mpfc_ptr);
      init(*res);
      set(*res, a);
      result = MPF_RINGELEM(res);
    }

    void from_ring_elem(ElementType &result, const ring_elem &a) const
    {
      set(result, *reinterpret_cast<mpfc_ptr>(a.poly_val));
    }

    // 'init', 'init_set' functions

    void init(ElementType &result) const { 
      mpfr_init2(&result.re, mPrecision); 
      mpfr_init2(&result.im, mPrecision); 
    }

    void init_set(ElementType &result, const ElementType& a) const { 
      init(result);
      mpfr_set(&result.re, &a.re, GMP_RNDN);
      mpfr_set(&result.im, &a.im, GMP_RNDN);
    }

    void set(ElementType &result, const ElementType& a) const { 
      mpfr_set(&result.re, &a.re, GMP_RNDN);
      mpfr_set(&result.im, &a.im, GMP_RNDN);
    }

    void set_zero(ElementType &result) const { 
      mpfr_set_si(&result.re, 0, GMP_RNDN); 
      mpfr_set_si(&result.im, 0, GMP_RNDN); 
    }

    void clear(ElementType &result) const { 
      mpfr_clear(&result.re); 
      mpfr_clear(&result.im); 
    }

    void copy(ElementType &result, const ElementType& a) const { 
      set(result, a); 
    }

    void set_from_int(ElementType &result, long a) const {
      mpfr_set_si(&result.re, a, GMP_RNDN);
      mpfr_set_si(&result.im, 0, GMP_RNDN);    
    }

    void set_var(ElementType &result, int v) const { 
      set_from_int(result, 1); 
    }

    void set_from_mpz(ElementType &result, mpz_ptr a) const {
      mpfr_set_z(&result.re, a, GMP_RNDN);
      mpfr_set_si(&result.im, 0, GMP_RNDN);
    }

    void set_from_mpq(ElementType &result, mpq_ptr a) const {
      mpfr_set_q(&result.re, a, GMP_RNDN);
      mpfr_set_si(&result.im, 0, GMP_RNDN);
    }

    bool set_from_BigReal(ElementType &result, gmp_RR a) const {
      mpfr_set(&result.re, a, GMP_RNDN);
      mpfr_set_si(&result.im, 0, GMP_RNDN);
      return true;
    }

    bool set_from_BigComplex(ElementType &result, gmp_CC a) const { //???
      mpfr_set(&result.re, a->re, GMP_RNDN);
      mpfr_set(&result.im, a->im, GMP_RNDN);
      return true;
    }

    // arithmetic
    void negate(ElementType &result, const ElementType& a) const
    {
      mpfr_neg(&result.re, &a.re, GMP_RNDN);
      mpfr_neg(&result.im, &a.im, GMP_RNDN);
    }

    void invert(ElementType &result, const ElementType& a) const
      // we silently assume that a != 0.  If it is, result is set to a^0, i.e. 1
    {
      mpfr_t p, denom;
      mpfr_init2(p, mPrecision);
      mpfr_init2(denom, mPrecision);

      if (mpfr_cmpabs(&a.re,&a.im) >= 0)
        {
          // double p = &a.im/&a.re;
          // double denom = &a.re + p * &a.im;
          // &result.re = 1.0/denom;
          // &result.im = - p/denom;

          mpfr_div(p,&a.im,&a.re,GMP_RNDN);
          mpfr_mul(denom,p,&a.im,GMP_RNDN);
          mpfr_add(denom,denom,&a.re,GMP_RNDN);
          mpfr_si_div(&result.re,1,denom,GMP_RNDN);
          mpfr_div(&result.im,p,denom,GMP_RNDN);
          mpfr_neg(&result.im,&result.im,GMP_RNDN);
        }
      else
        {
          // double p = &a.re/&a.im;
          // double denom = &a.im + p * &a.re;
          // &result.re = p/denom;
          // &result.im = -1.0/denom;

          mpfr_div(p,&a.re,&a.im,GMP_RNDN);
          mpfr_mul(denom,p,&a.re,GMP_RNDN);
          mpfr_add(denom,denom,&a.im,GMP_RNDN);
          mpfr_si_div(&result.im,1,denom,GMP_RNDN);
          mpfr_neg(&result.im,&result.im,GMP_RNDN);
          mpfr_div(&result.re,p,denom,GMP_RNDN);
        }
      
      mpfr_clear(p);
      mpfr_clear(denom);
    }

    void add(ElementType &result, const ElementType& a, const ElementType& b) const
    {
      mpfr_add(&result.re, &a.re, &b.re, GMP_RNDN);
      mpfr_add(&result.im, &a.im, &b.im, GMP_RNDN);
    }

    void subtract(ElementType &result, const ElementType& a, const ElementType& b) const
    {
      mpfr_sub(&result.re, &a.re, &b.re, GMP_RNDN);
      mpfr_sub(&result.im, &a.im, &b.im, GMP_RNDN);
    }

    void subtract_multiple(ElementType &result, const ElementType& a, const ElementType& b) const
    {
      //TODO: write this
      // we assume: a, b are NONZERO!!
      // result -= a*b
    }

    void mult(ElementType &res, const ElementType& a, const ElementType& b) const
    {
      // std::cout << "in ARingCCC::mult" << std::endl;
      mpfr_t tmp;
      ElementType result;
      init(result);
      mpfr_init2(tmp, mPrecision);

      // &result.re = &a.re*&b.re - &a.im*&b.im;
      mpfr_mul(tmp,&a.re,&b.re,GMP_RNDN);
      mpfr_set(&result.re,tmp,GMP_RNDN);
      mpfr_mul(tmp,&a.im,&b.im,GMP_RNDN);
      mpfr_sub(&result.re,&result.re,tmp,GMP_RNDN);

      // &result.im = &a.re*&b.im + &a.im*&b.re;
      mpfr_mul(tmp,&a.re,&b.im,GMP_RNDN);
      mpfr_set(&result.im,tmp,GMP_RNDN);
      mpfr_mul(tmp,&a.im,&b.re,GMP_RNDN);
      mpfr_add(&result.im,&result.im,tmp,GMP_RNDN);

      set(res,result);
      clear(result);
      mpfr_clear(tmp);
    }

    void divide(ElementType &res, const ElementType& a, const ElementType& b) const
    {
      mpfr_t p, denom;
      mpfr_init2(p, mPrecision);
      mpfr_init2(denom, mPrecision);
      ElementType result;
      init(result);

      if (mpfr_cmpabs(&b.re, &b.im) >= 0)
        {
          // for v = c + d*i,
          // p = d/c
          // c+di = c(1+p*i), so denom is c(1+p^2)
          // which is c + d*p

          // double p = v.im/v.re;
          // double denom = v.re + p * v.im;
          // result.re = (u.re + p*u.im)/denom;
          // result.im = (u.im - p*u.re)/denom;

          mpfr_div(p, &b.im, &b.re, GMP_RNDN);
          mpfr_mul(denom,p,&b.im,GMP_RNDN);
          mpfr_add(denom,denom,&b.re,GMP_RNDN);

          mpfr_mul(&result.re,p,&a.im,GMP_RNDN);
          mpfr_add(&result.re,&result.re,&a.re,GMP_RNDN);
          mpfr_div(&result.re,&result.re,denom,GMP_RNDN);

          mpfr_mul(&result.im,p,&a.re,GMP_RNDN);
          mpfr_neg(&result.im,&result.re,GMP_RNDN);
          mpfr_add(&result.im,&result.re,&a.im,GMP_RNDN);
          mpfr_div(&result.im,&result.re,denom,GMP_RNDN);
        }
      else
        {
          // double p = v.re/v.im;
          // double denom = v.im + p * v.re;
          // result.re = (u.re * p + u.im)/denom;
          // result.im = (-u.re + p * u.im)/denom;

          mpfr_div(p, &b.re, &b.im, GMP_RNDN);
          mpfr_mul(denom,p,&b.re,GMP_RNDN);
          mpfr_add(denom,denom,&b.im,GMP_RNDN);

          mpfr_mul(&result.re,p,&a.re,GMP_RNDN);
          mpfr_add(&result.re,&result.re,&a.im,GMP_RNDN);
          mpfr_div(&result.re,&result.re,denom,GMP_RNDN);

          mpfr_mul(&result.im,p,&a.im,GMP_RNDN);
          mpfr_sub(&result.im,&result.re,&a.re,GMP_RNDN);
          mpfr_div(&result.im,&result.re,denom,GMP_RNDN);
        }
      mpfr_clear(p);
      mpfr_clear(denom);
      set(res,result);
      clear(result);
    }

    void power(ElementType &result, const ElementType& a, int n) const
    {
      ElementType curr_pow;
      init(curr_pow);
      set_from_int(result,1);
      if (n == 0) {}
      else if (n < 0)
        {
          n = -n;
          invert(curr_pow, a);
        }
      else
        {
          set(curr_pow,a);
        }
      while (n > 0)
        {
          if (n%2) {
            mult(result, result, curr_pow);
          }
          n = n/2;
          mult(curr_pow, curr_pow, curr_pow);
        }
      clear(curr_pow);
    }

    void power_mpz(ElementType &result, const ElementType& a, mpz_ptr n) const
    {
      if (mpz_fits_slong_p(n)) 
        {
          int n1 = static_cast<int>(mpz_get_si(n));
          power(result,a,n1);
        } 
      else  
        {
          ERROR("exponent too large");
          set_from_int(result, 1);
        }
    }

    void swap(ElementType &a, ElementType &b) const
    {
      mpfr_swap(&a.re, &b.re);
      mpfr_swap(&a.im, &b.im);
    }

    void elem_text_out(buffer &o,
                       ElementType &a,
                       bool p_one,
                       bool p_plus,
                       bool p_parens) const;

    void syzygy(const ElementType& a, const ElementType& b,
                ElementType &x, ElementType &y) const // remove?
    // returns x,y s.y. x*a + y*b == 0.
    // if possible, x is set to 1.
    // no need to consider the case a==0 or b==0.
    {
      //TODO
    }

    void random(ElementType &result) const // redo?
    {
      rawRandomMpfr(&result.re, mPrecision);
      rawRandomMpfr(&result.im, mPrecision);
    }

    void eval(const RingMap *map, ElementType &f, int first_var, ring_elem &result) const
    {
      gmp_CC_struct g;
      g.re = &f.re;
      g.im = &f.im;
      map->get_ring()->from_BigComplex(&g, result);
    }

    // TODO: promote, lift.

    gmp_CC toBigComplex(const ElementType& a) const
    {
      gmp_CC result = getmemstructtype(gmp_CC);
      result->re = getmemstructtype(gmp_RR);
      result->im = getmemstructtype(gmp_RR);
      mpfr_init2(result->re,mPrecision);
      mpfr_init2(result->im,mPrecision);
      mpfr_set(result->re, &a.re, GMP_RNDN);
      mpfr_set(result->im, &a.im, GMP_RNDN);
      return result;
    }

    bool set_from_RRR(ElementType &result, const ARingRRR::ElementType& a) const {
      mpfr_set(&result.re, &a, GMP_RNDN);
      mpfr_set_si(&result.im, 0, GMP_RNDN);
      return true;
    }

  private:
      unsigned long mPrecision;
  };

}; // end namespace M2

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e  "
// indent-tabs-mode: nil
// End:

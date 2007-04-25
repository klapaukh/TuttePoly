#ifndef FACTOR_POLY_HPP
#define FACTOR_POLY_HPP

#include <iostream>
#include <string>
#include <utility>
#include "xy_term.hpp"

#define FPOLY_PADDING_FACTOR 1

template<class T>
class yterms {
public:  
  unsigned int ymin;
  unsigned int ymax;
private:  
  unsigned int fpadding;
  unsigned int bpadding;
  T *coefficients;
public:
  /* =============================== */
  /* ========= CONSTRUCTORS ======== */
  /* =============================== */

  yterms() : coefficients(NULL) { };

  yterms(unsigned int y_min, unsigned int y_max) {
    alloc(y_min,y_max);
  }

  yterms(yterms<T> const &src)  { clone(src); }

  ~yterms() { delete [] coefficients; }

  /* =============================== */
  /* ======== ASSIGNMENT OP ======== */
  /* =============================== */

  yterms const &operator=(yterms<T> const &src) {
    if(&src != this) {
      delete [] coefficients;
      clone(src);
    }
    return *this;
  }

  void swap(yterms<T> &src) {
    if(&src != this) {
      std::swap(ymin,src.ymin);
      std::swap(ymax,src.ymax);
      std::swap(fpadding,src.fpadding);
      std::swap(bpadding,src.bpadding);
      std::swap(coefficients,src.coefficients);
    }
  }

  /* =============================== */
  /* ======== ARITHMETIC OPS ======= */
  /* =============================== */

  void operator+=(xy_term const &p) {
    // make sure enough y terms
    resize(p.ypower,p.ypowerend);
    // now, do the addition
    for(unsigned int i=p.ypower;i<=p.ypowerend;++i) { 
      coefficients[i - ymin + fpadding] += 1;
    }    
  }

  void operator+=(yterms<T> const &src) {
    // make sure enough y terms
    resize(src.ptr->ymin,src.ptr->ymax);
    // now, do the addition
    unsigned int start = src.ptr->ymin;
    unsigned int end = src.ptr->ymax;
    for(unsigned int i=start;i<=end;++i) { 
      T c(src.coefficients[i - src.ymin + src.fpadding]);
      coefficients[i - ymin + fpadding] += c;
    }
  }

  void operator*=(xy_term const &p) {
    // if this poly is empty do nothing!
    if(is_empty()) { return;}
    // Ok, it's not empty ...
    unsigned int ystart = ymin;
    unsigned int yend = ymax;
    unsigned int ypadding = fpadding;
    unsigned int nystart = ymin + p.ypower;
    unsigned int nyend = ymax + p.ypowerend;
    
    if(p.ypower == p.ypowerend) {
      // easy case, only a shift required
      ymin = nystart;
      ymax = nyend;
    } else {
      // harder case
      
      // The following could use padding, since it currently doesn't!
      T *o_coefficients = coefficients;
      alloc(nystart,nyend);     
      unsigned int depth = (p.ypowerend-p.ypower)+1;
      unsigned int width = (yend-ystart)+1;
      
      // going up the triangle
      T acc = 0U;    
      for(unsigned int i=0;i<std::min(width,depth);++i) {
	acc += o_coefficients[i+ypadding];
	coefficients[i+nystart+fpadding-ymin] = acc;
      }
      // free fall (if there is any)
      for(unsigned int i=width;i<depth;++i) {
	coefficients[i+nystart+fpadding-ymin] = acc;
      }    
      // going along the top (if there is one)
      T sub = 0U;
      for(unsigned int i=depth;i<width;++i) {
	sub += o_coefficients[i+ypadding-depth];
	acc += o_coefficients[i+ypadding];
	coefficients[i+fpadding] = acc - sub;
      }    

      // going down the triangle
      for(unsigned int i=std::max(depth,width);i < (nyend-nystart)+1;++i) {
	sub += o_coefficients[i+ypadding - depth];
	coefficients[i+fpadding] = acc - sub;
      }
      delete [] o_coefficients;
    }
    // and we're done!
  }

  /* ========================== */
  /* ======== OTHER FNS ======= */
  /* ========================== */  

  unsigned int size() const {
    if(coefficients == NULL) { return 0; }
    return (ymax - ymin) + 1;
  }

  bool is_empty() const { return coefficients == NULL; }

  T operator[](int i) const { return coefficients[i + fpadding - ymin]; }

  double substitute(double y) const {
    return 0.0;
  }

  std::string str() const {
    std::stringstream ss;
    if(ymin != ymax) {
      ss << "y^{" << ymin << ".." << ymax << "}";
    } else if(ymin == 1) {
      ss << "y";
    } else if(ymin != 0) {
      ss << "y^" << ymin;
    } else if(ymin == 0) {
      return "";
    }
    ss << "(";
    for(unsigned int i=ymin;i<=ymax;++i) {
      if(i != ymin) { ss << " + "; }
      ss << coefficients[i + fpadding - ymin];
    }
    ss << ")";
    return ss.str();
  }

  void insert(unsigned int n, xy_term const &p) {   // needs a better name? 
    // make sure enough y terms
    resize(p.ypower,p.ypowerend);
    // now, do the addition
    for(unsigned int i=p.ypower;i<=p.ypowerend;++i) { 
      coefficients[i + fpadding - ymin] += n;
    }    
  }
  
private:  
  /* =============================== */
  /* ======== HELPERS ======= */
  /* =============================== */

  void resize(unsigned int y_min, unsigned int y_max)  {
    if(is_empty()) {
      alloc(y_min,y_max);
    } else {
      int d_end = y_max - ymax;
      int d_beg = ymin - y_min;
      
      if(d_beg <= (int) fpadding && d_end <= (int) bpadding) {
	// in this case, there is enough padding to cover it
	if(d_end > 0) {
	  ymax = y_max;      
	  bpadding -= d_end;
	}
	if(d_beg > 0) {
	  ymin = y_min;      
	  fpadding -= d_beg;
	}
      } else {
	// no, there definitely aren't enough y-terms
	unsigned int ystart = ymin;
	unsigned int yend = ymax;
	unsigned int ypadding = fpadding;
	T *o_coefficients = coefficients;	
	alloc(std::min(ystart,y_min),std::max(yend,y_max));
	// copy old stuff over
	for(unsigned int i=ystart;i<=yend;++i) { 
	  coefficients[i+fpadding-ymin] = o_coefficients[i+ypadding-ystart];
	}
	// free space!
	delete [] o_coefficients;
      }
    }
  }  

  void clone(yterms<T> const &src) { 
    if(src.is_empty()) { coefficients = NULL; }
    else {
      ymin = src.ymin;
      ymax = src.ymax;
      fpadding = src.fpadding;
      bpadding = src.bpadding;

      unsigned int ncoeffs = src.size() + src.fpadding + src.bpadding;
      
      coefficients = new T[ncoeffs];

      // copy old stuff over
      for(unsigned int i=ymin;i<=ymax;++i) { 
	coefficients[i+fpadding-ymin] = src.coefficients[i+fpadding-ymin];
      }
    }
  }

  void alloc(unsigned int _ymin, unsigned int _ymax) {
    unsigned int nyterms = (_ymax-_ymin)+1;
    bpadding = nyterms * FPOLY_PADDING_FACTOR;
    fpadding = std::min(_ymin,nyterms * FPOLY_PADDING_FACTOR);
    ymin = _ymin;
    ymax = _ymax;
    coefficients = new T[nyterms + bpadding + fpadding];
  }
};


// This iterator is a temporary structure
template<class T>
class factor_poly_iterator {
private:
  unsigned int x;
  unsigned int x_max;
  unsigned int y;
  yterms<T> *xterms;
public:
  factor_poly_iterator(unsigned int _x, unsigned int xm, yterms<T> *_xterms) 
    : x(_x), x_max(xm), xterms(_xterms) {
    while(x < x_max && xterms[x].is_empty()) { ++x; }      
    if(x >= x_max) { y = 0; }
    else { y = xterms[x].ymin; }
  }

  void operator++() { 
    if(++y > xterms[x].ymax) {
      do { ++x; }
      while(x < x_max && xterms[x].is_empty());
      if(x >= x_max) { y = 0; }
      else { y = xterms[x].ymin; }
    } 
  }

  factor_poly_iterator operator++(int) { 
    factor_poly_iterator tmp(*this);
    ++(*this);
    return tmp;
  }

  // crikey, this is ugly
  std::pair<xy_term, T> operator*() const {
    return std::make_pair(xy_term(x,y),xterms[x][y]);
  }

  std::string str() const {
    std::stringstream ss;
    ss << x << "," << y;
    return ss.str();
  }

  bool operator==(factor_poly_iterator const &o) const {
    return xterms == o.xterms && x == o.x && y == o.y;
  }

  bool operator!=(factor_poly_iterator const &o) const {
    return xterms != o.xterms || x != o.x || y != o.y;
  }
};

template<class T>
class factor_poly {
public:
  typedef factor_poly_iterator<T> const_iterator;
private:
  yterms<T> *xterms; 
  unsigned int nxterms;
public:
  /* =============================== */
  /* ========= CONSTRUCTORS ======== */
  /* =============================== */

  factor_poly() {
    nxterms = 5; // arbitrary ?
    // create the xterm array
    xterms = new yterms<T>[nxterms];  
  }

  factor_poly(xy_term const &xyt) {
    nxterms = xyt.xpower+1;
    // create the xterm array
    xterms = new yterms<T>[nxterms];  
    *this += xyt;
  }

  factor_poly(factor_poly<T> const &fp) { clone(fp); }

  ~factor_poly() { destroy(); }

  /* =============================== */
  /* ======== ASSIGNMENT OP ======== */
  /* =============================== */

  factor_poly const &operator=(factor_poly const &src) {
    if(&src != this) {
      destroy();
      clone(src);
    }
    return *this;  
  }

  /* =============================== */
  /* ======== ARITHMETIC OPS ======= */
  /* =============================== */

  void operator+=(factor_poly const &p) {
    // make sure enough x terms
    resize_xterms(p.nxterms); 
    for(unsigned int i=0;i<p.nxterms;++i) {
      if(!p.xterms[i].is_empty()) {
	xterms[i] += p.xterms[i];
      }
    }
  }

  void operator+=(xy_term const &p) {
    // make sure enough x terms
    resize_xterms(p.xpower+1); 
    // now, do the addition
    xterms[p.xpower] += p;
  }

  void operator*=(xy_term const &p) {
    if(p.xpower > 0) { 
      // need to shift the x's
      resize_xterms(p.xpower + nxterms + 1); 
      for(unsigned int i=nxterms;i>p.xpower;--i) {
	xterms[i-1].swap(xterms[i-(p.xpower+1)]);
      }
    }
    for(unsigned int i=0;i<nxterms;++i) { xterms[i] *= p; }
  }  

  /* ========================== */
  /* ======== OTHER OPS ======= */
  /* ========================== */

  void insert(unsigned int n, xy_term const &p) {
    // make sure enough x terms
    resize_xterms(p.xpower+1); 
    // now, do the addition
    xterms[p.xpower].insert(n,p);
  }
  
  std::string str() const {
    std::string r="";
    bool first_time=true;
    for(unsigned int i=0;i<nxterms;++i) {    
      if(!xterms[i].is_empty()) {
	if(!first_time) { r += " + "; }
	first_time=false;    
	if(i > 0) {
	  std::stringstream ss;
	  ss << i;
	  r += "x^" + ss.str();
	}
	r += xterms[i].str();
      }
    }
    return r;
  }

  double substitute(double x, double y) const {
    double r = 0.0;
    for(unsigned int i=0;i<nxterms;++i) {
      r += pow(x,(double)i) * xterms[i].substitute(y);
    }
    return r;
  }

  unsigned int nterms() const {
    unsigned int r=0;
    for(unsigned int i=0;i<nxterms;++i) {
      r += xterms[i].nterms();
    }
    return r;
  }
  
  const_iterator begin() const { return factor_poly_iterator<T>(0,nxterms,xterms); }
  const_iterator end() const { return factor_poly_iterator<T>(nxterms,nxterms,xterms); }
private:
  void destroy() {
    delete [] xterms; // will invoke yterms destructors 
  }
  
  void clone(factor_poly const &p) {
    nxterms = p.nxterms;
    xterms = new yterms<T>[nxterms];
    for(unsigned int i=0;i<nxterms;++i) {
      xterms[i] = p.xterms[i];
    }
  }

  void resize_xterms(unsigned int ns) {
    if(ns < nxterms) { return; }  
    yterms<T> *xs = new yterms<T>[ns];
    // need to use swap here, because
    // I don't want the following delete []
    // call to call destructors on my yterms
    for(unsigned int i=0;i<nxterms;++i) {
      xs[i].swap(xterms[i]);
    }
    delete [] xterms;
    xterms = xs;
    nxterms = ns;  
  }  
};

#endif

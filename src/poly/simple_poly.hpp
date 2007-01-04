#ifndef SIMPLE_POLY_HPP
#define SIMPLE_POLY_HPP

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

class term {
public:
  int xpower;
  int ypower;

  term(int x, int y) : xpower(x), ypower(y) {}

  bool operator<(term const &t) const {
    return (xpower < t.xpower) || (xpower == t.xpower && ypower < t.ypower);
  }

  bool operator==(term const &t) const {
    return (xpower == t.xpower && t.ypower == t.ypower);
  }

  std::string str() const {
    std::stringstream ss;
    if(xpower > 1) { ss << "x^" << xpower; }
    else if(xpower == 1) { ss << "x"; }
    if(ypower > 1) { ss << "y^" << ypower; }
    else if(ypower == 1) { ss << "y"; }
    return ss.str();
  }

  double substitute(double x, double y) const {
    return pow(x,xpower) * pow(y,ypower);
  }
};

class simple_poly {
  friend const simple_poly operator+(simple_poly const&, simple_poly const&);
  friend const simple_poly operator*(simple_poly const&, term const&);
private:
  std::map<term,int> terms;
public:
  simple_poly() {}

  simple_poly(int x, int y) {
    if(x > 0 || y > 0) { terms.insert(std::make_pair(term(x,y),1)); }
  }

  simple_poly const &operator=(simple_poly const &src) {
    if(&src != this) { 
      terms = src.terms;
    }
    return *this;
  }

  const std::string str() const {
    std::stringstream ss;
    // start with xs
    bool firstTime=true;
    for(std::map<term,int>::const_iterator i(terms.begin());i!=terms.end();++i) {      
      if(!firstTime) { ss << " + "; }
      firstTime=false;
      term const &t(i->first);
      int count = i->second;
      if(count > 1) { ss << count; }
      ss << i->first.str();
    }
    
    return ss.str(); 
  }

  double substitute(double x, double y) const {
    double val=0;
    for(std::map<term,int>::const_iterator i(terms.begin());i!=terms.end();++i) {   
      val += i->first.substitute(x,y) * i->second;
    }
    return val;
  }
};

// useful methods

const simple_poly operator+(simple_poly const &p1, simple_poly const &p2);
const simple_poly operator*(simple_poly const &p1, term const &p2);

#endif

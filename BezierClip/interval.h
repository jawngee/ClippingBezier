/**
 * \file
 * \brief  Simple closed interval class
 *
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
 *
 * Original Rect/Range code by:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */
#ifndef SEEN_INTERVAL_H
#define SEEN_INTERVAL_H

#include <algorithm>
#include <cmath>
#include <limits>
#import <UIKit/UIKit.h>

namespace Geom {

typedef double Coord;

const Coord EPSILON = 1e-5; //1e-18;

inline Coord infinity() {  return std::numeric_limits<Coord>::infinity();  }

//IMPL: NearConcept
inline bool are_near(Coord a, Coord b, double eps) { return fabs(a-b) <= eps; }


/* Although an Interval where _b[0] > _b[1] is considered empty, for proper functioning of other methods,
 * a proper empty Interval is [+infinity, -infinity]. Then, expandTo(p) will set the interval to [p,p].
 */
class Interval {
private:
    Coord _b[2];
    
public:
    // The default constructor creates an empty interval, that ranges from +infinity to -infinity.
    // Doing an expandTo(p) on this empty interval will correctly set the whole interval to [p,p].
    explicit Interval() { _b[0] = +infinity();  _b[1] = -infinity(); }
    explicit Interval(Coord u) { _b[0] = _b[1] = u; }
    /* When creating an Interval using the constructor specifying the exact range, the created interval
     * will be [u,v] when u<=v ; and will be [v,u] when v < u !!!
     */
    Interval(Coord u, Coord v) {
        if(u < v) {
            _b[0] = u; _b[1] = v;
        } else {
            _b[0] = v; _b[1] = u;
        }
    }
    
    double operator[](unsigned i) const {
        assert(i < 2);
        return _b[i];
    }
    inline double& operator[](unsigned i) { return _b[i]; }  //Trust the user...
    
    inline Coord min() const { return _b[0]; }
    inline Coord max() const { return _b[1]; }
    inline Coord extent() const { return _b[1] - _b[0]; }
    inline Coord middle() const { return (_b[1] + _b[0]) * 0.5; }
    
    inline bool isEmpty() const { return _b[0] > _b[1]; }
    inline bool contains(Coord val) const { return _b[0] <= val && val <= _b[1]; }
    bool contains(const Interval & val) const { return _b[0] <= val._b[0] && val._b[1] <= _b[1]; }
    bool intersects(const Interval & val) const {
        return contains(val._b[0]) || contains(val._b[1]) || val.contains(*this);
    }
    
    inline bool operator==(Interval other) const { return _b[0] == other._b[0] && _b[1] == other._b[1]; }
    inline bool operator!=(Interval other) const { return _b[0] != other._b[0] || _b[1] != other._b[1]; }
    
    //IMPL: OffsetableConcept
    //TODO: rename output_type to something else in the concept
    typedef Coord output_type;
    inline Interval operator+(Coord amnt) {
        return Interval(_b[0] + amnt, _b[1] + amnt);
    }
    inline Interval operator-(Coord amnt) {
        return Interval(_b[0] - amnt, _b[1] - amnt);
    }
    inline Interval operator+=(Coord amnt) {
        _b[0] += amnt; _b[1] += amnt;
        return *this;
    }
    inline Interval operator-=(Coord amnt) {
        _b[0] -= amnt; _b[1] -= amnt;
        return *this;
    }
    
    //IMPL: ScalableConcept
    inline Interval operator-() const { return Interval(*this); }
    inline Interval operator*(Coord s) const { return Interval(_b[0]*s, _b[1]*s); }
    inline Interval operator/(Coord s) const { return Interval(_b[0]/s, _b[1]/s); }
    Interval operator*=(Coord s) {
        if(s < 0) {
            Coord temp = _b[0];
            _b[0] = _b[1]*s;
            _b[1] = temp*s;
        } else {
            _b[0] *= s;
            _b[1] *= s;
        }
        return *this;
    }
    Interval operator/=(Coord s) {
        //TODO: what about s=0?
        if(s < 0) {
            Coord temp = _b[0];
            _b[0] = _b[1]/s;
            _b[1] = temp/s;
        } else {
            _b[0] /= s;
            _b[1] /= s;
        }
        return *this;
    }
    
    //TODO: NaN handleage for the next two?
    //TODO: Evaluate if wrap behaviour is proper.
    //If val > max, then rather than becoming a min==max range, it 'wraps' over
    void setMin(Coord val) {
        if(val > _b[1]) {
            _b[0] = _b[1];
            _b[1] = val;
        } else {
            _b[0] = val;
        }
    }
    //If val < min, then rather than becoming a min==max range, it 'wraps' over
    void setMax(Coord val) {
        if(val < _b[0]) {
            _b[1] = _b[0];
            _b[0] = val;
        } else {
            _b[1] = val;
        }
    }
    
    inline void extendTo(Coord val) {
        if(val < _b[0]) _b[0] = val;
        if(val > _b[1]) _b[1] = val;  //no else, as we want to handle NaN
    }
    
    static Interval fromArray(const Coord* c, int n) {
        assert(n > 0);
        Interval result(c[0]);
        for(int i = 1; i < n; i++) result.extendTo(c[i]);
        return result;
    }
    
    inline void expandBy(double amnt) {
        _b[0] -= amnt;
        _b[1] += amnt;
    }
    
    inline void unionWith(const Interval & a) {
        if(a._b[0] < _b[0]) _b[0] = a._b[0];
        if(a._b[1] > _b[1]) _b[1] = a._b[1];
    }
};

//IMPL: AddableConcept
inline Interval operator+(const Interval & a, const Interval & b) {
    return Interval(a.min() + b.min(), a.max() + b.max());
}
inline Interval operator-(const Interval & a, const Interval & b) {
    return Interval(a.min() - b.max(), a.max() - b.min());
}
inline Interval operator+=(Interval & a, const Interval & b) { a = a + b; return a; }
inline Interval operator-=(Interval & a, const Interval & b) { a = a - b; return a; }

//There might be impls of this based off sign checks
inline Interval operator*(const Interval & a, const Interval & b) {
    Interval res(a.min() * b.min());
    res.extendTo(a.min() * b.max());
    res.extendTo(a.max() * b.min());
    res.extendTo(a.max() * b.max());
    return res;
}
inline Interval operator*=(Interval & a, const Interval & b) { a = a * b; return a; }

/* reinstate if useful (doesn't do the proper thing for 0 inclusion)
 inline Interval operator/(const Interval & a, const Interval & b) {
 Interval res(a.min() / b.min());
 res.extendTo(a.min() / b.max());
 res.extendTo(a.max() / b.min());
 res.extendTo(a.max() / b.max());
 return res;
 }
 inline Interval operator/=(Interval & a, const Interval & b) { a = a / b; return a; }
 */

// 'union' conflicts with C keyword
inline Interval unify(const Interval & a, const Interval & b) {
    return Interval(std::min(a.min(), b.min()),
                    std::max(a.max(), b.max()));
}
//inline boost::optional<Interval> intersect(const Interval & a, const Interval & b) {
//    Coord u = std::max(a.min(), b.min()),
//          v = std::min(a.max(), b.max());
//    //technically >= might be incorrect, but singulars suck
//    return u >= v ? boost::optional<Interval>()
//                  : boost::optional<Interval>(Interval(u, v));
//}

    
}

#endif //SEEN_INTERVAL_H

/*
 Local Variables:
 mode:c++
 c-file-style:"stroustrup"
 c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
 indent-tabs-mode:nil
 fill-column:99
 End:
 */
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

/*
 *  glml.h
 *  glml
 *
 *  Created by Chris Larsen on 8/1/11.
 *  Copyright 2011-2013 Chris Larsen. All rights reserved.
 *
 * This code may be used freely for non-commercial use provided that the
 * above copyright notice and this paragraph are duplicated in all such
 * forms and that any documentation, advertising materials, and other
 * materials related to such distribution, and use acknowledge that the
 * software was developed by Chris Larsen.  The code may not be used for
 * commercial purposes without the express consent, and licensing of the
 * copyright holder.
 *
 */


#ifndef __glml__glml__
#define __glml__glml__

#include <iostream>

// Test to see if the compiler version is less than C++ 11 to make the
// following #define conditional.
#if __cplusplus < 201103L
#define constexpr
#endif  /* __cplusplus < 201103L */

// Create math library to emulate GLSL vector and matrix capabilities
// For all classes created we need
// - constructor,
// - destructor,
// - assignment operator,
// - copy constructor, and
// - print
//
// Vector classes will need
// - addition,
// - substraction,
// - left/right multiplication with a scalar,
// - vector normalization,
// - projection onto another vector of the same size,
// - calculation of vector length,
// - dot product,
// - cross product (3-tuples only), and
// - left/right multiplication by matrices.
//
// Matrix classes will need
// - addition,
// - substraction,
// - left/right multiplication with a scalar,
// - right multiplication by matrix,
// - left/right multiplication by vector,
// - determinant,
// - inversion constructor, and
// - transpose operator.
//
// 4D Matrix class will need
// - rotation constructor,
// - scaling constructor, and
// - translation contructor.
//
// Stack class of 4D transformation matrices.
//
// Function templates for implementing Spherical Linear intERPolation
// (SLERP).

#include "cmath"
#include "iostream"
#include "cassert"
#include "stdexcept"

#define DegreesToRadians(d) ((d)*M_PI/180.0)

// This function checks if two floating point values are "close enough"
// to each other to be considered equal.
//
// Need to investigate if EPSILON value can be made smaller.
inline bool withinEpsilon(const float a, const float b)
{
    static const float epsilon = 1.0e-5;

    if (a == 0.0f || b == 0.0f) {
        return fabsf(a-b) < epsilon;
    } else {
        float   max = fabsf(a) > fabsf(b) ? a : b;
        return fabsf((a-b)/max) < epsilon;
    }
}

inline bool withinEpsilon(const double a, const double b)
{
    static const double epsilon = 1.0e-13;

    if (a == 0.0 || b == 0.0) {
        return fabs(a-b) < epsilon;
    } else {
        double  max = fabs(a) > fabs(b) ? a : b;
        return fabs((a-b)/max) < epsilon;
    }
}

template <typename Type, int nrows, int ncols> class mat;
template <typename Type, int nrc> class matNxN;
template <typename Type> class cpvec;
template <typename Type> class qTemplate;

template <typename Type, int nelems>
class vec {
public:
    typedef Type    eType;

    vec() {}
    ~vec() {}
    vec &operator=(const vec &src) {
        if (this != &src) {
            for (int i=0; i!=nElems(); ++i)
                _v[i] = src._v[i];
        }
        return *this;
    }
    vec(const vec &src) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] = src._v[i];
    }

protected:
    Type    _v[nelems];
    
public:
    constexpr const int nElems(void) const { return nelems; }
    const Type *v(void) const { return _v; }
    Type *v(void) { return _v; }
    const Type operator[](const int i) const {
        assert(0<=i && i<nElems());
        return _v[i];
    }
    Type &operator[](const int i) {
        assert(0<=i && i<nElems());
        return _v[i];
    }
    vec &operator+=(const vec &rhs) {
        for (int i=0; i!=nElems(); ++i) {
            this->_v[i] += rhs._v[i];
        }
        return *this;
    }
    const vec operator+(const vec &rhs) const {
        return vec(*this) += rhs;
    }
    vec &operator-=(const vec &rhs) {
        for (int i=0; i!=nElems(); ++i) {
            this->_v[i] -= rhs._v[i];
        }
        return *this;
    }
    const vec operator-(const vec &rhs) const {
        return vec(*this) -= rhs;
    }
    vec &operator*=(const Type rhs) {
        for (int i=0; i!=nElems(); ++i) {
            this->_v[i] *= rhs;
        }
        return *this;
    }
    const vec operator*(const Type rhs) const {
        return vec(*this) *= rhs;
    }
//    vec &operator*=(const mat<Type,nelems,nelems> &rhs)
//    {
//        vec<Type,nelems>    tmp(*this);
//        for (int i=0; i!=nElems(); ++i) {
//            _v[i] = tmp[0] * rhs[0][i];
//            for (int j=1; j!=nElems(); ++j)
//                _v[i] += tmp[j] * rhs[j][i];
//        }
//        return *this;
//    }
    vec &operator*=(const mat<Type,nelems,nelems> &rhs);
    vec &operator/=(const Type rhs) {
#ifndef NDEBUG
        if (rhs == 0.0)
            throw std::runtime_error("Divide by zero");
#endif
        Type    oneOverRhs = 1.0 / rhs;
        for (int i=0; i!=nElems(); ++i) {
            this->_v[i] *= oneOverRhs;
        }
        return *this;
    }
    const vec operator/(const Type rhs) const {
        return vec(*this) /= rhs;
    }
    vec &operator/=(const matNxN<Type,nelems> &rhs);
    const vec operator/(const matNxN<Type,nelems> &rhs) const;
    // Gee, I wish I could declare dotProduct as
    // "const Type operator∙(const vec &rhs) const" so I could use it like
    // "u ∙ v".
    const Type dotProduct(const vec &rhs) const {
        Type    tmp=0.0;
        for (int i=0; i!=nElems(); ++i) {
            tmp += this->_v[i] * rhs._v[i];
        }
        return tmp;
    }
    const Type length(void) const {
        return sqrt(this->dotProduct(*this));
    }
    const vec normalize(void) const {
        return *this / length();
    }
    float safeNormalize(const vec &rhs) {
        float   l = rhs.length();
        float   m = (l!=0.0f) ? (1.0f/l) : 0.0f;
        *this *= m;
        return m;
    }
    float safeNormalize(const vec *rhs=NULL) {
        const vec  *use = rhs!=NULL ? rhs : this;
        return safeNormalize(*use);
    }
    const vec projectionOnto(const vec &u) const {
        return	u * (this->dotProduct(u)/u.dotProduct(u));
    }
    const vec operator-(void) const {
        vec tmp;
        for (int i=0; i!=nElems(); ++i)
            tmp[i] = - this->_v[i];
        return tmp;
    }
};

// This implementation of withinEpsilon for vector types is experimental.
// I'm not sure if this is the correct way to determine if each of the
// elements of the vectors a, and b are within epsilon WRT to the scale
// of the vectors a, and b.  This is just my current, best guess.
template <typename Type, int nelems>
bool withinEpsilon(const vec<Type,nelems> &a, const vec<Type,nelems> &b) {
    // Compute max(|a|, |b|).
    Type    la = a.length();
    Type    lb = b.length();

    // If the lengths don't match, there's no need to continue.
    if (!withinEpsilon(la, lb))
        return false;

    Type    l = (la>lb) ? la : lb;
    int     i;

    if (l == 0.0f) {
        // Both vectors are zero.
        return true;
    } else {
        Type    oneOverL = 1.0f / l;

        for (i=0; i!=a.nElems() && withinEpsilon(a[i]*oneOverL, b[i]*oneOverL); ++i)
            ;
    }

    return i==a.nElems();
}

template <typename Type>
class cpvec : public vec<Type,3> {
public:
    // Constructors
    cpvec(void) {}
    cpvec(const Type x, const Type y=0.0f, const Type z=0.0f) {
        vec<Type,3>::_v[0] = x;
        vec<Type,3>::_v[1] = y;
        vec<Type,3>::_v[2] = z;
    }
    explicit cpvec(const vec<Type,2> &argXY, const Type argZ=0.0f) {
        vec<Type,3>::_v[0] = argXY[0];
        vec<Type,3>::_v[1] = argXY[1];
        vec<Type,3>::_v[2] = argZ;
    }
//    cpvec(vec<eType,2> &argXY);	// Use zero for Z value
    cpvec(const Type argX, const vec<Type,2> &argYZ) {
        vec<Type,3>::_v[0] = argX;
        vec<Type,3>::_v[1] = argYZ[0];
        vec<Type,3>::_v[2] = argYZ[1];
    }
    cpvec(const vec<Type,3> &src) : vec<Type,3>(src) {}
    // See comments above for the constructor making a vec2 from a vec4.
    explicit cpvec(const vec<Type,4> &argXYZW, const bool truncate=false) {
        for (int i=0; i!=vec<Type,3>::nElems(); ++i)
            vec<Type,3>::_v[i] = argXYZW[i];
        if (!truncate && argXYZW[3]!=0.0f && argXYZW[3]!=1.0f) {
            for (int i=0; i!=vec<Type,3>::nElems(); ++i)
                vec<Type,3>::_v[i] /= argXYZW[3];
        }
    }
    explicit cpvec(const qTemplate<Type> &q) {
        *this = q.v;
    }
    // Destructor
    ~cpvec(void) {}
    // Copy Constructor
    cpvec(const cpvec &src) : vec<Type,3>(src) {}
    // Gee, I wish I could declare crossProduct as
    // "const Type operator×(const vec<Type,3> &rhs) const" so I could use
    // it like "u × v".
    const vec<Type,3> crossProduct(const vec<Type,3> &rhs) const {
        return cpvec(vec<Type,3>::_v[1]*rhs[2] - vec<Type,3>::_v[2]*rhs[1],
                     vec<Type,3>::_v[2]*rhs[0] - vec<Type,3>::_v[0]*rhs[2],
                     vec<Type,3>::_v[0]*rhs[1] - vec<Type,3>::_v[1]*rhs[0]);
    }
};

template <typename Type, int nelems>
inline const vec<Type,nelems> operator*(const Type lhs, const vec<Type,nelems> &rhs)
{
    return rhs * lhs;
}

template <typename Type, int nelems>
const bool operator==(const vec<Type,nelems> &lhs, const vec<Type,nelems> &rhs) {
    for (int i=0; i!=lhs.nElems(); ++i)
        if (lhs[i] != rhs[i]) return false;
    return true;
}

template <typename Type, int nelems>
const bool operator!=(const vec<Type,nelems> &lhs, const vec<Type,nelems> &rhs) {
    for (int i=0; i!=lhs.nElems(); ++i)
        if (lhs[i] != rhs[i]) return true;
    return false;
}

template <typename Type, int nelems>
inline std::ostream &operator<<(std::ostream &os, const vec<Type,nelems> &rhs)
{
    os << "( " << rhs[0];
    for (int i=1; i!=rhs.nElems(); ++i)
        os << ", " << rhs[i];
    os << " )";
    return os;
}

template <typename Type, int nrows, int ncols>
class mat {
public:
    typedef Type    eType;
    mat() {}
    ~mat() {}
    mat &operator=(const mat &src) {
        if (this != &src) {
            for (int i=0; i!=nRows(); ++i)
                for (int j=0; j!=nCols(); ++j)
                    _m[i][j] = src._m[i][j];
        }
        return *this;
    }
    mat(const mat &src) {
        for (int i=0; i!=nRows(); ++i)
            for (int j=0; j!=nCols(); ++j)
                _m[i][j] = src._m[i][j];
    }

protected:
    vec<Type,ncols> _m[nrows];
    
public:
    constexpr const int nRows(void) const { return nrows; }
    constexpr const int nCols(void) const { return ncols; }
    const vec<Type,ncols> getRow(const int i) const { return _m[i]; }
    const vec<Type,nrows> getColumn(const int i) const {
        vec<Type,nrows> tmp;
        for (int j=0; j!=nRows(); ++j)
            tmp[j] = _m[j][i];
        return tmp;
    }
    const vec<Type,ncols> &operator[](const int i) const {
        assert(0<=i && i<nRows());
        return _m[i];
    }
    vec<Type,ncols> &operator[](const int i) {
        assert(0<=i && i<nRows());
        return _m[i];
    }
    const Type *m(void) const {
        assert(nrows*ncols*sizeof(Type) == sizeof(_m));
        return (Type *)_m;
    }
    Type *m(void) {
        assert(nrows*ncols*sizeof(Type) == sizeof(_m));
        return (Type *)_m;
    }
    mat &operator+=(const mat &rhs) {
        for (int i=0; i!=nRows(); ++i)
            for (int j=0; j!=nCols(); ++j)
                _m[i][j] += rhs._m[i][j];
        return *this;
    }
    const mat operator+(const mat &rhs) const {
        return mat(*this) += rhs;
    }
    mat &operator-=(const mat &rhs) {
        for (int i=0; i!=nRows(); ++i)
            for (int j=0; j!=nCols(); ++j)
                _m[i][j] -= rhs._m[i][j];
        return *this;
    }
    const mat operator-(const mat &rhs) const {
        return mat(*this) -= rhs;
    }
    mat &operator*=(const mat<Type,ncols,ncols> &rhs) {
        mat tmp(*this);
        for (int i=0; i!=nRows(); ++i) {
            for (int j=0; j!=nCols(); ++j) {
                this->_m[i][j] = tmp[i][0] * rhs[0][j];
                for (int k=1; k!=nCols(); ++k) {
                    this->_m[i][j] += tmp[i][k] * rhs[k][j];
                }
            }
        }
        return *this;
    }
    mat &operator*=(const Type rhs) {
        for (int i=0; i!=nRows(); ++i)
            for (int j=0; j!=nCols(); ++j)
                _m[i][j] *= rhs;
        return *this;
    }
    const mat operator*(const Type rhs) const {
        return mat(*this) *= rhs;
    }
    const vec<Type,nrows> operator*(const vec<Type,ncols> &rhs) const {
        vec<Type,nrows> tmp;
        for (int i=0; i!=nRows(); ++i)
            tmp[i] = _m[i].dotProduct(rhs);
        return tmp;
    }
    mat &operator/=(const Type rhs) {
#ifndef NDEBUG
        if (rhs == 0.0)
            throw std::runtime_error("Divide by zero");
#endif
        Type    oneOverRhs = 1.0 / rhs;
        return *this *= oneOverRhs;
    }
    const mat operator/(const Type rhs) const {
#ifndef NDEBUG
        if (rhs == 0.0)
            throw std::runtime_error("Divide by zero");
#endif
        Type    oneOverRhs = 1.0 / rhs;
        return mat(*this) *= oneOverRhs;
    }
    mat &operator/=(const matNxN<Type,ncols> &rhs);
    const mat operator/(const matNxN<Type,ncols> &rhs) const;
    const mat<Type,ncols,nrows> transpose(void) const {
        mat<Type,ncols,nrows>   tmp;
        for (int i=0; i!=nRows(); ++i)
            for (int j=0; j!=nCols(); ++j)
                tmp[j][i] = _m[i][j];
        return tmp;
    }
    const mat operator-(void) const {
        mat tmp;
        for (int i=0; i!=nRows(); ++i)
            for (int j=0; j!=nCols(); ++j)
                tmp[i][j] = - _m[i][j];
        return tmp;
    }
};

template <typename Type, int nelems>
inline vec<Type,nelems> &vec<Type,nelems>::operator*=(const mat<Type,nelems,nelems> &rhs)
{
    vec<Type,nelems>    tmp(*this);
    for (int i=0; i!=nElems(); ++i) {
        _v[i] = tmp[0] * rhs[0][i];
        for (int j=1; j!=nElems(); ++j)
            _v[i] += tmp[j] * rhs[j][i];
    }
    return *this;
}

template <typename Type, int nrows, int ncols>
inline const mat<Type,nrows,ncols> operator*(const Type lhs, const mat<Type,nrows,ncols> &rhs)
{
    return rhs * lhs;
}
    
template <typename Type, int nrows, int ncols>
inline const vec<Type,ncols> operator*(const vec<Type,nrows> &lhs, const mat<Type,nrows,ncols> &rhs)
{
    vec<Type,ncols> tmp;
    for (int i=0; i!=ncols; ++i) {
        tmp[i] = lhs[0] * rhs[0][i];
        for (int j=1; j!=nrows; ++j) {
            tmp[i] += lhs[j] * rhs[j][i];
        }
    }
    return tmp;
}
    
template <typename Type, int nrows, int nrc, int ncols>
inline const mat<Type,nrows,ncols> operator*(const mat<Type,nrows,nrc> &lhs, const mat<Type,nrc,ncols> &rhs) {
    mat<Type,nrows,ncols>   tmp;
    for (int i=0; i!=nrows; ++i) {
        for (int j=0; j!=ncols; ++j) {
            tmp[i][j] = lhs[i][0] * rhs[0][j];
            for (int k=1; k!=nrc; ++k) {
                tmp[i][j] += lhs[i][k] * rhs[k][j];
            }
        }
    }
    return tmp;
}
    
template <typename Type, int nrows, int ncols>
inline std::ostream &operator<<(std::ostream &os, const mat<Type,nrows,ncols> &rhs)
{
    os << "( " << rhs[0];
    for (int i=1; i!=rhs.nRows(); ++i)
        os << ", " << rhs[i];
    os << " )";
    return os;
}

template <typename Type, int nrc>
class matNxN : public mat<Type, nrc, nrc> {
public:
    matNxN(void) {}
    ~matNxN(void) {}
    matNxN(const matNxN &src) : mat<Type,nrc,nrc>(src) {}
    matNxN(const mat<Type,nrc,nrc> &src) : mat<Type,nrc,nrc>(src) {}
    matNxN &loadIdentity(void) {
        for (int i=0; i!=mat<Type,nrc,nrc>::nRows(); ++i) {
            int j;
            for (j=0; j!=i; ++j)
                mat<Type,nrc,nrc>::_m[i][j] = 0.0f;
            mat<Type,nrc,nrc>::_m[i][j] = 1.0f;
            for (++j; j!=mat<Type,nrc,nrc>::nCols(); ++j)
                mat<Type,nrc,nrc>::_m[i][j] = 0.0f;
        }
        return *this;
    }
    const Type determinant(void) const;
    const mat<Type,nrc,nrc> adjoint(void) const;
    const mat<Type,nrc,nrc> inverse(void) const;
};

template <>
inline const float matNxN<float,1>::determinant(void) const {
    return _m[0][0];
}

template <>
inline const double matNxN<double,1>::determinant(void) const {
    return _m[0][0];
}

template <typename Type, int nrc>
inline const Type matNxN<Type,nrc>::determinant(void) const {
    vec<Type,nrc>       c1;
    matNxN<Type,nrc-1>  tmp;
    const int   i=0;
    
    for (int j=0; j!=this->nCols(); ++j) {
        for (int i2=0, i3=0; i2!=this->nRows(); ++i2) {
            if (i2 != i) {
                for (int j2=0, j3=0; j2!=this->nCols(); ++j2) {
                    if (j2 != j) {
                        tmp[i3][j3++] = this->_m[i2][j2];
                    }
                }
                ++i3;
            }
        }
        Type    tmpDet = tmp.determinant();
        if ((i+j)&1) {
            c1[j] = -tmpDet;
        } else {
            c1[j] =  tmpDet;
        }
    }
    
    return c1.dotProduct(this->_m[0]);
}

template <>
inline const mat<float,1,1> matNxN<float,1>::adjoint(void) const {
    mat<float,1,1>  tmp;
    tmp[0][0] = 1.0f;
    return tmp;
}

template <>
inline const mat<double,1,1> matNxN<double,1>::adjoint(void) const {
    mat<double,1,1> tmp;
    tmp[0][0] = 1.0;
    return tmp;
}

template <typename Type, int nrc>
inline const mat<Type,nrc,nrc> matNxN<Type,nrc>::adjoint(void) const {
    mat<Type,nrc,nrc>   a;
    matNxN<Type,nrc-1>  tmp;
    for (int j=0; j!=mat<Type,nrc,nrc>::nCols(); ++j) {
        for (int i=0; i!=mat<Type,nrc,nrc>::nRows(); ++i) {
            for (int i2=0, i3=0; i2!=this->nRows(); ++i2) {
                if (i2 != i) {
                    for (int j2=0, j3=0; j2!=this->nCols(); ++j2) {
                        if (j2 != j) {
                            tmp[i3][j3++] = this->_m[i2][j2];
                        }
                    }
                    ++i3;
                }
            }
            Type    tmpDet = tmp.determinant();
            if ((i+j)&1) {
                a[j][i] = -tmpDet;
            } else {
                a[j][i] =  tmpDet;
            }
        }
    }
    return a;
}

template <>
inline const mat<float,1,1> matNxN<float,1>::inverse(void) const
{
#ifndef NDEBUG
    if (_m[0][0] == 0.0f)
        throw std::runtime_error("Divide by zero");
#endif
    mat<float,1,1>  tmp;
    tmp[0][0] = 1.0f / _m[0][0];
    return tmp;
}

template <>
inline const mat<double,1,1> matNxN<double,1>::inverse(void) const
{
#ifndef NDEBUG
    if (_m[0][0] == 0.0)
        throw std::runtime_error("Divide by zero");
#endif
    mat<double,1,1> tmp;
    tmp[0][0] = 1.0 / _m[0][0];
    return tmp;
}

template <typename Type, int nrc>
inline const mat<Type,nrc,nrc> matNxN<Type,nrc>::inverse(void) const
{
    mat<Type,nrc,nrc>   a;
    matNxN<Type,nrc-1>  tmp;
    Type    det=0.0;
    for (int j=0; j!=mat<Type,nrc,nrc>::nCols(); ++j) {
        for (int i=0; i!=mat<Type,nrc,nrc>::nRows(); ++i) {
            for (int i2=0, i3=0; i2!=this->nRows(); ++i2) {
                if (i2 != i) {
                    for (int j2=0, j3=0; j2!=this->nCols(); ++j2) {
                        if (j2 != j) {
                            tmp[i3][j3++] = this->_m[i2][j2];
                        }
                    }
                    ++i3;
                }
            }
            Type    tmpDet = tmp.determinant();
            if ((i+j)&1) {
                a[j][i] = -tmpDet;
            } else {
                a[j][i] =  tmpDet;
            }
        }
        if (j==0) {
            for (int k=0; k!=mat<Type,nrc,nrc>::nRows(); ++k) {
                det += a[0][k] * this->_m[k][0];
            }
#ifndef NDEBUG
            if (det == 0.0)
                throw std::runtime_error("Divide by zero");
#endif
        }
    }
    return a / det;
}

template <typename Type, int nrc>
inline const mat<Type, nrc, nrc> inverse(const mat<Type, nrc, nrc> &m) {
    return ((matNxN<Type, nrc> *)(&m))->inverse();
}

template <typename Type, int nelems>
inline vec<Type,nelems> &vec<Type,nelems>::operator/=(const matNxN<Type,nelems> &rhs)
{
    return *this *= rhs.inverse();
}

template <typename Type, int nelems>
inline const vec<Type,nelems> vec<Type,nelems>::operator/(const matNxN<Type,nelems> &rhs) const
{
    return vec(*this) *= rhs.inverse();
}

template <typename Type, int nrows, int ncols>
inline mat<Type,nrows,ncols> &mat<Type,nrows,ncols>::operator/=(const matNxN<Type,ncols> &rhs) {
    return *this *= rhs.inverse();
}

template <typename Type, int nrows, int ncols>
inline const mat<Type,nrows,ncols> mat<Type,nrows,ncols>::operator/(const matNxN<Type,ncols> &rhs) const {
    return mat(*this) *= rhs.inverse();
}

// This implementation of withinEpsilon for matrix types is experimental.
// I'm not sure if this is the correct way to determine if each of the
// elements of the matrices a, and b are within epsilon WRT to the scale
// of the matrices a, and b.  This is just my current, best guess.  Also,
// note that this implementation only works for square matrices because
// it requires calculation of the determinant.
template <typename Type, int nrc>
bool withinEpsilon(const matNxN<Type,nrc> &a, const matNxN<Type,nrc> &b) {
    // Compute max(|a|, |b|).
    Type    da = a.determinant();   // Unlike vector length,
    Type    db = b.determinant();   // determinants can be negative.

    // If the determinants don't match, there's no need to continue.
    if (!withinEpsilon(da, db))
        return false;

    Type    d = (fabs(da)>fabs(db)) ? fabs(da) : fabs(db);
    int     i;

    // Here things get a bit tricky.  Two matrices could have
    // determinants which are zero, yet have elements which are wildly
    // different from each other.  Unlike the vector case we can't short
    // circuit the test just because the determinants are zero.
    if (d == 0.0) {
        for (i=0; i!=nrc && withinEpsilon(a[i], b[i]); ++i)
            ;
    } else {
        Type    oneOverD = 1.0 / d;

        for (i=0; i!=nrc && withinEpsilon(a[i]*oneOverD, b[i]*oneOverD); ++i)
            ;
    }

    return i==nrc;
}

class vec3;
class vec4;
class mat2;
class mat2x3;
class mat2x4;
class mat3x2;
class mat3;
class mat3x4;
class mat4x2;
class mat4x3;
class mat4;
class quaternion;
class dquaternion;

class vec2 : public vec<float,2> {
private:
    // Used below to enable the use of named components
    typedef struct { eType x, y; } _CRL_vf2;
public:
    // Constructors
    vec2(void) {}
    vec2(const eType x, const eType y=0.0f) {
        _v[0] = x;
        _v[1] = y;
    }
    vec2(const vec<eType,2> &src) : vec<eType,2>(src) {}
    explicit vec2(const vec<eType,3> &argXYZ) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] = argXYZ[i];
    }
    // We're probably converting out of homogeneous space so, if possible (W≠0)
    // and needed (W≠1), we divide by W.
    // If W=0 then argXYZW probably represents a direction, not a location, and
    // this is also a valid use of 4-tuples.
    explicit vec2(const vec4 &argXYZW, const bool truncate=false);
    // Copy Constructor
    vec2(const vec2 &src) : vec<eType,2>(src) {}
    // Destructor
    ~vec2(void) {}
    // Enable use of named components
    _CRL_vf2 *operator->() { return reinterpret_cast<_CRL_vf2 *>(this); }
    const _CRL_vf2 *operator->() const { return reinterpret_cast<const _CRL_vf2 *>(this); }
};
    
typedef vec2 fvec2;

class dvec2 : public vec<double,2> {
private:
    // Used below to enable the use of named components
    typedef struct { eType x, y; } _CRL_vd2;
public:
    // Constructors
    dvec2(void) {}
    dvec2(const eType x, const eType y=0.0f) {
        _v[0] = x;
        _v[1] = y;
    }
    dvec2(const vec<eType,2> &src) : vec<eType,2>(src) {}
    explicit dvec2(const vec<eType,3> &argXYZ) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] = argXYZ[i];
    }
    // We're probably converting out of homogeneous space so, if possible (W≠0)
    // and needed (W≠1), we divide by W.
    // If W=0 then argXYZW probably represents a direction, not a location, and
    // this is also a valid use of 4-tuples.
    explicit dvec2(const vec4 &argXYZW, const bool truncate=false);
    // Copy Constructor
    dvec2(const dvec2 &src) : vec<eType,2>(src) {}
    // Destructor
    ~dvec2(void) {}
    // Enable use of named components
    _CRL_vd2 *operator->() { return reinterpret_cast<_CRL_vd2 *>(this); }
    const _CRL_vd2 *operator->() const { return reinterpret_cast<const _CRL_vd2 *>(this); }
};

class vec3 : public cpvec<float> {
private:
    // Used below to enable the use of named components
    typedef struct { eType x, y, z; } _CRL_vf3;
public:
    // Constructors
    vec3(void) {}
    vec3(const eType x, const eType y=0.0f, const eType z=0.0f) : cpvec<eType>(x,y,z) {}
    explicit vec3(const vec<eType,2> &argXY, const eType argZ=0.0f) : cpvec<eType>(argXY,argZ) {}
//    vec3(vec2 &argXY);	// Use zero for Z value
    vec3(const eType argX, const vec<eType,2> &argYZ) : cpvec<eType>(argX,argYZ) {}
    vec3(const vec<eType,3> &src) : cpvec<eType>(src) {}
    // See comments above for the constructor making a vec2 from a vec4.
    explicit vec3(const vec<eType,4> &argXYZW, const bool truncate=true) : cpvec<eType>(argXYZW, truncate) {}
    explicit vec3(const qTemplate<eType> &q) : cpvec<eType>(q) {}
    // Copy Constructor
    vec3(const vec3 &src) : cpvec<eType>(src) {}
    // Destructor
    ~vec3(void) {}
    _CRL_vf3 *operator->() { return reinterpret_cast<_CRL_vf3 *>(this); }
    const _CRL_vf3 *operator->() const { return reinterpret_cast<const _CRL_vf3 *>(this); }
};
    
typedef vec3 fvec3;

class dvec3 : public cpvec<double> {
private:
    // Used below to enable the use of named components
    typedef struct { eType x, y, z; } _CRL_vd3;
public:
    // Constructors
    dvec3(void) {}
    dvec3(const eType x, const eType y=0.0f, const eType z=0.0f) : cpvec<eType>(x,y,z) {}
    explicit dvec3(const vec<eType,2> &argXY, const eType argZ=0.0f) : cpvec<eType>(argXY,argZ) {}
//    dvec3(vec2 &argXY);	// Use zero for Z value
    dvec3(const eType argX, const vec<eType,2> &argYZ) : cpvec<eType>(argX,argYZ) {}
    dvec3(const vec<eType,3> &src) : cpvec<eType>(src) {}
    // See comments above for the constructor making a vec2 from a vec4.
    explicit dvec3(const vec<eType,4> &argXYZW, const bool truncate=true) : cpvec<eType>(argXYZW, truncate) {}
    explicit dvec3(const qTemplate<eType> &q) : cpvec<eType>(q) {}
    // Copy Constructor
    dvec3(const dvec3 &src) : cpvec<eType>(src) {}
    // Destructor
    ~dvec3(void) {}
    _CRL_vd3 *operator->() { return reinterpret_cast<_CRL_vd3 *>(this); }
    const _CRL_vd3 *operator->() const { return reinterpret_cast<const _CRL_vd3 *>(this); }
};
    
// The default constructor for the vec4, type (like the default
// constructors for the vec2, and vec3 types) doesn't do anything; I
// kept running into cases where using resources to give the various
// vector types a default value was a waste of resources because
// immediately after creating the vector its values would be changed.
// However, I optimized the vec4 and mat4 classes to do 3D affine
// transformations in a 4D homogeneous space.  For this reason the other
// constructors set the the "w" component to 1, by default.  If this
// class were to be used for other purposes some other default value
// might make more sense.
class vec4 : public vec<float,4> {
private:
    // Used below to enable the use of named components
    typedef struct { eType x, y, z, w; } _CRL_vf4;
public:
    // Constructors
    vec4(void) {}
    vec4(const eType argX, const eType argY=0.0f, const eType argZ=0.0f, const eType argW=1.0f) {
        _v[0] = argX;
        _v[1] = argY;
        _v[2] = argZ;
        _v[3] = argW;
    }
//    vec4(const eType argX, const eType argY, const eType argZ);
//    vec4(const eType argX, const eType argY);
//    vec4(const eType argX);
    vec4(const eType argX, const vec<eType,2> &argYZ, const eType argW=1.0f) {
        _v[0] = argX;
        _v[1] = argYZ[0];
        _v[2] = argYZ[1];
        _v[3] = argW;
    }
//    vec4(const eType argX, const vec<eType,2> &argYZ);
    vec4(const eType argX, const eType argY, const vec<eType,2> &argZW) {
        _v[0] = argX;
        _v[1] = argY;
        _v[2] = argZW[0];
        _v[3] = argZW[1];
    }
    vec4(const eType argX, const vec<eType,3> &argYZW) {
        _v[0] = argX;
        _v[1] = argYZW[0];
        _v[2] = argYZW[1];
        _v[3] = argYZW[2];
    }
    //
    explicit vec4(const vec<eType,2> &argXY, const eType argZ=0.0, const eType argW=1.0) {
        _v[0] = argXY[0];
        _v[1] = argXY[1];
        _v[2] = argZ;
        _v[3] = argW;
    }
//    vec4(const vec<eType,2> &argXY, const eType argZ=0.0f);
//    vec4(const vec<eType,2> &argXY);
    vec4(const vec<eType,2> &argXY, const vec<eType,2> &argZW) {
        _v[0] = argXY[0];
        _v[1] = argXY[1];
        _v[2] = argZW[0];
        _v[3] = argZW[1];
    }
    //
    explicit vec4(const vec<eType,3> &argXYZ, const eType argW=1.0) {
        _v[0] = argXYZ[0];
        _v[1] = argXYZ[1];
        _v[2] = argXYZ[2];
        _v[3] = argW;
    }
//    vec4(const vec<float,3> &argXYZ);
    vec4(const vec<eType,4> &src) : vec<eType,4>(src) {}
    // Copy Constructor
    vec4(const vec4 &src) : vec<eType,4>(src) {}
    // Destructor
    ~vec4(void) {}
    // Enable use of named components
    _CRL_vf4 *operator->() { return reinterpret_cast<_CRL_vf4 *>(this); }
    const _CRL_vf4 *operator->() const { return reinterpret_cast<const _CRL_vf4 *>(this); }
};

inline vec2::vec2(const vec4 &argXYZW, const bool truncate) {
    for (int i=0; i!=nElems(); ++i)
        _v[i] = argXYZW[i];
    if (!truncate && argXYZW[3]!=0.0f && argXYZW[3]!=1.0f) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] /= argXYZW[3];
    }
}

typedef vec4 fvec4;

class dvec4 : public vec<double,4> {
private:
    // Used below to enable the use of named components
    typedef struct { eType x, y, z, w; } _CRL_vd4;
public:
    // Constructors
    dvec4(void) {}
    dvec4(const eType argX, const eType argY=0.0, const eType argZ=0.0, const eType argW=1.0) {
        _v[0] = argX;
        _v[1] = argY;
        _v[2] = argZ;
        _v[3] = argW;
    }
//    dvec4(const eType argX, const eType argY, const eType argZ);
//    dvec4(const eType argX, const eType argY);
//    dvec4(const eType argX);
    dvec4(const eType argX, const vec<eType,2> &argYZ, const eType argW=1.0) {
        _v[0] = argX;
        _v[1] = argYZ[0];
        _v[2] = argYZ[1];
        _v[3] = argW;
    }
//    dvec4(const eType argX, const vec<eType,2> &argYZ);
    dvec4(const eType argX, const eType argY, const vec<eType,2> &argZW) {
        _v[0] = argX;
        _v[1] = argY;
        _v[2] = argZW[0];
        _v[3] = argZW[1];
    }
    dvec4(const eType argX, const vec<eType,3> &argYZW) {
        _v[0] = argX;
        _v[1] = argYZW[0];
        _v[2] = argYZW[1];
        _v[3] = argYZW[2];
    }
//
    explicit dvec4(const vec<eType,2> &argXY, const eType argZ=0.0, const eType argW=1.0) {
        _v[0] = argXY[0];
        _v[1] = argXY[1];
        _v[2] = argZ;
        _v[3] = argW;
    }
//    dvec4(const vec<eType,2> &argXY, const eType argZ=0.0);
//    dvec4(const vec<eType,2> &argXY);
    dvec4(const vec<eType,2> &argXY, const vec<eType,2> &argZW) {
        _v[0] = argXY[0];
        _v[1] = argXY[1];
        _v[2] = argZW[0];
        _v[3] = argZW[1];
    }
//
    explicit dvec4(const vec<eType,3> &argXYZ, const eType argW=1.0) {
        _v[0] = argXYZ[0];
        _v[1] = argXYZ[1];
        _v[2] = argXYZ[2];
        _v[3] = argW;
    }
//    dvec4(const vec<eType,3> &argXYZ);
    dvec4(const vec<eType,4> &src) : vec<eType,4>(src) {}
    // Copy Constructor
    dvec4(const dvec4 &src) : vec<eType,4>(src) {}
    // Destructor
    ~dvec4(void) {}
    // Enable use of named components
    _CRL_vd4 *operator->() { return reinterpret_cast<_CRL_vd4 *>(this); }
    const _CRL_vd4 *operator->() const { return reinterpret_cast<const _CRL_vd4 *>(this); }
};

inline dvec2::dvec2(const vec4 &argXYZW, const bool truncate) {
    for (int i=0; i!=nElems(); ++i)
        _v[i] = argXYZW[i];
    if (!truncate && argXYZW[3]!=0.0f && argXYZW[3]!=1.0f) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] /= argXYZW[3];
    }
}

// The default constructors for each of the matrix classes (mat2, mat3,
// and mat4) don't do anything.  In too many cases, as soon as the
// object is created any default value I might have chosen doesn't match
// the need "of the moment" and the contents are changed immediately
// after object creation.  Thus making the CPU cycles, and memory space
// to initialize the object an utter waste of resources.  In addition to
// constructing a matrix by default there are constructor methods for
// creating the matrix from a collection of rows, or from individual
// scalars.  All of the matrix types are "row major" implementations
// because it's traditional for both mathematics and the C programming
// language.  [Note:  Why OpenGL uses column major matrices I'll never
// know.  I can only speculate it was to be more compatible with
// FORTRAN.]  If you have a list of column vectors you wish to use to
// initialize a matrix you'll need to use the constructor which takes
// scalar arguments.  For example, to create a  mat2 variable out of
// column vectors c1 and c2 you would do something like:
//     vec2	c1, c2;
//     ...	// Initialize c1 and c2 to suitable values
//     mat2	cMajorMatrix(c1[0], c2[0],
//                           c1[1], c2[1]);
// The reader should easily be able to figure how to do something
// similar for the 3D and 4D matrix cases.
class mat2 : public matNxN<float,2> {
public:
    // Constructors
    mat2(void) {}
    mat2(const eType x11,      const eType x12=0.0f,
         const eType x21=0.0f, const eType x22=1.0f)
    {
        _m[0][0] = x11;
        _m[0][1] = x12;
        _m[1][0] = x21;
        _m[1][1] = x22;
    }
    mat2(const vec<eType,2> &r1, const eType x21=0.0f, const eType x22=1.0f) {
        _m[0][0] = r1[0];
        _m[0][1] = r1[1];
        _m[1][0] = x21;
        _m[1][1] = x22;
    }
    mat2(const vec<eType,2> &r1, const vec<eType,2> &r2) {
        _m[0] = r1;
        _m[1] = r2;
    }
    mat2(const mat<eType,2,2> &src) : matNxN<eType,2>(src) {}
    // Copy Constructor
    mat2(const mat2 &src) : matNxN<eType,2>(src) {}
    // Destructor
    ~mat2() {}
};
    
typedef mat2 fmat2;
typedef mat2 mat2x2;
typedef mat2 fmat2x2;

class dmat2 : public matNxN<double,2> {
public:
    // Constructors
    dmat2(void) {}
    dmat2(const eType x11,     const eType x12=0.0,
          const eType x21=0.0, const eType x22=1.0)
    {
        _m[0][0] = x11;
        _m[0][1] = x12;
        _m[1][0] = x21;
        _m[1][1] = x22;
    }
    dmat2(const vec<eType,2> &r1, const eType x21=0.0, const eType x22=1.0) {
        _m[0][0] = r1[0];
        _m[0][1] = r1[1];
        _m[1][0] = x21;
        _m[1][1] = x22;
    }
    dmat2(const vec<eType,2> &r1, const vec<eType,2> &r2) {
        _m[0] = r1;
        _m[1] = r2;
    }
    dmat2(const mat<eType,2,2> &src) : matNxN<eType,2>(src) {}
    // Copy Constructor
    dmat2(const dmat2 &src) : matNxN<eType,2>(src) {}
    //Destructor
    ~dmat2() {}
};
    
typedef dmat2 dmat2x2;
    
template <>
inline const float matNxN<float,2>::determinant(void) const {
    return _m[0][0]*_m[1][1] - _m[0][1]*_m[1][0];
}

template <>
inline const double matNxN<double,2>::determinant(void) const {
    return _m[0][0]*_m[1][1] - _m[0][1]*_m[1][0];
}

template <>
inline const mat<float,2,2> matNxN<float,2>::adjoint(void) const {
    return mat2( _m[1][1], -_m[0][1],
                -_m[1][0],  _m[0][0]);
}

template <>
inline const mat<double,2,2> matNxN<double,2>::adjoint(void) const {
    return dmat2( _m[1][1], -_m[0][1],
                 -_m[1][0],  _m[0][0]);
}

template <>
inline const mat<float,2,2> matNxN<float,2>::inverse(void) const
{
    eType   det = determinant();
#ifndef NDEBUG
    if (det == 0.0f)
        throw std::runtime_error("Divide by zero");
#endif
    eType   inverseDet = 1.0f / det;
    mat2    tmp( this->_m[1][1]*inverseDet, -this->_m[0][1]*inverseDet,
                -this->_m[1][0]*inverseDet,  this->_m[0][0]*inverseDet);
    return tmp;
}

template <>
inline const mat<double,2,2> matNxN<double,2>::inverse(void) const
{
    eType   det = determinant();
#ifndef NDEBUG
    if (det == 0.0)
        throw std::runtime_error("Divide by zero");
#endif
    eType   inverseDet = 1.0 / det;
    dmat2   tmp( this->_m[1][1]*inverseDet, -this->_m[0][1]*inverseDet,
                -this->_m[1][0]*inverseDet,  this->_m[0][0]*inverseDet);
    return tmp;
}

class mat2x3 : public mat<float,2,3> {
public:
    // Constructors
    mat2x3(void) {}
    mat2x3(const eType x11,      const eType x12=0.0f, const eType x13=0.0f,
           const eType x21=0.0f, const eType x22=0.0f, const eType x23=0.0f) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23;
    }
    mat2x3(const vec<eType,3> &r0, const vec<eType,3> &r1) { _m[0] = r0; _m[1] = r1; }
    mat2x3(const mat<eType,2,3> &rhs) {
        for (int i=0; i<nRows(); ++i)
            _m[i] = rhs[i];
    }
    // Copy Constructor
    mat2x3(const mat2x3 &rhs) {
        for (int i=0; i<nRows(); ++i)
            _m[i] = rhs[i];
    }
    // Destructor
    ~mat2x3(void) {}
};
    
typedef mat2x3 fmat2x3;

class dmat2x3 : public mat<double,2,3> {
public:
    dmat2x3(void) {}
    dmat2x3(const eType x11,   const eType x12=0, const eType x13=0,
            const eType x21=0, const eType x22=0, const eType x23=0) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23;
    }
    dmat2x3(const vec<eType,3> &r0, const vec<eType,3> &r1) { _m[0] = r0; _m[1] = r1; }
    dmat2x3(const mat<eType,2,3> &rhs) {
        for (int i=0; i<nRows(); ++i)
            _m[i] = rhs[i];
    }
    // Copy Constructor
    dmat2x3(const dmat2x3 &rhs) {
        for (int i=0; i<nRows(); ++i)
            _m[i] = rhs[i];
    }
    // Destructor
    ~dmat2x3(void) {}
};
    
class mat2x4 : public mat<float,2,4> {
public:
    // Constructors
    mat2x4(void) {}
    mat2x4(const eType x11,      const eType x12=0.0f, const eType x13=0.0f, const eType x14=0.0f,
           const eType x21=0.0f, const eType x22=0.0f, const eType x23=0.0f, const eType x24=0.0f) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23;
    }
    mat2x4(const vec<eType,4> &r0, const vec<eType,4> &r1) { _m[0] = r0; _m[1] = r1; }
    mat2x4(const mat<eType,2,4> &rhs) {
        for (int i=0; i<nRows(); ++i)
            _m[i] = rhs[i];
    }
    // Copy Constructor
    mat2x4(const mat2x4 &rhs) {
        for (int i=0; i<nRows(); ++i)
            _m[i] = rhs[i];
    }
    // Destructor
    ~mat2x4(void) {}
};
    
typedef mat2x4  fmat2x4;
    
class dmat2x4 : public mat<double,2,4> {
public:
    // Constructors
    dmat2x4(void) {}
    dmat2x4(const eType x11,   const eType x12=0, const eType x13=0, const eType x14=0,
            const eType x21=0, const eType x22=0, const eType x23=0, const eType x24=0) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23;
    }
    dmat2x4(const vec<eType,4> &r0, const vec<eType,4> &r1) { _m[0] = r0; _m[1] = r1; }
    dmat2x4(const mat<eType,2,4> &rhs) {
        for (int i=0; i<nRows(); ++i)
            _m[i] = rhs[i];
    }
    // Copy Constructor
    dmat2x4(const dmat2x4 &rhs) {
        for (int i=0; i<nRows(); ++i)
            _m[i] = rhs[i];
    }
    // Destructor
    ~dmat2x4(void) {}
};
    
class mat3x2 : public mat<float,3,2> {
public:
    // Constructors
    mat3x2(void) {}
    mat3x2(const eType x11,      const eType x12=0.0f,
           const eType x21=0.0f, const eType x22=0.0f,
           const eType x31=0.0f, const eType x32=0.0f) {
        _m[0][0] = x11; _m[0][1] = x12;
        _m[1][0] = x21; _m[1][1] = x22;
        _m[2][0] = x31; _m[2][1] = x32;
    }
    mat3x2(const vec<eType,2> &r1, const vec<eType,2> &r2, const vec<eType,2> &r3) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3;
    }
    mat3x2(const mat<eType,3,2> &src) {
        for (int i=0; i!=nRows(); ++i)
            for (int j=0; j!=nCols(); ++j)
                _m[i][j] = src[i][j];
    }
    // Copy Constructor
    mat3x2(const mat3x2 &src) {
        for (int i=0; i!=nRows(); ++i)
            for (int j=0; j!=nCols(); ++j)
                _m[i][j] = src[i][j];
    }
    // Destructor
    ~mat3x2() {}
};
    
typedef mat3x2  fmat3x2;
    
class dmat3x2 : public mat<double,3,2> {
public:
    // Constructors
    dmat3x2(void) {}
    dmat3x2(const eType x11,     const eType x12=0.0,
            const eType x21=0.0, const eType x22=0.0,
            const eType x31=0.0, const eType x32=0.0) {
        _m[0][0] = x11; _m[0][1] = x12;
        _m[1][0] = x21; _m[1][1] = x22;
        _m[2][0] = x31; _m[2][1] = x32;
    }
    dmat3x2(const vec<eType,2> &r1, const vec<eType,2> &r2, const vec<eType,2> &r3) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3;
    }
    dmat3x2(const mat<eType,3,2> &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Copy Constructor
    dmat3x2(const dmat3x2 &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Destructor
    ~dmat3x2() {}
};
    
class mat3 : public matNxN<float,3> {
public:
    // Constructors
    mat3(void) {}
    mat3(const eType x11,      const eType x12=0.0f, const eType x13=0.0f,
         const eType x21=0.0f, const eType x22=1.0f, const eType x23=0.0f,
         const eType x31=0.0f, const eType x32=0.0f, const eType x33=1.0f) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23;
        _m[2][0] = x31; _m[2][1] = x32; _m[2][2] = x33;
    }
    mat3(const vec<eType,3> &r1, const vec<eType,3> &r2, const vec<eType,3> &r3) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3;
    }
    mat3(const mat<eType,3,3> &src) : matNxN<eType,3>(src) {}
    // Copy Constructor
    mat3(const mat3 &src) : matNxN<eType,3>(src) {}
    // Destructor
    ~mat3(void) {}
};
    
typedef mat3 fmat3;
typedef mat3 mat3x3;
typedef mat3 fmat3x3;

class dmat3 : public matNxN<double,3> {
public:
    // Constructors
    dmat3(void) {}
    dmat3(const eType x11,     const eType x12=0.0, const eType x13=0.0,
          const eType x21=0.0, const eType x22=1.0, const eType x23=0.0,
          const eType x31=0.0, const eType x32=0.0, const eType x33=1.0) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23;
        _m[2][0] = x31; _m[2][1] = x32; _m[2][2] = x33;
    }
    dmat3(const vec<eType,3> &r1, const vec<eType,3> &r2, const vec<eType,3> &r3) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3;
    }
    dmat3(const mat<eType,3,3> &src) : matNxN<double,3>(src) {}
    // Copy Constructor
    dmat3(const dmat3 &src) : matNxN<double,3>(src) {}
    // Destructor
    ~dmat3() {}
};
    
typedef dmat3 dmat3x3;
    
template <>
inline const float matNxN<float,3>::determinant(void) const {
    return static_cast<vec3>(_m[0]).crossProduct(_m[1]).dotProduct(_m[2]);
}

template <>
inline const double matNxN<double,3>::determinant(void) const {
    return static_cast<dvec3>(_m[0]).crossProduct(_m[1]).dotProduct(_m[2]);
}
    
template <>
inline const mat<float,3,3> matNxN<float,3>::adjoint(void) const {
    vec3    c0(static_cast<vec3>(_m[1]).crossProduct(this->_m[2]));
    vec3    c1(static_cast<vec3>(_m[2]).crossProduct(this->_m[0]));
    vec3    c2(static_cast<vec3>(_m[0]).crossProduct(this->_m[1]));

    return mat3(c0[0], c1[0], c2[0],
                c0[1], c1[1], c2[1],
                c0[2], c1[2], c2[2]);
}

template <>
inline const mat<double,3,3> matNxN<double,3>::adjoint(void) const {
    dvec3   c0(static_cast<dvec3>(_m[1]).crossProduct(this->_m[2]));
    dvec3   c1(static_cast<dvec3>(_m[2]).crossProduct(this->_m[0]));
    dvec3   c2(static_cast<dvec3>(_m[0]).crossProduct(this->_m[1]));
        
    return dmat3(c0[0], c1[0], c2[0],
                 c0[1], c1[1], c2[1],
                 c0[2], c1[2], c2[2]);
}
    
template <>
inline const mat<float,3,3> matNxN<float,3>::inverse(void) const {
    vec3    c2(static_cast<vec3>(_m[0]).crossProduct(this->_m[1]));
    eType   det = c2.dotProduct(this->_m[2]);
#ifndef NDEBUG
    if (det == 0.0f)
        throw std::runtime_error("Divide by zero");
#endif
    eType   oneOverDet = 1.0 / det;
    vec3    c0(static_cast<vec3>(_m[1]).crossProduct(this->_m[2]));
    vec3    c1(static_cast<vec3>(_m[2]).crossProduct(this->_m[0]));

    return mat3(c0[0] * oneOverDet, c1[0] * oneOverDet, c2[0] * oneOverDet,
                c0[1] * oneOverDet, c1[1] * oneOverDet, c2[1] * oneOverDet,
                c0[2] * oneOverDet, c1[2] * oneOverDet, c2[2] * oneOverDet);
}

template <>
inline const mat<double,3,3> matNxN<double,3>::inverse(void) const {
    dvec3    c2(static_cast<dvec3>(_m[0]).crossProduct(this->_m[1]));
    eType   det = c2.dotProduct(this->_m[2]);
#ifndef NDEBUG
    if (det == 0.0)
        throw std::runtime_error("Divide by zero");
#endif
    eType   oneOverDet = 1.0 / det;
    dvec3    c0(static_cast<dvec3>(_m[1]).crossProduct(this->_m[2]));
    dvec3    c1(static_cast<dvec3>(_m[2]).crossProduct(this->_m[0]));
        
    return dmat3(c0[0] * oneOverDet, c1[0] * oneOverDet, c2[0] * oneOverDet,
                 c0[1] * oneOverDet, c1[1] * oneOverDet, c2[1] * oneOverDet,
                 c0[2] * oneOverDet, c1[2] * oneOverDet, c2[2] * oneOverDet);
}
    
class mat3x4 : public mat<float,3,4> {
public:
    // Constructors
    mat3x4(void) {}
    mat3x4(const eType x11,     const eType x12=0.0, const eType x13=0.0, const eType x14=0.0,
           const eType x21=0.0, const eType x22=0.0, const eType x23=0.0, const eType x24=0.0,
           const eType x31=0.0, const eType x32=0.0, const eType x33=0.0, const eType x34=0.0) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13; _m[0][3] = x14;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23; _m[1][3] = x24;
        _m[2][0] = x31; _m[1][1] = x32; _m[2][2] = x33; _m[2][3] = x34;
    }
    mat3x4(const vec<eType,4> &r1, const vec<eType,4> &r2, const vec<eType,4> &r3) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3;
    }
    mat3x4(const mat<eType,3,4> &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Copy Constructor
    mat3x4(const mat3x4 &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Destructor
    ~mat3x4() {}
};
    
typedef mat3x4 fmat3x4;

class dmat3x4 : public mat<double,3,4> {
public:
    // Constructors
    dmat3x4(void) {}
    dmat3x4(const eType x11,     const eType x12=0.0, const eType x13=0.0, const eType x14=0.0,
            const eType x21=0.0, const eType x22=0.0, const eType x23=0.0, const eType x24=0.0,
            const eType x31=0.0, const eType x32=0.0, const eType x33=0.0, const eType x34=0.0) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13; _m[0][3] = x14;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23; _m[1][3] = x24;
        _m[2][0] = x31; _m[1][1] = x32; _m[2][2] = x33; _m[2][3] = x34;
    }
    dmat3x4(const vec<eType,4> &r1, const vec<eType,4> &r2, const vec<eType,4> &r3) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3;
    }
    dmat3x4(const mat<eType,3,4> &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Copy Constructor
    dmat3x4(const dmat3x4 &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Destructor
    ~dmat3x4() {}
};
    
class mat4x2 : public mat<float,4,2> {
public:
    // Constructors
    mat4x2(void) {}
    mat4x2(const eType x11,      const eType x12=0.0f,
           const eType x21=0.0f, const eType x22=0.0f,
           const eType x31=0.0f, const eType x32=0.0f,
           const eType x41=0.0f, const eType x42=0.0f) {
        _m[0][0] = x11; _m[0][1] = x12;
        _m[0][2] = x21; _m[0][3] = x22;
        _m[1][0] = x31; _m[1][1] = x32;
        _m[1][2] = x41; _m[1][3] = x42;
    }
    mat4x2(const vec<eType,2> &r1, const vec<eType,2> &r2, const vec<eType,2> &r3, const vec<eType,2> &r4) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3; _m[3] = r4;
    }
    mat4x2(const mat<eType,4,2> &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Copy Constructor
    mat4x2(const mat4x2 &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Destructor
    ~mat4x2() {}
};
    
typedef mat4x2  fmat4x2;
    
class dmat4x2 : public mat<double,4,2> {
public:
    // Constructors
    dmat4x2(void) {}
    dmat4x2(const eType x11,     const eType x12=0.0,
            const eType x21=0.0, const eType x22=0.0,
            const eType x31=0.0, const eType x32=0.0,
            const eType x41=0.0, const eType x42=0.0) {
        _m[0][0] = x11; _m[0][1] = x12;
        _m[0][2] = x21; _m[0][3] = x22;
        _m[1][0] = x31; _m[1][1] = x32;
        _m[1][2] = x41; _m[1][3] = x42;
    }
    dmat4x2(const vec<eType,2> &r1, const vec<eType,2> &r2, const vec<eType,2> &r3, const vec<eType,2> &r4) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3; _m[3] = r4;
    }
    dmat4x2(const mat<eType,4,2> &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Copy Constructor
    dmat4x2(const dmat4x2 &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Destructor
    ~dmat4x2() {}
};
    
class mat4x3 : public mat<float,4,3> {
public:
    // Constructors
    mat4x3(void) {}
    mat4x3(const eType x11,      const eType x12=0.0f, const eType x13=0.0f,
           const eType x21=0.0f, const eType x22=0.0f, const eType x23=0.0f,
           const eType x31=0.0f, const eType x32=0.0f, const eType x33=0.0f,
           const eType x41=0.0f, const eType x42=0.0f, const eType x43=0.0f) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23;
        _m[2][0] = x31; _m[2][1] = x32; _m[2][2] = x33;
        _m[3][0] = x41; _m[3][1] = x42; _m[3][2] = x43;
    }
    mat4x3(const vec<eType,3> &r1, const vec<eType,3> &r2, const vec<eType,3> &r3, const vec<eType,3> &r4) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3; _m[3] = r4;
    }
    mat4x3(const mat<eType,4,3> &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Copy Constructor
    mat4x3(const mat4x3 &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Destructor
    ~mat4x3() {}
};
    
typedef mat4x3  fmat4x3;
    
class dmat4x3 : public mat<double,4,3> {
public:
    // Constructors
    dmat4x3(void) {}
    dmat4x3(const eType x11,     const eType x12=0.0, const eType x13=0.0,
            const eType x21=0.0, const eType x22=0.0, const eType x23=0.0,
            const eType x31=0.0, const eType x32=0.0, const eType x33=0.0,
            const eType x41=0.0, const eType x42=0.0, const eType x43=0.0) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23;
        _m[2][0] = x31; _m[2][1] = x32; _m[2][2] = x33;
        _m[3][0] = x41; _m[3][1] = x42; _m[3][2] = x43;
    }
    dmat4x3(const vec<eType,3> &r1, const vec<eType,3> &r2, const vec<eType,3> &r3, const vec<eType,3> &r4) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3; _m[3] = r4;
    }
    dmat4x3(const mat<eType,4,3> &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Copy Constructor
    dmat4x3(const dmat4x3 &src) {
        for (int i=0; i!=nRows(); ++i)
            _m[i] = src[i];
    }
    // Destructor
    ~dmat4x3() {}
};
    
class mat4 : public matNxN<float,4> {
public:
    // Constructors
    mat4(void) {}
    mat4(const eType x11,      const eType x12=0.0f, const eType x13=0.0f, const eType x14=0.0f,
         const eType x21=0.0f, const eType x22=1.0f, const eType x23=0.0f, const eType x24=0.0f,
         const eType x31=0.0f, const eType x32=0.0f, const eType x33=1.0f, const eType x34=0.0f,
         const eType x41=0.0f, const eType x42=0.0f, const eType x43=0.0f, const eType x44=1.0f) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13; _m[0][3] = x14;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23; _m[1][3] = x24;
        _m[2][0] = x31; _m[2][1] = x32; _m[2][2] = x33; _m[2][3] = x34;
        _m[3][0] = x41; _m[3][1] = x42; _m[3][2] = x43; _m[3][3] = x44;
    }
    mat4(const vec<eType,4> &r1, const vec<eType,4> &r2, const vec<eType,4> &r3, const vec<eType,4> &r4) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3; _m[3] = r4;
    }
    mat4(const mat<eType,4,4> &src) : matNxN<eType,4>(src) {}
    // Copy Constructor
    mat4(const mat4 &src) : matNxN<eType,4>(src) {}
    // Destructor
    ~mat4() {}
};
    
typedef mat4 mat4x4;
typedef mat4 fmat4;
typedef mat4 fmat4x4;

class dmat4 : public matNxN<double,4> {
public:
    // Constructors
    dmat4(void) {}
    dmat4(const eType x11,     const eType x12=0.0, const eType x13=0.0, const eType x14=0.0,
          const eType x21=0.0, const eType x22=1.0, const eType x23=0.0, const eType x24=0.0,
          const eType x31=0.0, const eType x32=0.0, const eType x33=1.0, const eType x34=0.0,
          const eType x41=0.0, const eType x42=0.0, const eType x43=0.0, const eType x44=1.0) {
        _m[0][0] = x11; _m[0][1] = x12; _m[0][2] = x13; _m[0][3] = x14;
        _m[1][0] = x21; _m[1][1] = x22; _m[1][2] = x23; _m[1][3] = x24;
        _m[2][0] = x31; _m[2][1] = x32; _m[2][2] = x33; _m[2][3] = x34;
        _m[3][0] = x41; _m[3][1] = x42; _m[3][2] = x43; _m[3][3] = x44;
    }
    dmat4(const vec<eType,4> &r1, const vec<eType,4> &r2, const vec<eType,4> &r3, const vec<eType,4> &r4) {
        _m[0] = r1; _m[1] = r2; _m[2] = r3; _m[3] = r4;
    }
    dmat4(const mat<eType,4,4> &src) : matNxN<eType,4>(src) {}
    // Copy Constructor
    dmat4(const dmat4 &src) : matNxN<eType,4>(src) {}
    // Destructor
    ~dmat4() {}
};
    
typedef dmat4 dmat4x4;

template <>
const float matNxN<float,4>::determinant(void) const;

template <>
const mat<float,4,4> matNxN<float,4>::adjoint(void) const;

template <>
const mat<float,4,4> matNxN<float,4>::inverse(void) const;

template <>
const double matNxN<double,4>::determinant(void) const;
    
template <>
const mat<double,4,4> matNxN<double,4>::adjoint(void) const;
    
template <>
const mat<double,4,4> matNxN<double,4>::inverse(void) const;
    
// I constructed a quaternion class to do 3D rotations.  For this reason
// the default constructor sets the quaternion to the value of the
// identity rotation (Yes, I know there are really two of these for
// quaternions.  I picked the one which is more obvious).  If this class
// were to be used for other purposes some other default value might
// make more sense.
template <typename Type>
class qTemplate {
private:
    // Used below to enable the use of named components
    typedef struct { Type r, i, j, k; } _CRL_q;
public:
    typedef Type eType;
    qTemplate(void) {}
    qTemplate(const Type s, const vec<Type,3> &u) : w(s), v(u) {}
    qTemplate(const Type s, const Type i, const Type j, const Type k) : w(s), v(i,j,k) {}
    // Will use the following later when we subclass SFRotation from
    // vec4 to convert SFRotation to a quaternion.
    explicit qTemplate(const vec<eType,4> &p) {
        w = cos(0.5*p->w);
        if (withinEpsilon(fabs(w),1.0)) {
            w = 1.0f;
            v[0] = v[1] = v[2] = 0.0;
        } else {
            v = cpvec<eType>(p[0],p[1],p[2]);
            if (w < 0.0) {
                w = -w;
                v *= sin(-0.5*p->w)/v.length();
            } else {
                v *= sin( 0.5*p->w)/v.length();
            }
        }
    }
    ~qTemplate(void) {}
    qTemplate(const qTemplate &src) : w(src.w), v(src.v) {}
    qTemplate &operator=(const qTemplate &src) {
        if (this != &src) {
            w = src.w;
            v = src.v;
        }
        return *this;
    }
public:
    Type        w;
    cpvec<Type> v;
public:
    const Type dotProduct(const qTemplate &rhs) const {
        return w * rhs.w + v.dotProduct(rhs.v);
    }
    const qTemplate operator-(void) const {
        return qTemplate(-w, -v);
    }
    qTemplate &operator+=(const qTemplate &rhs) {
        w += rhs.w;
        v += rhs.v;
        return *this;
    }
    const qTemplate operator+(const qTemplate &rhs) const {
        return qTemplate(*this) += rhs;
    }
    qTemplate &operator-=(const qTemplate &rhs) {
        w -= rhs.w;
        v -= rhs.v;
        return *this;
    }
    const qTemplate operator-(const qTemplate &rhs) const {
        return qTemplate(*this) -= rhs;
    }
    qTemplate &operator*=(const qTemplate &rhs) {
        qTemplate   tmp(*this);
        
        this->w = tmp.w*rhs.w - tmp.v.dotProduct(rhs.v);
        this->v = tmp.w*rhs.v + rhs.w*tmp.v + tmp.v.crossProduct(rhs.v);
        
        return *this;
    }
    const qTemplate operator*(const qTemplate &rhs) const {
        return qTemplate(*this) *= rhs;
    }
    qTemplate &operator*=(const Type rhs) {
        w *= rhs;
        v *= rhs;
        return *this;
    }
    const qTemplate operator*(const Type rhs) const {
        return qTemplate(*this) *= rhs;
    }
    const Type length(void) const { return sqrt(this->dotProduct(*this)); }
    const qTemplate normalize(void) const { return *this / this->length(); }
    // safeNormalize() must be redefined here since qTemplate is not a
    // subclass of the vec class template.
    float safeNormalize(const qTemplate &rhs) {
        Type   l = rhs.length();
        Type   m = (l!=0.0f) ? (1.0f/l) : 0.0f;
        *this *= m;
        return m;
    }
    float safeNormalize(const qTemplate *rhs=NULL) {
        const qTemplate  *use = rhs!=NULL ? rhs : this;
        return safeNormalize(*use);
    }
    qTemplate &loadIdentity(void) {
        w = 1.0;
        v[0] = v[1] = v[2] = 0.0;
        return *this;
    }
    const qTemplate conjugate(void) const {
        return qTemplate(this->w, -this->v);
    }
    const qTemplate inverse(void) const {
        return this->conjugate() / this->dotProduct(*this);
    }
    qTemplate &operator/=(const qTemplate &rhs) {
        return *this *= rhs.inverse();
    }
    const qTemplate operator/(const qTemplate &rhs) const{
        return qTemplate(*this) *= rhs.inverse();
    }
    qTemplate &operator/=(const Type rhs) {
#ifndef NDEBUG
    if (rhs == 0.0)
        throw std::runtime_error("Divide by zero");
#endif
        Type	oneOverRhs = 1.0 / rhs;
        w *= oneOverRhs;
        v *= oneOverRhs;
        return *this;
    }
    const qTemplate operator/(const Type rhs) const {
        return qTemplate(*this) /= rhs;
    }
    // Enable use of named components
    _CRL_q *operator->() { return reinterpret_cast<_CRL_q *>(this); }
    const _CRL_q *operator->() const { return reinterpret_cast<const _CRL_q *>(this); }
};

template <typename Type>
inline const qTemplate<Type> operator*(const Type lhs, const qTemplate<Type> &rhs) {
    return qTemplate<Type>(rhs) *= lhs;
}

template <typename Type>
std::ostream &operator<<(std::ostream &os, const qTemplate<Type> &rhs) {
    os << "( " << rhs.w << ", " << rhs.v << " )";
    return os;
}

template <typename Type>
const bool operator==(const qTemplate<Type> &lhs, const qTemplate<Type> &rhs) {
    return (lhs.w == rhs.w) && (lhs.v == rhs.v);
}

template <typename Type>
const bool operator!=(const qTemplate<Type> &lhs, const qTemplate<Type> &rhs) {
    return (lhs.w != rhs.w) || (lhs.v != rhs.v);
}
    
template <typename Type>
const bool withinEpsilon(const qTemplate<Type> &lhs, const qTemplate<Type> &rhs) {
    return withinEpsilon(lhs.w, rhs.w) && withinEpsilon(lhs.v, rhs.v);
}

class quaternion : public qTemplate<float> {
public:
    // Constructor
    quaternion(const eType s=1.0f,
               const cpvec<eType> &u=cpvec<eType>(0.0f,0.0f,0.0f)) :
        qTemplate<eType>(s,u) {}
//    quaternion(void);
    quaternion(const eType r,
               const eType i,
               const eType j,
               const eType k) : qTemplate<eType>(r,i,j,k) {}
    quaternion(const qTemplate<eType> &src) : qTemplate<eType>(src) {}
    // Will use the following later when we subclass SFRotation from
    // vec4 to convert SFRotation to a quaternion.
    explicit quaternion(const vec4 &p) {
        w = cosf(0.5*p->w);
        v = vec3(p, true);
        if (withinEpsilon(fabsf(w),1.0f)) {
            w = 1.0f;
            v[0] = v[1] = v[2] = 0.0f;
        } else if (w < 0.0f) {
            w = -w;
            v *= sinf(-0.5*p->w)/v.length();
        } else {
            v *= sinf( 0.5*p->w)/v.length();
        }
    }
    // The following is useful for taking a point in 3D and converting
    // it to a quaternion so we can apply the rotation directly using
    // quaternion multiplication.  Otherwise, we would normally convert
    // the point to a 4D vector (vec4) and apply the rotation using a
    // rotation matrix created from a quaternion.
    explicit quaternion(const cpvec<eType> &p) : qTemplate<eType>(0.0f,p) {}
    // Copy Constructor
    quaternion(const quaternion &src) : qTemplate<eType>(src) {}
    // Destructor
    ~quaternion(void) {}
};

typedef quaternion  fquaternion;

class dquaternion : public qTemplate<double> {
public:
    // Constructor
    dquaternion(const eType s=1.0f,
                const cpvec<eType> &u=cpvec<eType>(0.0f,0.0f,0.0f)) :
    qTemplate<eType>(s,u) {}
//    quaternion(void);
    dquaternion(const eType s,
                         const eType i,
                         const eType j,
                const eType k) : qTemplate<eType>(s,i,j,k) {}
    // The following is useful for taking a point in 3D and converting
    // it to a quaternion so we can apply the rotation directly using
    // quaternion multiplication.  Otherwise, we would normally convert
    // the point to a 4D vector (vec4) and apply the rotation using a
    // rotation matrix created from a quaternion.
    explicit dquaternion(const cpvec<eType> &p) : qTemplate<eType>(0.0,p) {}
    // Copy Constructor
    dquaternion(const dquaternion &src) : qTemplate<eType>(src) {}
    // Destructor
    ~dquaternion(void) {}
};
    
// SLERP -- Spherical Linear intERPolation function
//
// We can write these functions as templates.  We'll actually invoke slerp()
// using two of our self defined data types (vec3 and quaternion) but
// it may be useful for other things in other contexts with the vec2 and
// vec4 data types.  Note that whatever data type is used must have defined
// the class/friend methods:
// o friend const Type operator*(const float, const Type &),
// o const Type Type::operator+(const Type &) const,
// o const Type Type::operator/(const float) const,
// o const Type Type::operator-(const Type &) const,
// o const float Type::length(void) const, and
// o const float Type::dotProduct(const Type &) const.
//
// We'll use these function templates with vec3 to implement the
// NormalInterpolator type and with quaternion to implement the
// OrientationInterpolator type.  In both cases slerp() assumes that the
// n-tuple (i.e. vec3 and quaternion) data are normalized, i.e., have
// length 1.  Also, we expect that 0.0 <= alpha <= 1.0 but we don't
// enforce this.
//
// When using SLERP with quaternions we leave it to the higher level
// logic using these functions to determine if we should interpolate
// between q1 & q2 or between q1 & -q2.  [Note:  See Buss, p. 307.]

// This version of the function assumes that when we set up the
// NormalInterpolator node or the OrientationInterpolator node that we
// precomputed an array of the phi and sin(phi) values using the node's
// keyValue fields to improve performance.  This version of the function
// is also used by the normal user interface to the SLERP function which
// takes two n-tuples and an alpha value to do the actual work so we
// don't needlessly duplicate code.
template <typename vType, typename eType>
inline vType slerp(const vType &q1, const vType &q2, const eType alpha,
		   const eType phi, const eType sinPhi)
{
    // Use epsilon to determine if the angle between q1 & q2 is so small
    // that it might cause numerical instabilities or even divide by zero.
    // If so, we'll do linear interpolation between q1 and q2 rather than
    // SLERP.  The value of epsilon chosen is per Buss, p. 125; if we are
    // using double precision the value would be 1.0E-09, again, per Buss.
    static const eType	epsilon = (sizeof(eType) > sizeof(float)) ?
							  1.0E-09 :
							  1.0E-04;
    static const eType  one = 1.0;

    if (phi < epsilon) {
        return (one - alpha)*q1 + alpha*q2;
    }
    
    return (static_cast<eType>(sin((one - alpha)*phi))*q1 + static_cast<eType>(sin(alpha*phi))*q2) / sinPhi;
}

// Normal user interface to SLERP function:
template <typename vType, typename eType>
inline vType slerp(const vType &x, const vType &y, const eType alpha)
{
    // Our implementation of Buss's second version of
    // Precompute_for_Slerp logic.
    eType	c = x.dotProduct(y);    // cos(phi)

    vType   tmpY;
    if (c < 0.0) {
        tmpY = -y;
        c    = -c;
    } else {
        tmpY = y;
    }

    vType	v = tmpY - c*x;
    eType	s = v.length();         // sin(phi)
    eType	phi = atan2(s,c);
    
    return slerp(x, tmpY, alpha, phi, s);
}

template <typename vType, typename eType>
inline vType linterp(const vType &x, const vType &y, const eType alpha)
{
    static const eType  one = 1.0;

    return (one - alpha)*x + alpha*y;
}

// Rotation linear interpolation, sort of.
// This is a hack to deal with fact that are times one wants to interpolate
// quaternions which represent rotations.  This operations breaks if one
// uses real linear interpolation on quaternions.  Rather than bastardize
// the linear interpolation template above to deal with the special case of
// rotations I've created a special function template to deal with the case.
template <typename Type>
inline quaternion rinterp(const qTemplate<Type> &x, const qTemplate<Type> &y, const Type alpha)
{
    static const Type  one = 1.0;

    qTemplate<Type>  tmp;
    if (x.dotProduct(y) < 0.0) {
        tmp = -y;
    } else {
        tmp = y;
    }

    return (one - alpha)*x + alpha*tmp;
}

#endif /* defined(__glml__glml__) */

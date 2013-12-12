/*

GFX Lightweight OpenGLES 2.0 Game and Graphics Engine

Copyright (C) 2011 Romain Marucchi-Foino http://gfx.sio2interactive.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an acknowledgment
in the product would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#ifndef TYPES_H
#define TYPES_H

#define MAX_CHAR	64

#define MAX_PATH	256

#define DEG_TO_RAD	(M_PI / 180.0f)

#define RAD_TO_DEG	(180.0f / M_PI)

#define BUFFER_OFFSET( x ) ( ( char * )NULL + x )

#define CLAMP( x, low, high ) ( ( x > high ) ? high : ( ( x < low ) ? low : x ) )

#include <iostream>

enum AnimState {
    STOP  = 0,
    PLAY  = 1,
    PAUSE = 2
};

inline bool withinEpsilon(const float lhs, const float rhs)
{
    float   epsilon = 1.0e-5;
    float   absl = fabsf(lhs),
            absr = fabsf(rhs);
    float   max = (absl > absr) ? absl : absr;

    if (max)
        return (fabsf(absl-absr)/max) < epsilon;

    return true;
}

struct vec2 {
private:
    typedef struct { float x, y; } _CRL_vf2;
    float   _v[2];
public:
    int nElems() const { return sizeof(_v)/sizeof(_v[0]); }
    vec2() {}
    vec2(const float x, const float y) {
        _v[0] = x;
        _v[1] = y;
    }
    vec2(const vec2 &rhs) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] = rhs._v[i];
    }
    vec2 &operator=(const vec2 &rhs) {
        if (this != &rhs) {
            for (int i=0; i!=nElems(); ++i)
                _v[i] = rhs._v[i];
        }
        return *this;
    }
    const float operator[](const int i) const {
        assert(0<=i && i<nElems());
        return _v[i];
    }
    float &operator[](const int i) {
        assert(0<=i && i<nElems());
        return _v[i];
    }
    _CRL_vf2 *operator->() { return (_CRL_vf2 *)this; }
    const _CRL_vf2 *operator->() const { return (const _CRL_vf2 *)this; }
    vec2 &operator+=(const vec2 &rhs) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] += rhs._v[i];
        return *this;
    }
    vec2 &operator+(const vec2 &rhs) const {
        return vec2(*this) += rhs;
    }
    vec2 &operator-=(const vec2 &rhs) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] -= rhs._v[i];
        return *this;
    }
    vec2 &operator-(const vec2 &rhs) const {
        return vec2(*this) -= rhs;
    }
    vec2 &operator*=(const float rhs) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] *= rhs;
        return *this;
    }
    const vec2 operator*(const float rhs) const {
        return vec2(*this) *= rhs;
    }
    bool operator==(const vec2 &rhs) const {
        for (int i=0; i!=nElems(); ++i)
            if (_v[i] != rhs._v[i])
                return false;
        return true;
    }
    bool operator!=(const vec2 &rhs) const {
        for (int i=0; i!=nElems(); ++i)
            if (_v[i] != rhs._v[i])
                return true;
        return false;
    }
};

inline vec2 operator*(const float lhs, const vec2 &rhs) {
    return rhs * lhs;
}
    
inline const vec2 linterp(const vec2 &v0, const vec2 &v1, float t) {
    if (t==0) {
        return v0;
    } else if (t==1) {
        return v1;
    }

    return v0 * (1.0f - t) + v1 * t;
}

inline std::ostream &operator<<(std::ostream &os, const vec2 &rhs)
{
    os << "( " << rhs[0];
    for (int i=1; i!=2; ++i)
        os << ", " << rhs[i];
    os << " )";
    return os;
}

struct vec4;

struct vec3 {
	float x;
	float y;
	float z;

    vec3() {}
    vec3(const float x, const float y, const float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    vec3(const vec3 &rhs) {
        this->x = rhs.x;
        this->y = rhs.y;
        this->z = rhs.z;
    }
    vec3(const vec4 &rhs, const bool truncate=false);
    vec3 &operator=(const vec3 &rhs) {
        if (this != &rhs) {
            this->x = rhs.x;
            this->y = rhs.y;
            this->z = rhs.z;
        }
        return *this;
    }
    const float operator[](const int i) const {
        switch (i) {
            case 0:
                return x;
                break;
            case 1:
                return y;
                break;
        }
        return z;
    }
    float &operator[](const int i) {
        switch (i) {
            case 0:
                return x;
                break;
            case 1:
                return y;
                break;
        }
        return z;
    }
    vec3 &operator+=(const vec3 &rhs) {
        this->x += rhs.x;
        this->y += rhs.y;
        this->z += rhs.z;
        return *this;
    }
    vec3 &operator+(const vec3 &rhs) const {
        return vec3(*this) += rhs;
    }
    vec3 &operator-=(const vec3 &rhs) {
        this->x -= rhs.x;
        this->y -= rhs.y;
        this->z -= rhs.z;
        return *this;
    }
    vec3 &operator-(const vec3 &rhs) const {
        return vec3(*this) -= rhs;
    }
    vec3 &operator*=(const float rhs) {
        this->x *= rhs;
        this->y *= rhs;
        this->z *= rhs;
        return *this;
    }
    const vec3 operator*(const float rhs) const {
        return vec3(*this) *= rhs;
    }
    vec3 &operator/=(const float rhs) {
#ifndef NDEBUG
        if (rhs == 0.0)
            exit(1);
#endif
        float    oneOverRhs = 1.0 / rhs;
        this->x *= oneOverRhs;
        this->y *= oneOverRhs;
        this->z *= oneOverRhs;
        return *this;
    }
    const vec3 operator/(const float rhs) const {
        return vec3(*this) /= rhs;
    }
    const vec3 crossProduct(const vec3 &rhs) const {
        return vec3(this->y*rhs.z - this->z*rhs.y,
                    this->z*rhs.x - this->x*rhs.z,
                    this->x*rhs.y - this->y*rhs.x);
    }
    bool operator==(const vec3 &rhs) const {
        return (this->x==rhs.x) &&
               (this->y==rhs.y) &&
               (this->z==rhs.z);
    }
    bool operator!=(const vec3 &rhs) const {
        return (this->x!=rhs.x) ||
               (this->y!=rhs.y) ||
               (this->z!=rhs.z);
    }
    const float dotProduct(const vec3 &rhs) const {
        return this->x*rhs.x + this->y*rhs.y + this->z*rhs.z;
    }
    const float length(void) const {
        return sqrt(this->dotProduct(*this));
    }
    const vec3 normalize(void) const {
        return *this / length();
    }
    float safeNormalize(const vec3 &rhs) {
        float   l = rhs.length();
        float   m = (l!=0.0f) ? (1.0f/l) : 0.0f;
        *this *= m;
        return m;
    }
    float safeNormalize(const vec3 *rhs=NULL) {
        const vec3  *use = rhs!=NULL ? rhs : this;
        float   l = use->length();
        float   m = (l!=0.0f) ? (1.0f/l) : 0.0f;
        *this *= m;
        return m;
    }
    const vec3 operator-(void) const {
        return vec3(-this->x, -this->y, -this->z);
    }
};

inline vec3 operator*(const float lhs, const vec3 &rhs) {
    return rhs * lhs;
}

inline const vec3 linterp(const vec3 &v0, const vec3 &v1, float t) {
    if (t==0) {
        return v0;
    } else if (t==1) {
        return v1;
    }

    return v0 * (1.0f - t) + v1 * t;
}

inline bool withinEpsilon(const vec3 &lhs, const vec3 &rhs) {
    float   epsilon = 1.0e-5;
    float   ll = lhs.length(),
            rl = rhs.length();

    if (!withinEpsilon(ll, rl))
        return false;

    float   max = (ll > rl) ? ll : rl;

    if (max)
        return (fabsf(ll-rl)/max) < epsilon;

    return true;
}

inline std::ostream &operator<<(std::ostream &os, const vec3 &rhs)
{
    os << "( " << rhs[0];
    for (int i=1; i!=3; ++i)
        os << ", " << rhs[i];
    os << " )";
    return os;
}
    
struct vec4 {
private:
    typedef struct { float x, y, z, w; } _CRL_vf4;
    float   _v[4];
public:
    int nElems() const { return sizeof(_v)/sizeof(_v[0]); }

    vec4() {}
    vec4(const float x, const float y, const float z, const float w) {
        _v[0] = x;
        _v[1] = y;
        _v[2] = z;
        _v[3] = w;
    }
    explicit vec4(const vec3 &argXYZ, const float argW=1.0) {
        _v[0] = argXYZ.x;
        _v[1] = argXYZ.y;
        _v[2] = argXYZ.z;
        _v[3] = argW;
    }
    vec4(const vec4 &rhs) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] = rhs._v[i];
    }
    vec4 &operator=(const vec4 &rhs) {
        if (this != &rhs) {
            for (int i=0; i!=nElems(); ++i)
                _v[i] = rhs._v[i];
        }
        return *this;
    }
    const float operator[](const int i) const {
        assert(0<=i && i<nElems());
        return _v[i];
    }
    float &operator[](const int i) {
        assert(0<=i && i<nElems());
        return _v[i];
    }
    _CRL_vf4 *operator->() { return (_CRL_vf4 *)this; }
    const _CRL_vf4 *operator->() const { return (const _CRL_vf4 *)this; }
    vec4 &operator+=(const vec4 &rhs) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] += rhs._v[i];
        return *this;
    }
    vec4 &operator+(const vec4 &rhs) const {
        return vec4(*this) += rhs;
    }
    vec4 &operator-=(const vec4 &rhs) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] -= rhs._v[i];
        return *this;
    }
    vec4 &operator-(const vec4 &rhs) {
        return vec4(*this) -= rhs;
    }
    vec4 &operator*=(const float rhs) {
        for (int i=0; i!=nElems(); ++i)
            _v[i] *= rhs;
        return *this;
    }
    const vec4 operator*(const float rhs) const {
        return vec4(*this) *= rhs;
    }
    vec4 &operator/=(const float rhs) {
#ifndef NDEBUG
        if (rhs == 0.0)
            exit(1);
#endif
        float    oneOverRhs = 1.0 / rhs;
        for (int i=0; i!=nElems(); ++i)
            _v[i] *= oneOverRhs;
        return *this;
    }
    const vec4 operator/(const float rhs) const {
        return vec4(*this) /= rhs;
    }
    const float dotProduct(const vec4 &rhs) const {
        float tmp = _v[0] * rhs._v[0];
        for (int i=1; i!=nElems(); ++i)
            tmp += _v[i] * rhs._v[i];
        return tmp;
    }
    const float length(void) const {
        return sqrt(this->dotProduct(*this));
    }
    const vec4 normalize(void) const {
        return *this / length();
    }
    float safeNormalize(const vec4 &rhs) {
        float   l = rhs.length();
        float   m = (l!=0.0f) ? (1.0f/l) : 0.0f;
        *this *= m;
        return m;
    }
    float safeNormalize(const vec4 *rhs=NULL) {
        const vec4  *use = rhs!=NULL ? rhs : this;
        float   l = use->length();
        float   m = (l!=0.0f) ? (1.0f/l) : 0.0f;
        *this *= m;
        return m;
    }
    const vec4 operator-(void) const {
        return vec4(-_v[0], -_v[1], -_v[2], -_v[3]);
    }
    bool operator==(const vec4 &rhs) const {
        for (int i=0; i!=nElems(); ++i)
            if (_v[i] != rhs._v[i])
                return false;
        return true;
    }
    bool operator!=(const vec4 &rhs) const {
        for (int i=0; i!=nElems(); ++i)
            if (_v[i] != rhs._v[i])
                return true;
        return false;
    }
};

inline vec3::vec3(const vec4 &argXYZW, const bool truncate) {
    this->x = argXYZW[0];
    this->y = argXYZW[1];
    this->z = argXYZW[2];
    if (!truncate && argXYZW[3]!=0.0f && argXYZW[3]!=1.0f) {
        this->x /= argXYZW[3];
        this->y /= argXYZW[3];
        this->z /= argXYZW[3];
    }
}

inline vec4 operator*(float lhs, vec4 &rhs) {
    return rhs * lhs;
}

inline const vec4 linterp(const vec4 &v0, const vec4 &v1, float t) {
    if (t==0) {
        return v0;
    } else if (t==1) {
        return v1;
    }

    return v0 * (1.0f - t) + v1 * t;
}
    

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;	

} ucol4;


struct quaternion {
private:
    typedef struct { float r, i, j, k; } _CRL_q;
public:
    float   w;
    vec3    v;

    quaternion() {}
    quaternion(const float argW, const float argX, const float argY, const float argZ) {
        w    = argW;
        v[0] = argX;
        v[1] = argY;
        v[2] = argZ;
    }
    explicit quaternion(const float argW, const vec3 &argXYZ) {
        w = argW;
        v = argXYZ;
    }
    quaternion(const quaternion &rhs) {
        w = rhs.w;
        v = rhs.v;
    }
    quaternion &operator=(const quaternion &rhs) {
        if (this != &rhs) {
            w = rhs.w;
            v = rhs.v;
        }
        return *this;
    }
    _CRL_q *operator->() { return (_CRL_q *)this; }
    const _CRL_q *operator->() const { return (const _CRL_q *)this; }
    quaternion &operator+=(const quaternion &rhs) {
        w += rhs.w;
        v += rhs.v;
        return *this;
    }
    quaternion &operator+(const quaternion &rhs) const {
        return quaternion(*this) += rhs;
    }
    quaternion &operator-=(const quaternion &rhs) {
        w -= rhs.w;
        v -= rhs.v;
        return *this;
    }
    quaternion &operator-(const quaternion &rhs) {
        return quaternion(*this) -= rhs;
    }
    quaternion &operator*=(const quaternion &rhs) {
        quaternion   tmp(*this);

        this->w = tmp.w*rhs.w - tmp.v.dotProduct(rhs.v);
        this->v = tmp.w*rhs.v + rhs.w*tmp.v + tmp.v.crossProduct(rhs.v);

        return *this;
    }
    const quaternion operator*(const quaternion &rhs) const {
        return quaternion(*this) *= rhs;
    }
    quaternion &operator*=(const float rhs) {
        w *= rhs;
        v *= rhs;
        return *this;
    }
    const quaternion operator*(const float rhs) const {
        return quaternion(*this) *= rhs;
    }
    quaternion &operator/=(const float rhs) {
#ifndef NDEBUG
        if (rhs == 0.0)
            exit(1);
#endif
        float    oneOverRhs = 1.0 / rhs;
        w *= oneOverRhs;
        v *= oneOverRhs;
        return *this;
    }
    const quaternion operator/(const float rhs) const {
        return quaternion(*this) /= rhs;
    }
    const float dotProduct(const quaternion &rhs) const {
        return w*rhs.w + v.dotProduct(rhs.v);
    }
    const float length(void) const {
        return sqrt(this->dotProduct(*this));
    }
    const quaternion normalize(void) const {
        return *this / length();
    }
    float safeNormalize(const quaternion &rhs) {
        float   l = rhs.length();
        float   m = (l!=0.0f) ? (1.0f/l) : 0.0f;
        *this *= m;
        return m;
    }
    float safeNormalize(const quaternion *rhs=NULL) {
        const quaternion  *use = rhs!=NULL ? rhs : this;
        float   l = use->length();
        float   m = (l!=0.0f) ? (1.0f/l) : 0.0f;
        *this *= m;
        return m;
    }
    const quaternion operator-(void) const {
        return quaternion(-w, -v);
    }
    const quaternion conjugate() const {
        return quaternion(w, -v);
    }
    bool operator==(const quaternion &rhs) const {
        return (this->w==rhs.w) &&
               (this->v==rhs.v);
    }
    bool operator!=(const quaternion &rhs) const {
        return (this->w!=rhs.w) ||
               (this->v!=rhs.v);
    }
};


struct mat3 {
	vec3 m[3];

    mat3() {}
};


typedef struct
{
	vec4 m[4];

} mat4;


typedef struct
{
	unsigned short vertex_index;
	
	unsigned short uv_index;

} vecind;


#endif

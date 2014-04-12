//
//  tstack.cpp
//  glml
//
//  Created by Chris Larsen on 8/1/11.
//  Copyright (c) 2011-2014 Chris Larsen. All rights reserved.
//

#include "tstack.h"

TStack &TStack::push()
{
    if (mStack.size()) {
        mStack.resize(mStack.size()+1);
        *(mStack.end()-1) = *(mStack.end()-2);
    } else {
        // The stack is empty.  Push on the identity matrix.
        mStack.push_back(mat4(1));
    }

    return *this;
}

TStack &TStack::pop()
{
    if (mStack.size() == 1)
        throw stack_bottom("Already at bottom of transformation stack");

    mStack.pop_back();

    return *this;
}

// Create rotation matrix from a quaternion
static void rotationMatrix(mat4 &m, const quaternion &q)
{
    m[0][0] = 1.0 - 2.0 * (q->j*q->j + q->k*q->k);
    m[1][0] = 2.0 * (q->i*q->j - q->k*q->r);
    m[2][0] = 2.0 * (q->k*q->i + q->j*q->r);
    m[3][0] = 0.0;

    m[0][1] = 2.0 * (q->i*q->j + q->k*q->r);
    m[1][1] = 1.0 - 2.0 * (q->k*q->k + q->i*q->i);
    m[2][1] = 2.0 * (q->j*q->k - q->i*q->r);
    m[3][1] = 0.0;

    m[0][2] = 2.0 * (q->k*q->i - q->j*q->r);
    m[1][2] = 2.0 * (q->j*q->k + q->i*q->r);
    m[2][2] = 1.0 - 2.0 * (q->j*q->j + q->i*q->i);
    m[3][2] = 0.0;

    m[0][3] = 0.0;
    m[1][3] = 0.0;
    m[2][3] = 0.0;
    m[3][3] = 1.0;
}

TStack &TStack::loadRotation(const quaternion &q)
{
    rotationMatrix(mStack.back(), q);

    return *this;
}

static void rotationMatrix(mat4 &m, const float theta, const vec3 &v)
{
    vec3    n = v.normalize();
    float   sinTheta = sinf(theta);
    float   cosTheta = cosf(theta);
    float   a = 1.0 - cosTheta;
    float   ax = a * n->x;
    float   ay = a * n->y;
    float   az = a * n->z;
    float   axx = ax * n->x;
    float   ayy = ay * n->y;
    float   azz = az * n->z;
    float   axy = ax * n->y;    // aka ayx
    float   axz = ax * n->z;    // aka azx
    float   ayz = ay * n->z;    // aka azy
    float   xSinTheta = n->x * sinTheta;
    float   ySinTheta = n->y * sinTheta;
    float   zSinTheta = n->z * sinTheta;

    m[0][0] = axx + cosTheta;  m[0][1] = axy + zSinTheta; m[0][2] = axz - ySinTheta; m[0][3] = 0.0,
    m[1][0] = axy - zSinTheta; m[1][1] = ayy + cosTheta;  m[1][2] = ayz + xSinTheta; m[1][3] = 0.0,
    m[2][0] = axz + ySinTheta; m[2][1] = ayz - xSinTheta; m[2][2] = azz + cosTheta;  m[2][3] = 0.0,
    m[3][0] = 0.0;             m[3][1] = 0.0;             m[3][2] = 0.0;             m[3][3] = 1.0;
}

TStack &TStack::loadRotation(const float degrees, const vec3 &v)
{
    rotationMatrix(mStack.back(), DegreesToRadians(degrees), v);

    return *this;
}

static void rotationMatrix(mat4 &m, const vec4 &r)
{
    vec3    v(r->x, r->y, r->z);
    float   theta = r->w;

    rotationMatrix(m, theta, v);
}

TStack &TStack::loadRotation(const vec4 &v)
{
    rotationMatrix(mStack.back(), v);

    return *this;
}

static void rotateMultiply(mat4 &m, const mat4 &r)
{
    mat3x4  a(m[0], m[1], m[2]);

    for (int i=0; i!=a.nRows(); ++i) {
        for (int j=0; j!=a.nCols(); ++j) {
            m[i][j] = r[i][0] * a[0][j];
            for (int k=1; k!=a.nRows(); ++k)
                m[i][j] += r[i][k] * a[k][j];
        }
    }
}

TStack &TStack::rotate(const quaternion &q)
{
    mat4    r;

    rotationMatrix(r, q);

    rotateMultiply(mStack.back(), r);

    return *this;
}

TStack &TStack::rotate(const float degrees, const vec3 &v)
{
    mat4    r;

    rotationMatrix(r, DegreesToRadians(degrees), v);

    rotateMultiply(mStack.back(), r);

    return *this;
}

TStack &TStack::rotate(const vec4 &v)
{
    mat4    r;

    rotationMatrix(r, v);

    rotateMultiply(mStack.back(), r);

    return *this;
}

TStack &TStack::loadScaleFactors(const vec3 &s)
{
    mat4    &M = mStack.back();

    for (int i=0; i!=s.nElems(); ++i) {
        int j=0;
        for ( ; j!=i; ++j) {
            M[i][j] = 0.0;
        }
        M[i][j] = s[i];
        for (++j; j!=M.nCols(); ++j) {
            M[i][j] = 0.0;
        }
    }
    M[3][0] = M[3][1] = M[3][2] = 0.0;
    M[3][3] = 1.0;

    return *this;
}

TStack &TStack::loadScaleFactors(const float sx, const float sy, const float sz)
{
    mat4    &M = mStack.back();

    M[0][0] = sx;   M[0][1] = 0.0f; M[0][2] = 0.0f; M[0][3] = 0.0f;
    M[1][0] = 0.0f; M[1][1] = sy;   M[1][2] = 0.0f; M[1][3] = 0.0f;
    M[2][0] = 0.0f; M[2][1] = 0.0f; M[2][2] = sz;   M[2][3] = 0.0f;
    M[3][0] = 0.0f; M[3][1] = 0.0f; M[3][2] = 0.0f; M[3][3] = 1.0f;

    return *this;
}

TStack &TStack::scale(const vec3 &s)
{
    for (int i=0; i!=s.nElems(); ++i)
        mStack.back()[i] *= s[i];

    return *this;
}

TStack &TStack::scale(const float sx, const float sy, const float sz)
{
    mStack.back()[0] *= sx;
    mStack.back()[1] *= sy;
    mStack.back()[2] *= sz;

    return *this;
}

TStack &TStack::loadTranslation(const vec3 &t)
{
    mat4    &M = mStack.back();

    for (int i=0; i!=M.nRows()-1; ++i) {
        int j=0;
        for (; j!=i; ++j) {
            M[i][j] = 0.0;
        }
        M[i][j] = 1.0;
        for (++j ; j!=M.nCols(); ++j) {
            M[i][j];
        }
    }
    M[3] = vec4(t, 1.0);

    return *this;
}

TStack &TStack::loadTranslation(const float tx, const float ty, const float tz)
{
    mat4    &M = mStack.back();

    M[0][0] = 1.0f; M[0][1] = 0.0f; M[0][2] = 0.0f; M[0][3] = 0.0f;
    M[1][0] = 0.0f; M[1][1] = 1.0f; M[1][2] = 0.0f; M[1][3] = 0.0f;
    M[2][0] = 0.0f; M[2][1] = 0.0f; M[2][2] = 1.0f; M[2][3] = 0.0f;
    M[3][0] = tx;   M[3][1] = ty;   M[3][2] = tz;   M[3][3] = 1.0f;

    return *this;
}

TStack &TStack::translate(const vec3 &t)
{
    mat4    &M = mStack.back();

    for (int j=0; j<4; j++)
        for (int i=0; i<3; i++)
            M[3][j] += t[i] * M[i][j];

    return *this;
}

TStack &TStack::translate(const float tx, const float ty, const float tz)
{
    mat4    &M = mStack.back();

    for (int j=0; j<4; j++) {
        M[3][j] += tx * M[0][j];
        M[3][j] += ty * M[1][j];
        M[3][j] += tz * M[2][j];
    }

    return *this;
}

TStack &TStack::loadMatrix(const mat4 &a)
{
    mStack.back() = a;

    return *this;
}

TStack &TStack::multiplyMatrix(const mat4 &a)
{
    mStack.back() = a * mStack.back();

    return *this;
}

static void lookAtMatrix(mat4 &m, const vec3 &eye, const vec3 &center, const vec3 &up)
{
    vec3    f = (eye-center).normalize();       // This vector will get rotated to the Z-axis
    vec3    s = up.crossProduct(f).normalize(); // This vector will get rotated to the X-axis
    vec3    u = f.crossProduct(s);              // This vector will get rotated to the Y-axis
    vec3    t(-eye.dotProduct(s),
              -eye.dotProduct(u),
              -eye.dotProduct(f));

    m[0][0] = s[0]; m[0][1] = u[0]; m[0][2] = f[0]; m[0][3] = 0.0f;
    m[1][0] = s[1]; m[1][1] = u[1]; m[1][2] = f[1]; m[1][3] = 0.0f;
    m[2][0] = s[2]; m[2][1] = u[2]; m[2][2] = f[2]; m[2][3] = 0.0f;
    m[3][0] = t[0]; m[3][1] = t[1]; m[3][2] = t[2]; m[3][3] = 1.0f;
}

// Apply a model transformation using gluLookAt()'s method
TStack &TStack::lookAt(const vec3 &eye, const vec3 &center, const vec3 &up)
{
    mat4    &M = mStack.back();
    mat4    m;
    mat3x4  tmp(M[0], M[1], M[2]);

    lookAtMatrix(m, eye, center, up);

    for (int i=0; i!=3; ++i) {
        M[i] =  m[i][0]*tmp[0] + m[i][1]*tmp[1] + m[i][2]*tmp[2];
        M[3] += m[3][i]*tmp[i];
    }
    
    return *this;
}

TStack &TStack::loadLookAt(const vec3 &eye, const vec3 &center, const vec3 &up)
{
    lookAtMatrix(mStack.back(), eye, center, up);

    return *this;
}

static void frustumMatrix(mat4 &m,
                          const float l, const float r,
                          const float b, const float t,
                          const float n, const float f)
{
    float   rml(r-l);
    float   tmb(t-b);
    float   fmn(f-n);
    m[0][0] = 2*n/rml;   m[0][1] = 0.0f;      m[0][2] = 0.0f;       m[0][3] =  0.0f;
    m[1][0] = 0.0f;      m[1][1] = 2*n/tmb;   m[1][2] = 0.0f;       m[1][3] =  0.0f;
    m[2][0] = (r+l)/rml; m[2][1] = (t+b)/tmb; m[2][2] = -(f+n)/fmn; m[2][3] = -1.0f;
    m[3][0] = 0.0f;      m[3][1] = 0.0f;      m[3][2] = -2*f*n/fmn; m[3][3] =  0.0f;
}

// Apply a projection transformation using glFrustum()'s method
TStack &TStack::frustum(const float l, const float r,
                        const float b, const float t,
                        const float n, const float f)
{
    mat4    &M = mStack.back();
    mat4    m;
    vec4    tmp(M[2]);

    frustumMatrix(m, l, r, b, t, n, f);

    M[2] =  m[2][0]*M[0] +
            m[2][1]*M[1] +
            m[2][2]*M[2] -
                    M[3];
    M[0] *= m[0][0];
    M[1] *= m[1][1];
    M[3] =  m[3][2]*tmp;

    return *this;
}

TStack &TStack::loadFrustum(const float l, const float r,
                            const float b, const float t,
                            const float n, const float f)
{
    frustumMatrix(mStack.back(), l, r, b, t, n, f);

    return *this;
}

static void orthoMatrix(mat4 &m,
                        const float l, const float r,
                        const float b, const float t,
                        const float n, const float f)
{
    float   rml(r-l);
    float   tmb(t-b);
    float   fmn(f-n);
    m[0][0] = 2.0f/rml;   m[0][1] = 0.0f;       m[0][2] = 0.0f;       m[0][3] = 0.0f;
    m[1][0] = 0.0f;       m[1][1] = 2.0f/tmb;   m[1][2] = 0.0f;       m[1][3] = 0.0f;
    m[2][0] = 0.0f;       m[2][1] = 0.0f;       m[2][2] = -2.0f/fmn;  m[2][3] = 0.0f;
    m[3][0] = -(r+l)/rml; m[3][1] = -(t+b)/tmb; m[3][2] = -(f+n)/fmn; m[3][3] = 1.0f;
}

// Apply a projection transformation using glOrtho()'s method
TStack &TStack::ortho(const float l, const float r,
                      const float b, const float t,
                      const float n, const float f)
{
    mat4    &M = mStack.back();
    mat4    m;

    orthoMatrix(m, l, r, b, t, n, f);

    M[3] += m[3][0]*M[0] +
            m[3][1]*M[1] +
            m[3][2]*M[2];
    M[0] *= m[0][0];
    M[1] *= m[1][1];
    M[2] *= m[2][2];

    return *this;
}

TStack &TStack::loadOrtho(const float l, const float r,
                          const float b, const float t,
                          const float n, const float f)
{
    orthoMatrix(mStack.back(), l, r, b, t, n, f);

    return *this;
}

static void perspectiveMatrix(mat4 &m,
                              const float fovy, const float aspect,
                              const float near, const float far)
{
//    float   f = 1 / tanf(DegreesToRadians(fovy)/2.0);   // cot(fovy/2)
    float   f = tanf(M_PI_2 - DegreesToRadians(fovy*0.5));  // cot(fovy/2)

    m[0][0] = f/aspect; m[0][1] = 0.0f; m[0][2] = 0.0f;                     m[0][3] =  0.0f;
    m[1][0] = 0.0f;     m[1][1] = f;    m[1][2] = 0.0f;                     m[1][3] =  0.0f;
    m[2][0] = 0.0f;     m[2][1] = 0.0f; m[2][2] = (far+near)/(near-far);    m[2][3] = -1.0f;
    m[3][0] = 0.0f;     m[3][1] = 0.0f; m[3][2] = 2.0f*far*near/(near-far); m[3][3] =  0.0f;
}

// Apply a projection transformation using gluPerspective()'s method
TStack &TStack::perspective(const float fovy, const float aspect,
                            const float near, const float far)
{
    mat4    &M = mStack.back();
    mat4    m;
    vec4    tmp(M[2]);

    perspectiveMatrix(m, fovy, aspect, near, far);

    M[0] *= m[0][0];
    M[1] *= m[1][1];
    M[2] =  m[2][2]*tmp - M[3];
    M[3] =  m[3][2]*tmp;

    return *this;
}

TStack &TStack::loadPerspective(const float fovy, const float aspect,
                                const float near, const float far)
{
    perspectiveMatrix(mStack.back(), fovy, aspect, near, far);

    return *this;
}

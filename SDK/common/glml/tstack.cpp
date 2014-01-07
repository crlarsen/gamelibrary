//
//  tstack.cpp
//  chapter4-5
//
//  Created by Chris Larsen on 8/1/11.
//  Copyright (c) 2014 Chris Larsen. All rights reserved.
//

#include "tstack.h"

void TStack::push()
{
    if (mStack.size()) {
        mat4    a(mStack.back());

        // Can't use "mStack.push_back(mStack.back());" because push_back() might
        // need to reallocate space and I don't know if the push_back()
        // with try to use the old or new location of mStack.back().
        mStack.push_back(a);
    } else {
        mStack.push_back(mat4(1));
    }
}

void TStack::pop()
{
    if (mStack.size() == 1)
        throw stack_bottom("Already at bottom of transformation stack");

    mStack.pop_back();
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

static void rotationMatrix(mat4 &m, const float thetaInDegrees, const vec3 &v)
{
    // Convert theta to radians and divide by 2.
    float   alpha = thetaInDegrees * M_PI / 360.0;
    float   c = cosf(alpha);
    float   s = sinf(alpha);
    vec3    n(v);
    if (n) n *= s/n.length();
    quaternion  q(c, n);

    rotationMatrix(m, q);
}

TStack &TStack::loadRotation(const float degrees, const vec3 &v)
{
    rotationMatrix(mStack.back(), degrees, v);

    return *this;
}

static void rotationMatrix(mat4 &m, const vec4 &v)
{
    float   alpha = v->w * 0.5;
    float   c = cosf(alpha);
    float   s = sinf(alpha);
    vec3    n(v->x, v->y, v->z);
    if (n) n *= s/n.length();
    quaternion  q(c, n);

    rotationMatrix(m, q);
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

    rotationMatrix(r, degrees, v);

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

TStack &TStack::scale(const vec3 &s)
{
    for (int i=0; i!=s.nElems(); ++i)
        mStack.back()[i] *= s[i];

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

TStack &TStack::translate(const vec3 &t)
{
    mat4    &M = mStack.back();

    for (int j=0; j<4; j++)
        for (int i=0; i<3; i++)
            M[3][j] += t[i] * M[i][j];

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

// Apply a projection transformation using glFrustum()'s method
TStack &TStack::frustum(const float l, const float r,
                        const float b, const float t,
                        const float n, const float f)
{
    mat4    &M = mStack.back();
    vec4    tmp(M[2]);

    M[2] = ((r+l)/(r-l))*M[0] +
           ((t+b)/(t-b))*M[1] -
           ((f+n)/(f-n))*M[2] -
    M[3];
    M[0] *= 2*n/(r-l);
    M[1] *= 2*n/(t-b);
    M[3] = (-2*f*n/(f-n))*tmp;

    return *this;
}

// Apply a projection transformation using glOrtho()'s method
TStack &TStack::ortho(const float l, const float r,
                      const float b, const float t,
                      const float n, const float f)
{
    mat4    &M = mStack.back();

    M[3] += (-(r+l)/(r-l))*M[0] -
            ((t+b)/(t-b))*M[1] -
            ((f+n)/(f-n))*M[2];
    M[0] *= 2/(r-l);
    M[1] *= 2/(t-b);
    M[2] *= -2/(f-n);

    return *this;
}

// Apply a projection transformation using gluPerspective()'s method
TStack &TStack::perspective(const float fovy, const float aspect,
                            const float near, const float far)
{
    mat4    &M = mStack.back();

//    float   f = 1 / tanf(DegreesToRadians(fovy)/2.0);   // cot(fovy)
    float   f = tanf(M_PI_2 - DegreesToRadians(fovy*0.5));  // cot(fovy)

    vec4    tmp(M[2]);

    M[0] *= f/aspect;
    M[1] *= f;
    M[2] = ((far+near)/(near-far))*tmp - M[3];
    M[3] = (2*far*near/(near-far))*tmp;

    return *this;
}

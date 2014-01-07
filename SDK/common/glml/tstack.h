//
//  tstack.h
//  chapter4-5
//
//  Created by Chris Larsen on 8/1/11.
//  Copyright (c) 2014 Chris Larsen. All rights reserved.
//

#ifndef __glml__tstack__
#define __glml__tstack__

#include <iostream>
#include <vector>
#include <cmath>

#include "glml.h"

class stack_bottom : public std::logic_error {
public:
    explicit stack_bottom(const std::string &s) : std::logic_error(s) {}
    virtual ~stack_bottom(void) throw() {}
};

#define mglMODELVIEW    0
#define mglPROJECTION   1
#define mglTEXTURE      2
#define mglCOLOR        3

class TStack {
    std::vector<mat4>   mStack;

public:
    TStack(void) {
        mStack.push_back(mat4(1));
    }
    ~TStack(void) {}
    void push(void);
    void pop(void);
    mat4 &back(void) {
        return mStack.back();
    }
    const mat4 &operator[](const int i) const {
        if (i == INT_MAX) {
            return mStack.back();
        } else if (i < 0) {
            return mStack[mStack.size()-1+i];
        } else {
            return mStack[i];
        }
    }
    mat4 &operator[](const int i) {
        if (i == INT_MAX) {
            return mStack.back();
        } else if (i < 0) {
            return mStack[mStack.size()-1+i];
        } else {
            return mStack[i];
        }
    }
    TStack &loadIdentity(void) {
        mStack.back().loadIdentity();
        return *this;
    }
    TStack &loadRotation(const vec4 &);
    TStack &loadRotation(const float, const vec3 &);
    TStack &loadRotation(const quaternion &);
    TStack &rotate(const vec4 &);
    TStack &rotate(const float, const vec3 &);
    TStack &rotate(const quaternion &);
    TStack &loadScaleFactors(const vec3 &);
    TStack &scale(const vec3 &);
    TStack &loadTranslation(const vec3 &);
    TStack &translate(const vec3 &);
    TStack &loadMatrix(const mat4 &);
    TStack &multiplyMatrix(const mat4 &);
    const mat3 getNormalMatrix(void) const;
    TStack &lookAt(const vec3 &eye, const vec3 &center, const vec3 &up);
    TStack &frustum(const float left, const float right,
                    const float bottom, const float top,
                    const float near, const float far);
    TStack &ortho(const float left, const float right,
                  const float bottom, const float top,
                  const float near, const float far);
    TStack &perspective(const float fovy, const float aspect,
                        const float near, const float far);
    mat4 &pickMatrix(const float x, const float y,
                     const float width, const float height,
                     const int *viewport);
};

// Normal vector transformation matrix per "The OpenGL Shading Language,
// Language Version 1.20", p. 50, that is, it's the transpose of the inverse
// of the upper 3x3 submatrix of the model view matrix.  Rather than creating
// the submatrix by converting the top 3 rows of the model view matrix into
// vec3 types from vec4 types, taking the inverse of the matrix, and then
// taking the tranpose we can reduce the number of steps by first constructing
// the transpose of the upper 3x3 submatrix and taking its inverse.  This
// avoids 3 conversions from vec4 to vec3, and the transpose operation.  It's
// a small optimization but every little bit helps.
inline const mat3 TStack::getNormalMatrix(void) const
{
    const mat4  &m = this->mStack.back();
    return mat3(m[0][0], m[1][0], m[2][0],
                m[0][1], m[1][1], m[2][1],
                m[0][2], m[1][2], m[2][2]).inverse();
}

#endif /* defined(__glml__tstack__) */

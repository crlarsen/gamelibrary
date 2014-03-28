/*
 *  glml.cpp
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

#include "glml.h"

// This file has hand optimized versions of the matNxN<nrc,Type> methods
// determinant(), adjoint(), and inverse() for the 4x4 case.  The lower order
// cases generate the code inline and depend on good optimizing compilers to
// accelerate them.  The 4x4 case is important for 3D graphics and big enough to
// amortize the overhead of calling functions.  Of course, where to balance code
// size with potential compiler optimizations is more a matter of art than
// science.  The reader should be aware that where I've drawn the line may not
// be suitable in his problem space.  Worse than that, I have *no* empirical
// data to support my choice of where to draw the line.
//
// The 5x5 and all higher order cases for the determinant(), adjoint(), and
// inverse() rely on recursion for their computation.  Of course, that recursion
// will get short circuited at the 4x4 case because of the methods below.  If
// one needs hand tuned code for these cases please feel free to modify this
// code.

template <>
const float matNxN<4>::determinant(void) const {
    float   det=0.0f;

    if (_m[0][3]) {
        float bg = _m[0][1] * _m[1][2]; // 0 x x 3
        float bh = _m[0][1] * _m[1][3]; // 0 x 2 x
        float cf = _m[0][2] * _m[1][1]; // 0 x x 3
        float ch = _m[0][2] * _m[1][3]; // 0 1 x x
        float df = _m[0][3] * _m[1][1]; // 0 x 2 x
        float dg = _m[0][3] * _m[1][2]; // 0 1 x x

        float bg_cf = bg - cf;  // 0 x x 3
        float bh_df = bh - df;  // 0 x 2 x
        float ch_dg = ch - dg;  // 0 1 x x

        det += (-ch_dg*_m[2][1] + bh_df*_m[2][2] - bg_cf*_m[2][3]) * _m[3][0];

        if (_m[1][3]) {
            float ag = _m[0][0] * _m[1][2]; // x 1 x 3
            float ah = _m[0][0] * _m[1][3]; // x 1 2 x
            float ce = _m[0][2] * _m[1][0]; // x 1 x 3
            float de = _m[0][3] * _m[1][0]; // x 1 2 x

            float ag_ce = ag - ce;  // x 1 x 3
            float ah_de = ah - de;  // x 1 2 x

            det += ( ch_dg*_m[2][0] - ah_de*_m[2][2] + ag_ce*_m[2][3]) * _m[3][1];

            if (_m[2][3]) {
                float af = _m[0][0] * _m[1][1]; // x x 2 3
                float be = _m[0][1] * _m[1][0]; // x x 2 3

                float af_be = af - be;  // x x 2 3

                det += (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * _m[3][2];

                if (_m[3][3]) {
                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            } else {
                if (_m[3][3]) {
                    float af = _m[0][0] * _m[1][1]; // x x 2 3
                    float be = _m[0][1] * _m[1][0]; // x x 2 3

                    float af_be = af - be;  // x x 2 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            }
        } else {
            if (_m[2][3]) {
                float af = _m[0][0] * _m[1][1]; // x x 2 3
                float ah = _m[0][0] * _m[1][3]; // x 1 2 x
                float be = _m[0][1] * _m[1][0]; // x x 2 3
                float de = _m[0][3] * _m[1][0]; // x 1 2 x

                float af_be = af - be;  // x x 2 3
                float ah_de = ah - de;  // x 1 2 x

                det += (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * _m[3][2];

                if (_m[3][3]) {
                    float ag = _m[0][0] * _m[1][2]; // x 1 x 3
                    float ce = _m[0][2] * _m[1][0]; // x 1 x 3

                    float ag_ce = ag - ce;  // x 1 x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            } else {
                if (_m[3][3]) {
                    float af = _m[0][0] * _m[1][1]; // x x 2 3
                    float ag = _m[0][0] * _m[1][2]; // x 1 x 3
                    float be = _m[0][1] * _m[1][0]; // x x 2 3
                    float ce = _m[0][2] * _m[1][0]; // x 1 x 3

                    float af_be = af - be;  // x x 2 3
                    float ag_ce = ag - ce;  // x 1 x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            }
        }
    } else {
        if (_m[1][3]) {
            float ag = _m[0][0] * _m[1][2]; // x 1 x 3
            float ah = _m[0][0] * _m[1][3]; // x 1 2 x
            float ce = _m[0][2] * _m[1][0]; // x 1 x 3
            float ch = _m[0][2] * _m[1][3]; // 0 1 x x
            float de = _m[0][3] * _m[1][0]; // x 1 2 x
            float dg = _m[0][3] * _m[1][2]; // 0 1 x x

            float ag_ce = ag - ce;  // x 1 x 3
            float ah_de = ah - de;  // x 1 2 x
            float ch_dg = ch - dg;  // 0 1 x x

            det += ( ch_dg*_m[2][0] - ah_de*_m[2][2] + ag_ce*_m[2][3]) * _m[3][1];

            if (_m[2][3]) {
                float af = _m[0][0] * _m[1][1]; // x x 2 3
                float be = _m[0][1] * _m[1][0]; // x x 2 3
                float bh = _m[0][1] * _m[1][3]; // 0 x 2 x
                float df = _m[0][3] * _m[1][1]; // 0 x 2 x

                float af_be = af - be;  // x x 2 3
                float bh_df = bh - df;  // 0 x 2 x

                det += (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * _m[3][2];

                if (_m[3][3]) {
                    float bg = _m[0][1] * _m[1][2]; // 0 x x 3
                    float cf = _m[0][2] * _m[1][1]; // 0 x x 3

                    float bg_cf = bg - cf;  // 0 x x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            } else {
                if (_m[3][3]) {
                    float af = _m[0][0] * _m[1][1]; // x x 2 3
                    float be = _m[0][1] * _m[1][0]; // x x 2 3
                    float bg = _m[0][1] * _m[1][2]; // 0 x x 3
                    float cf = _m[0][2] * _m[1][1]; // 0 x x 3

                    float af_be = af - be;  // x x 2 3
                    float bg_cf = bg - cf;  // 0 x x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            }
        } else {
            if (_m[2][3]) {
                float af = _m[0][0] * _m[1][1]; // x x 2 3
                float ah = _m[0][0] * _m[1][3]; // x 1 2 x
                float be = _m[0][1] * _m[1][0]; // x x 2 3
                float bh = _m[0][1] * _m[1][3]; // 0 x 2 x
                float de = _m[0][3] * _m[1][0]; // x 1 2 x
                float df = _m[0][3] * _m[1][1]; // 0 x 2 x

                float af_be = af - be;  // x x 2 3
                float ah_de = ah - de;  // x 1 2 x
                float bh_df = bh - df;  // 0 x 2 x

                det += (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * _m[3][2];

                if (_m[3][3]) {
                    float ag = _m[0][0] * _m[1][2]; // x 1 x 3
                    float bg = _m[0][1] * _m[1][2]; // 0 x x 3
                    float ce = _m[0][2] * _m[1][0]; // x 1 x 3
                    float cf = _m[0][2] * _m[1][1]; // 0 x x 3

                    float ag_ce = ag - ce;  // x 1 x 3
                    float bg_cf = bg - cf;  // 0 x x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            } else {
                if (_m[3][3]) {
                    float af = _m[0][0] * _m[1][1]; // x x 2 3
                    float ag = _m[0][0] * _m[1][2]; // x 1 x 3
                    float be = _m[0][1] * _m[1][0]; // x x 2 3
                    float bg = _m[0][1] * _m[1][2]; // 0 x x 3
                    float ce = _m[0][2] * _m[1][0]; // x 1 x 3
                    float cf = _m[0][2] * _m[1][1]; // 0 x x 3
                    
                    float af_be = af - be;  // x x 2 3
                    float ag_ce = ag - ce;  // x 1 x 3
                    float bg_cf = bg - cf;  // 0 x x 3
                    
                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            }
        }
    }
    
    return det;
}

template <>
const mat<4,4> matNxN<4>::adjoint(void) const {
    float af = _m[0][0] * _m[1][1];
    float ag = _m[0][0] * _m[1][2];
    float ah = _m[0][0] * _m[1][3];
    float aj = _m[0][0] * _m[2][1];
    float ak = _m[0][0] * _m[2][2];
    float al = _m[0][0] * _m[2][3];
    float be = _m[0][1] * _m[1][0];
    float bg = _m[0][1] * _m[1][2];
    float bh = _m[0][1] * _m[1][3];
    float bi = _m[0][1] * _m[2][0];
    float bk = _m[0][1] * _m[2][2];
    float bl = _m[0][1] * _m[2][3];
    float ce = _m[0][2] * _m[1][0];
    float cf = _m[0][2] * _m[1][1];
    float ch = _m[0][2] * _m[1][3];
    float ci = _m[0][2] * _m[2][0];
    float cj = _m[0][2] * _m[2][1];
    float cl = _m[0][2] * _m[2][3];
    float de = _m[0][3] * _m[1][0];
    float df = _m[0][3] * _m[1][1];
    float dg = _m[0][3] * _m[1][2];
    float di = _m[0][3] * _m[2][0];
    float dj = _m[0][3] * _m[2][1];
    float dk = _m[0][3] * _m[2][2];
    float ej = _m[1][0] * _m[2][1];
    float ek = _m[1][0] * _m[2][2];
    float el = _m[1][0] * _m[2][3];
    float fi = _m[1][1] * _m[2][0];
    float fk = _m[1][1] * _m[2][2];
    float fl = _m[1][1] * _m[2][3];
    float gi = _m[1][2] * _m[2][0];
    float gj = _m[1][2] * _m[2][1];
    float gl = _m[1][2] * _m[2][3];
    float hi = _m[1][3] * _m[2][0];
    float hj = _m[1][3] * _m[2][1];
    float hk = _m[1][3] * _m[2][2];
    float af_be = af - be;
    float ag_ce = ag - ce;
    float ah_de = ah - de;
    float aj_bi = aj - bi;
    float ak_ci = ak - ci;
    float al_di = al - di;
    float bg_cf = bg - cf;
    float bh_df = bh - df;
    float bk_cj = bk - cj;
    float bl_dj = bl - dj;
    float ch_dg = ch - dg;
    float cl_dk = cl - dk;
    float ej_fi = ej - fi;
    float ek_gi = ek - gi;
    float el_hi = el - hi;
    float fk_gj = fk - gj;
    float fl_hj = fl - hj;
    float gl_hk = gl - hk;

    return mat4( gl_hk*_m[3][1] - fl_hj*_m[3][2] + fk_gj*_m[3][3],
                -cl_dk*_m[3][1] + bl_dj*_m[3][2] - bk_cj*_m[3][3],
                 ch_dg*_m[3][1] - bh_df*_m[3][2] + bg_cf*_m[3][3],
                -ch_dg*_m[2][1] + bh_df*_m[2][2] - bg_cf*_m[2][3],

                -gl_hk*_m[3][0] + el_hi*_m[3][2] - ek_gi*_m[3][3],
                 cl_dk*_m[3][0] - al_di*_m[3][2] + ak_ci*_m[3][3],
                -ch_dg*_m[3][0] + ah_de*_m[3][2] - ag_ce*_m[3][3],
                 ch_dg*_m[2][0] - ah_de*_m[2][2] + ag_ce*_m[2][3],

                 fl_hj*_m[3][0] - el_hi*_m[3][1] + ej_fi*_m[3][3],
                -bl_dj*_m[3][0] + al_di*_m[3][1] - aj_bi*_m[3][3],
                 bh_df*_m[3][0] - ah_de*_m[3][1] + af_be*_m[3][3],
                -bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3],

                -fk_gj*_m[3][0] + ek_gi*_m[3][1] - ej_fi*_m[3][2],
                 bk_cj*_m[3][0] - ak_ci*_m[3][1] + aj_bi*_m[3][2],
                -bg_cf*_m[3][0] + ag_ce*_m[3][1] - af_be*_m[3][2],
                 bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]);
}

template <>
const mat<4,4> matNxN<4>::inverse(void) const {
    float ej = _m[1][0] * _m[2][1];
    float ek = _m[1][0] * _m[2][2];
    float el = _m[1][0] * _m[2][3];
    float fi = _m[1][1] * _m[2][0];
    float fk = _m[1][1] * _m[2][2];
    float fl = _m[1][1] * _m[2][3];
    float gi = _m[1][2] * _m[2][0];
    float gj = _m[1][2] * _m[2][1];
    float gl = _m[1][2] * _m[2][3];
    float hi = _m[1][3] * _m[2][0];
    float hj = _m[1][3] * _m[2][1];
    float hk = _m[1][3] * _m[2][2];

    float ej_fi = ej - fi;
    float ek_gi = ek - gi;
    float el_hi = el - hi;
    float fk_gj = fk - gj;
    float fl_hj = fl - hj;
    float gl_hk = gl - hk;
    vec4    c1( gl_hk*_m[3][1] - fl_hj*_m[3][2] + fk_gj*_m[3][3],
               -gl_hk*_m[3][0] + el_hi*_m[3][2] - ek_gi*_m[3][3],
                fl_hj*_m[3][0] - el_hi*_m[3][1] + ej_fi*_m[3][3],
               -fk_gj*_m[3][0] + ek_gi*_m[3][1] - ej_fi*_m[3][2]);

    float       det = c1.dotProduct(_m[0]);

#ifndef NDEBUG
    if (det == 0.0f)
        throw std::runtime_error("Divide by zero");
#endif

    float   oneOverDet = 1.0f / det;

    float af = _m[0][0] * _m[1][1];
    float ag = _m[0][0] * _m[1][2];
    float ah = _m[0][0] * _m[1][3];
    float aj = _m[0][0] * _m[2][1];
    float ak = _m[0][0] * _m[2][2];
    float al = _m[0][0] * _m[2][3];
    float be = _m[0][1] * _m[1][0];
    float bg = _m[0][1] * _m[1][2];
    float bh = _m[0][1] * _m[1][3];
    float bi = _m[0][1] * _m[2][0];
    float bk = _m[0][1] * _m[2][2];
    float bl = _m[0][1] * _m[2][3];
    float ce = _m[0][2] * _m[1][0];
    float cf = _m[0][2] * _m[1][1];
    float ch = _m[0][2] * _m[1][3];
    float ci = _m[0][2] * _m[2][0];
    float cj = _m[0][2] * _m[2][1];
    float cl = _m[0][2] * _m[2][3];
    float de = _m[0][3] * _m[1][0];
    float df = _m[0][3] * _m[1][1];
    float dg = _m[0][3] * _m[1][2];
    float di = _m[0][3] * _m[2][0];
    float dj = _m[0][3] * _m[2][1];
    float dk = _m[0][3] * _m[2][2];
    float af_be = af - be;
    float ag_ce = ag - ce;
    float ah_de = ah - de;
    float aj_bi = aj - bi;
    float ak_ci = ak - ci;
    float al_di = al - di;
    float bg_cf = bg - cf;
    float bh_df = bh - df;
    float bk_cj = bk - cj;
    float bl_dj = bl - dj;
    float ch_dg = ch - dg;
    float cl_dk = cl - dk;

    return mat4(c1[0] * oneOverDet,
                (-cl_dk*_m[3][1] + bl_dj*_m[3][2] - bk_cj*_m[3][3]) * oneOverDet,
                ( ch_dg*_m[3][1] - bh_df*_m[3][2] + bg_cf*_m[3][3]) * oneOverDet,
                (-ch_dg*_m[2][1] + bh_df*_m[2][2] - bg_cf*_m[2][3]) * oneOverDet,

                c1[1] * oneOverDet,
                ( cl_dk*_m[3][0] - al_di*_m[3][2] + ak_ci*_m[3][3]) * oneOverDet,
                (-ch_dg*_m[3][0] + ah_de*_m[3][2] - ag_ce*_m[3][3]) * oneOverDet,
                ( ch_dg*_m[2][0] - ah_de*_m[2][2] + ag_ce*_m[2][3]) * oneOverDet,

                c1[2] * oneOverDet,
                (-bl_dj*_m[3][0] + al_di*_m[3][1] - aj_bi*_m[3][3]) * oneOverDet,
                ( bh_df*_m[3][0] - ah_de*_m[3][1] + af_be*_m[3][3]) * oneOverDet,
                (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * oneOverDet,

                c1[3] * oneOverDet,
                ( bk_cj*_m[3][0] - ak_ci*_m[3][1] + aj_bi*_m[3][2]) * oneOverDet,
                (-bg_cf*_m[3][0] + ag_ce*_m[3][1] - af_be*_m[3][2]) * oneOverDet,
                ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * oneOverDet);
}

template <>
const double matNxN<4,double>::determinant(void) const {
    double   det=0.0f;

    if (_m[0][3]) {
        double bg = _m[0][1] * _m[1][2]; // 0 x x 3
        double bh = _m[0][1] * _m[1][3]; // 0 x 2 x
        double cf = _m[0][2] * _m[1][1]; // 0 x x 3
        double ch = _m[0][2] * _m[1][3]; // 0 1 x x
        double df = _m[0][3] * _m[1][1]; // 0 x 2 x
        double dg = _m[0][3] * _m[1][2]; // 0 1 x x

        double bg_cf = bg - cf;  // 0 x x 3
        double bh_df = bh - df;  // 0 x 2 x
        double ch_dg = ch - dg;  // 0 1 x x

        det += (-ch_dg*_m[2][1] + bh_df*_m[2][2] - bg_cf*_m[2][3]) * _m[3][0];

        if (_m[1][3]) {
            double ag = _m[0][0] * _m[1][2]; // x 1 x 3
            double ah = _m[0][0] * _m[1][3]; // x 1 2 x
            double ce = _m[0][2] * _m[1][0]; // x 1 x 3
            double de = _m[0][3] * _m[1][0]; // x 1 2 x

            double ag_ce = ag - ce;  // x 1 x 3
            double ah_de = ah - de;  // x 1 2 x

            det += ( ch_dg*_m[2][0] - ah_de*_m[2][2] + ag_ce*_m[2][3]) * _m[3][1];

            if (_m[2][3]) {
                double af = _m[0][0] * _m[1][1]; // x x 2 3
                double be = _m[0][1] * _m[1][0]; // x x 2 3

                double af_be = af - be;  // x x 2 3

                det += (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * _m[3][2];

                if (_m[3][3]) {
                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            } else {
                if (_m[3][3]) {
                    double af = _m[0][0] * _m[1][1]; // x x 2 3
                    double be = _m[0][1] * _m[1][0]; // x x 2 3

                    double af_be = af - be;  // x x 2 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            }
        } else {
            if (_m[2][3]) {
                double af = _m[0][0] * _m[1][1]; // x x 2 3
                double ah = _m[0][0] * _m[1][3]; // x 1 2 x
                double be = _m[0][1] * _m[1][0]; // x x 2 3
                double de = _m[0][3] * _m[1][0]; // x 1 2 x

                double af_be = af - be;  // x x 2 3
                double ah_de = ah - de;  // x 1 2 x

                det += (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * _m[3][2];

                if (_m[3][3]) {
                    double ag = _m[0][0] * _m[1][2]; // x 1 x 3
                    double ce = _m[0][2] * _m[1][0]; // x 1 x 3

                    double ag_ce = ag - ce;  // x 1 x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            } else {
                if (_m[3][3]) {
                    double af = _m[0][0] * _m[1][1]; // x x 2 3
                    double ag = _m[0][0] * _m[1][2]; // x 1 x 3
                    double be = _m[0][1] * _m[1][0]; // x x 2 3
                    double ce = _m[0][2] * _m[1][0]; // x 1 x 3

                    double af_be = af - be;  // x x 2 3
                    double ag_ce = ag - ce;  // x 1 x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            }
        }
    } else {
        if (_m[1][3]) {
            double ag = _m[0][0] * _m[1][2]; // x 1 x 3
            double ah = _m[0][0] * _m[1][3]; // x 1 2 x
            double ce = _m[0][2] * _m[1][0]; // x 1 x 3
            double ch = _m[0][2] * _m[1][3]; // 0 1 x x
            double de = _m[0][3] * _m[1][0]; // x 1 2 x
            double dg = _m[0][3] * _m[1][2]; // 0 1 x x

            double ag_ce = ag - ce;  // x 1 x 3
            double ah_de = ah - de;  // x 1 2 x
            double ch_dg = ch - dg;  // 0 1 x x

            det += ( ch_dg*_m[2][0] - ah_de*_m[2][2] + ag_ce*_m[2][3]) * _m[3][1];

            if (_m[2][3]) {
                double af = _m[0][0] * _m[1][1]; // x x 2 3
                double be = _m[0][1] * _m[1][0]; // x x 2 3
                double bh = _m[0][1] * _m[1][3]; // 0 x 2 x
                double df = _m[0][3] * _m[1][1]; // 0 x 2 x

                double af_be = af - be;  // x x 2 3
                double bh_df = bh - df;  // 0 x 2 x

                det += (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * _m[3][2];

                if (_m[3][3]) {
                    double bg = _m[0][1] * _m[1][2]; // 0 x x 3
                    double cf = _m[0][2] * _m[1][1]; // 0 x x 3

                    double bg_cf = bg - cf;  // 0 x x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            } else {
                if (_m[3][3]) {
                    double af = _m[0][0] * _m[1][1]; // x x 2 3
                    double be = _m[0][1] * _m[1][0]; // x x 2 3
                    double bg = _m[0][1] * _m[1][2]; // 0 x x 3
                    double cf = _m[0][2] * _m[1][1]; // 0 x x 3

                    double af_be = af - be;  // x x 2 3
                    double bg_cf = bg - cf;  // 0 x x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            }
        } else {
            if (_m[2][3]) {
                double af = _m[0][0] * _m[1][1]; // x x 2 3
                double ah = _m[0][0] * _m[1][3]; // x 1 2 x
                double be = _m[0][1] * _m[1][0]; // x x 2 3
                double bh = _m[0][1] * _m[1][3]; // 0 x 2 x
                double de = _m[0][3] * _m[1][0]; // x 1 2 x
                double df = _m[0][3] * _m[1][1]; // 0 x 2 x

                double af_be = af - be;  // x x 2 3
                double ah_de = ah - de;  // x 1 2 x
                double bh_df = bh - df;  // 0 x 2 x

                det += (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * _m[3][2];

                if (_m[3][3]) {
                    double ag = _m[0][0] * _m[1][2]; // x 1 x 3
                    double bg = _m[0][1] * _m[1][2]; // 0 x x 3
                    double ce = _m[0][2] * _m[1][0]; // x 1 x 3
                    double cf = _m[0][2] * _m[1][1]; // 0 x x 3

                    double ag_ce = ag - ce;  // x 1 x 3
                    double bg_cf = bg - cf;  // 0 x x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            } else {
                if (_m[3][3]) {
                    double af = _m[0][0] * _m[1][1]; // x x 2 3
                    double ag = _m[0][0] * _m[1][2]; // x 1 x 3
                    double be = _m[0][1] * _m[1][0]; // x x 2 3
                    double bg = _m[0][1] * _m[1][2]; // 0 x x 3
                    double ce = _m[0][2] * _m[1][0]; // x 1 x 3
                    double cf = _m[0][2] * _m[1][1]; // 0 x x 3

                    double af_be = af - be;  // x x 2 3
                    double ag_ce = ag - ce;  // x 1 x 3
                    double bg_cf = bg - cf;  // 0 x x 3

                    det += ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * _m[3][3];
                }
            }
        }
    }

    return det;
}

template <>
const mat<4,4,double> matNxN<4,double>::adjoint(void) const {
    double af = _m[0][0] * _m[1][1];
    double ag = _m[0][0] * _m[1][2];
    double ah = _m[0][0] * _m[1][3];
    double aj = _m[0][0] * _m[2][1];
    double ak = _m[0][0] * _m[2][2];
    double al = _m[0][0] * _m[2][3];
    double be = _m[0][1] * _m[1][0];
    double bg = _m[0][1] * _m[1][2];
    double bh = _m[0][1] * _m[1][3];
    double bi = _m[0][1] * _m[2][0];
    double bk = _m[0][1] * _m[2][2];
    double bl = _m[0][1] * _m[2][3];
    double ce = _m[0][2] * _m[1][0];
    double cf = _m[0][2] * _m[1][1];
    double ch = _m[0][2] * _m[1][3];
    double ci = _m[0][2] * _m[2][0];
    double cj = _m[0][2] * _m[2][1];
    double cl = _m[0][2] * _m[2][3];
    double de = _m[0][3] * _m[1][0];
    double df = _m[0][3] * _m[1][1];
    double dg = _m[0][3] * _m[1][2];
    double di = _m[0][3] * _m[2][0];
    double dj = _m[0][3] * _m[2][1];
    double dk = _m[0][3] * _m[2][2];
    double ej = _m[1][0] * _m[2][1];
    double ek = _m[1][0] * _m[2][2];
    double el = _m[1][0] * _m[2][3];
    double fi = _m[1][1] * _m[2][0];
    double fk = _m[1][1] * _m[2][2];
    double fl = _m[1][1] * _m[2][3];
    double gi = _m[1][2] * _m[2][0];
    double gj = _m[1][2] * _m[2][1];
    double gl = _m[1][2] * _m[2][3];
    double hi = _m[1][3] * _m[2][0];
    double hj = _m[1][3] * _m[2][1];
    double hk = _m[1][3] * _m[2][2];
    double af_be = af - be;
    double ag_ce = ag - ce;
    double ah_de = ah - de;
    double aj_bi = aj - bi;
    double ak_ci = ak - ci;
    double al_di = al - di;
    double bg_cf = bg - cf;
    double bh_df = bh - df;
    double bk_cj = bk - cj;
    double bl_dj = bl - dj;
    double ch_dg = ch - dg;
    double cl_dk = cl - dk;
    double ej_fi = ej - fi;
    double ek_gi = ek - gi;
    double el_hi = el - hi;
    double fk_gj = fk - gj;
    double fl_hj = fl - hj;
    double gl_hk = gl - hk;

    return dmat4( gl_hk*_m[3][1] - fl_hj*_m[3][2] + fk_gj*_m[3][3],
                 -cl_dk*_m[3][1] + bl_dj*_m[3][2] - bk_cj*_m[3][3],
                  ch_dg*_m[3][1] - bh_df*_m[3][2] + bg_cf*_m[3][3],
                 -ch_dg*_m[2][1] + bh_df*_m[2][2] - bg_cf*_m[2][3],

                 -gl_hk*_m[3][0] + el_hi*_m[3][2] - ek_gi*_m[3][3],
                  cl_dk*_m[3][0] - al_di*_m[3][2] + ak_ci*_m[3][3],
                 -ch_dg*_m[3][0] + ah_de*_m[3][2] - ag_ce*_m[3][3],
                  ch_dg*_m[2][0] - ah_de*_m[2][2] + ag_ce*_m[2][3],

                  fl_hj*_m[3][0] - el_hi*_m[3][1] + ej_fi*_m[3][3],
                 -bl_dj*_m[3][0] + al_di*_m[3][1] - aj_bi*_m[3][3],
                  bh_df*_m[3][0] - ah_de*_m[3][1] + af_be*_m[3][3],
                 -bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3],

                 -fk_gj*_m[3][0] + ek_gi*_m[3][1] - ej_fi*_m[3][2],
                  bk_cj*_m[3][0] - ak_ci*_m[3][1] + aj_bi*_m[3][2],
                 -bg_cf*_m[3][0] + ag_ce*_m[3][1] - af_be*_m[3][2],
                  bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]);
}

template <>
const mat<4,4,double> matNxN<4,double>::inverse(void) const {
    double ej = _m[1][0] * _m[2][1];
    double ek = _m[1][0] * _m[2][2];
    double el = _m[1][0] * _m[2][3];
    double fi = _m[1][1] * _m[2][0];
    double fk = _m[1][1] * _m[2][2];
    double fl = _m[1][1] * _m[2][3];
    double gi = _m[1][2] * _m[2][0];
    double gj = _m[1][2] * _m[2][1];
    double gl = _m[1][2] * _m[2][3];
    double hi = _m[1][3] * _m[2][0];
    double hj = _m[1][3] * _m[2][1];
    double hk = _m[1][3] * _m[2][2];

    double ej_fi = ej - fi;
    double ek_gi = ek - gi;
    double el_hi = el - hi;
    double fk_gj = fk - gj;
    double fl_hj = fl - hj;
    double gl_hk = gl - hk;
    dvec4    c1( gl_hk*_m[3][1] - fl_hj*_m[3][2] + fk_gj*_m[3][3],
                -gl_hk*_m[3][0] + el_hi*_m[3][2] - ek_gi*_m[3][3],
                 fl_hj*_m[3][0] - el_hi*_m[3][1] + ej_fi*_m[3][3],
                -fk_gj*_m[3][0] + ek_gi*_m[3][1] - ej_fi*_m[3][2]);

    double       det = c1.dotProduct(_m[0]);

#ifndef NDEBUG
    if (det == 0.0)
        throw std::runtime_error("Divide by zero");
#endif

    double   oneOverDet = 1.0f / det;

    double af = _m[0][0] * _m[1][1];
    double ag = _m[0][0] * _m[1][2];
    double ah = _m[0][0] * _m[1][3];
    double aj = _m[0][0] * _m[2][1];
    double ak = _m[0][0] * _m[2][2];
    double al = _m[0][0] * _m[2][3];
    double be = _m[0][1] * _m[1][0];
    double bg = _m[0][1] * _m[1][2];
    double bh = _m[0][1] * _m[1][3];
    double bi = _m[0][1] * _m[2][0];
    double bk = _m[0][1] * _m[2][2];
    double bl = _m[0][1] * _m[2][3];
    double ce = _m[0][2] * _m[1][0];
    double cf = _m[0][2] * _m[1][1];
    double ch = _m[0][2] * _m[1][3];
    double ci = _m[0][2] * _m[2][0];
    double cj = _m[0][2] * _m[2][1];
    double cl = _m[0][2] * _m[2][3];
    double de = _m[0][3] * _m[1][0];
    double df = _m[0][3] * _m[1][1];
    double dg = _m[0][3] * _m[1][2];
    double di = _m[0][3] * _m[2][0];
    double dj = _m[0][3] * _m[2][1];
    double dk = _m[0][3] * _m[2][2];
    double af_be = af - be;
    double ag_ce = ag - ce;
    double ah_de = ah - de;
    double aj_bi = aj - bi;
    double ak_ci = ak - ci;
    double al_di = al - di;
    double bg_cf = bg - cf;
    double bh_df = bh - df;
    double bk_cj = bk - cj;
    double bl_dj = bl - dj;
    double ch_dg = ch - dg;
    double cl_dk = cl - dk;

    return dmat4(c1[0] * oneOverDet,
                 (-cl_dk*_m[3][1] + bl_dj*_m[3][2] - bk_cj*_m[3][3]) * oneOverDet,
                 ( ch_dg*_m[3][1] - bh_df*_m[3][2] + bg_cf*_m[3][3]) * oneOverDet,
                 (-ch_dg*_m[2][1] + bh_df*_m[2][2] - bg_cf*_m[2][3]) * oneOverDet,

                 c1[1] * oneOverDet,
                 ( cl_dk*_m[3][0] - al_di*_m[3][2] + ak_ci*_m[3][3]) * oneOverDet,
                 (-ch_dg*_m[3][0] + ah_de*_m[3][2] - ag_ce*_m[3][3]) * oneOverDet,
                 ( ch_dg*_m[2][0] - ah_de*_m[2][2] + ag_ce*_m[2][3]) * oneOverDet,
                
                 c1[2] * oneOverDet,
                 (-bl_dj*_m[3][0] + al_di*_m[3][1] - aj_bi*_m[3][3]) * oneOverDet,
                 ( bh_df*_m[3][0] - ah_de*_m[3][1] + af_be*_m[3][3]) * oneOverDet,
                 (-bh_df*_m[2][0] + ah_de*_m[2][1] - af_be*_m[2][3]) * oneOverDet,
                
                 c1[3] * oneOverDet,
                 ( bk_cj*_m[3][0] - ak_ci*_m[3][1] + aj_bi*_m[3][2]) * oneOverDet,
                 (-bg_cf*_m[3][0] + ag_ce*_m[3][1] - af_be*_m[3][2]) * oneOverDet,
                 ( bg_cf*_m[2][0] - ag_ce*_m[2][1] + af_be*_m[2][2]) * oneOverDet);
}

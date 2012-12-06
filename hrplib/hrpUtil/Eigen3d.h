/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * National Institute of Advanced Industrial Science and Technology (AIST)
 * General Robotix Inc. 
 */

#ifndef HRPUTIL_EIGEN3D_H_INCLUDED
#define HRPUTIL_EIGEN3D_H_INCLUDED

#include "config.h"
#include "EigenTypes.h"

namespace hrp
{
    typedef Eigen::Vector3d Vector3Ref;
    inline Vector3Ref getVector3Ref(const double* data) {
        return Vector3Ref(data[0], data[1], data[2]);
    }

    HRP_UTIL_EXPORT void calcRodrigues(Matrix33& out_R, const Vector3& axis, double q);
    HRP_UTIL_EXPORT void calcRotFromRpy(Matrix33& out_R, double r, double p, double y);

    inline Matrix33 rodrigues(const Vector3& axis, double q){
        Matrix33 R;
        calcRodrigues(R, axis, q);
        return R;
    }
    
    inline Matrix33 rotFromRpy(const Vector3& rpy){
        Matrix33 R;
        calcRotFromRpy(R, rpy[0], rpy[1], rpy[2]);
        return R;
    }
    
    inline Matrix33 rotFromRpy(double r, double p, double y){
        Matrix33 R;
        calcRotFromRpy(R, r, p, y);
        return R;
    }

    inline Matrix33 rotationX(double theta){
        Matrix33 R;
        double c = cos(theta);
        double s = sin(theta);
        R << 1.0, 0.0, 0.0,
            0.0,  c,  -s,
            0.0,  s,   c ;
        return R;
    }
    
    inline Matrix33 rotationY(double theta){
        Matrix33 R;
        double c = cos(theta);
        double s = sin(theta);
        R << c,   0.0,  s,
            0.0, 1.0, 0.0,
            -s,  0.0,  c ;
        return R;
    }
    
    inline Matrix33 rotationZ(double theta){
        Matrix33 R;
        double c = cos(theta);
        double s = sin(theta);
        R <<  c,  -s,  0.0,
             s,   c,  0.0,
            0.0, 0.0, 1.0;
        return R;
    }
    
    HRP_UTIL_EXPORT Vector3 omegaFromRot(const Matrix33& r);
    HRP_UTIL_EXPORT Vector3 rpyFromRot(const Matrix33& m);
    
    HRP_UTIL_EXPORT bool isOrthogonalMatrix(Matrix33& m);

    inline Matrix33 hat(const Vector3& c) {
        Matrix33 m;
        m << 0.0,  -c(2),  c(1),
            c(2),  0.0,  -c(0),
            -c(1),  c(0),  0.0;
        return m;
    }
    
    template<class M> inline void setMatrix33(const Matrix33& m33, M& m, size_t row = 0, size_t col = 0){
        m(row, col) = m33(0, 0); m(row, col+1) = m33(0, 1); m(row, col+2) = m33(0, 2);
        ++row;
        m(row, col) = m33(1, 0); m(row, col+1) = m33(1, 1); m(row, col+2) = m33(1, 2);
        ++row;
        m(row, col) = m33(2, 0); m(row, col+1) = m33(2, 1); m(row, col+2) = m33(2, 2);
    }
    
    template<class M> inline void setTransMatrix33(const Matrix33& m33, M& m, size_t row = 0, size_t col = 0){
        m(row, col) = m33(0, 0); m(row, col+1) = m33(1, 0); m(row, col+2) = m33(2, 0);
        ++row;
        m(row, col) = m33(0, 1); m(row, col+1) = m33(1, 1); m(row, col+2) = m33(2, 1);
        ++row;
        m(row, col) = m33(0, 2); m(row, col+1) = m33(1, 2); m(row, col+2) = m33(2, 2);
    }
    
    template<class Array> inline void setMatrix33ToRowMajorArray
        (const Matrix33& m33, Array& a, size_t top = 0) {
        a[top++] = m33(0, 0);
        a[top++] = m33(0, 1);
        a[top++] = m33(0, 2);
        a[top++] = m33(1, 0);
        a[top++] = m33(1, 1);
        a[top++] = m33(1, 2);
        a[top++] = m33(2, 0);
        a[top++] = m33(2, 1);
        a[top  ] = m33(2, 2);
    }
    
    template<class Array> inline void getMatrix33FromRowMajorArray
        (Matrix33& m33, const Array& a, size_t top = 0) {
        m33(0, 0) = a[top++];
        m33(0, 1) = a[top++];
        m33(0, 2) = a[top++];
        m33(1, 0) = a[top++];
        m33(1, 1) = a[top++];
        m33(1, 2) = a[top++];
        m33(2, 0) = a[top++];
        m33(2, 1) = a[top++];
        m33(2, 2) = a[top  ];
    }
    
    template<class V> inline void setVector3(const Vector3& v3, V& v, size_t top = 0){
        v[top++] = v3(0); v[top++] = v3(1); v[top] = v3(2);
    }
    
    template<class V> inline void setVector3(const Vector3& v3, const V& v, size_t top = 0){
        v[top++] = v3(0); v[top++] = v3(1); v[top] = v3(2);
    }
    
    template<class V> inline void getVector3(Vector3& v3, const V& v, size_t top = 0){
        v3(0) = v[top++]; v3(1) = v[top++]; v3(2) = v[top]; 
    }
    
    template<class M> inline void setVector3(const Vector3& v3, M& m, size_t row, size_t col){
        m(row++, col) = v3(0); m(row++, col) = v3(1); m(row, col) = v3(2); 
    }
    
    template<class M> inline void getVector3(Vector3& v3, const M& m, size_t row, size_t col){
        v3(0) = m(row++, col);
        v3(1) = m(row++, col);
        v3(2) = m(row, col);
    }
    
};

#endif

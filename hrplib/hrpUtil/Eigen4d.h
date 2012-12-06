/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * National Institute of Advanced Industrial Science and Technology (AIST)
 */

#ifndef OPENHRP_UTIL_EIGEN4D_H_INCLUDED
#define OPENHRP_UTIL_EIGEN4D_H_INCLUDED

#include "config.h"
#include "EigenTypes.h"

namespace hrp
{
    class VrmlTransform;

    typedef Eigen::Matrix4d Matrix44;
    typedef Eigen::Vector4d Vector4;

    HRP_UTIL_EXPORT void calcRodrigues(Matrix44& out_R, const Vector3& axis, double q);
    HRP_UTIL_EXPORT void calcTransformMatrix(VrmlTransform* transform, Matrix44& out_T);
};

#endif

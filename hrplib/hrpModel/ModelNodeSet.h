/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * National Institute of Advanced Industrial Science and Technology (AIST)
 */

/*! @file
  @author Shin'ichiro Nakaoka
*/

#ifndef OPENHRP_MODEL_NODE_SET_H_INCLUDED
#define OPENHRP_MODEL_NODE_SET_H_INCLUDED

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/signals.hpp>
#include <hrpUtil/VrmlNodes.h>
#include <hrpUtil/Eigen4d.h>
#include "Config.h"


namespace hrp {

    class VrmlParser;

    class JointNodeSet;
    typedef boost::shared_ptr<JointNodeSet> JointNodeSetPtr;

    class JointNodeSet
    {
    public:
        VrmlProtoInstancePtr jointNode;
        std::vector<JointNodeSetPtr> childJointNodeSets;
        std::vector<Matrix44, Eigen::aligned_allocator<Matrix44> > transforms;
        std::vector<VrmlProtoInstancePtr> segmentNodes;
        std::vector<VrmlProtoInstancePtr> sensorNodes;
        std::vector<VrmlProtoInstancePtr> hwcNodes;
    };
    
    typedef std::vector<JointNodeSetPtr> JointNodeSetArray;

    class ModelNodeSetImpl;

    class HRPMODEL_API ModelNodeSet
    {
      public:

        ModelNodeSet();
        virtual ~ModelNodeSet();

        bool loadModelFile(const std::string& filename);
		
        int numJointNodes();
        VrmlProtoInstancePtr humanoidNode();
        JointNodeSetPtr rootJointNodeSet();

        /**
           @if jp
           読み込み進行状況のメッセージを出力するためのシグナル.
           @note エラー発生時のメッセージはこのシグナルではなく例外によって処理される。
           @endif
        */
        boost::signal<void(const std::string& message)> sigMessage;

        class Exception {
        public:
            Exception(const std::string& description) : description(description) { }
            const char* what() const { return description.c_str(); }
        private:
            std::string description;
        };

      private:
        ModelNodeSetImpl* impl;
    };

    typedef boost::shared_ptr<ModelNodeSet> ModelNodeSetPtr;
};
    

#endif

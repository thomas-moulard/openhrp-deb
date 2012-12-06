/*! @file
  @author Shin'ichiro Nakaoka
*/

#ifndef OPENHRP_MODEL_LOADER_VRML_UTIL_H_INCLUDED
#define OPENHRP_MODEL_LOADER_VRML_UTIL_H_INCLUDED

#include <string>
#include <hrpCorba/ModelLoader.hh>
#include <hrpUtil/VrmlNodes.h>
#include <hrpUtil/UrlUtil.h>

using namespace hrp;
using namespace OpenHRP;

void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, std::string& out_s);
void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, DblSequence& out_v);
void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, CORBA::Double& out_v);
void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, CORBA::Long& out_v);

//void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, DblSequence3& out_v);
//void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, DblSequence9& out_m);

void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, DblArray3& out_v);
void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, DblArray9& out_m);

void copyVrmlRotationFieldToDblArray9(TProtoFieldMap& fieldMap, const std::string name, DblArray9& out_R);
void copyVrmlRotationFieldToDblArray4(TProtoFieldMap& fieldMap, const std::string name, DblArray4& out_R);

std::string setTexturefileUrl( std::string modelfileDir, hrp::MFString urls);
#endif

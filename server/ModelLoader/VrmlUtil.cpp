/*! @file
  @author S.NAKAOKA
*/

#include "VrmlUtil.h"
#include <cmath>

using namespace std;

namespace {
    void throwException(const std::string& fieldName, const std::string& expectedFieldType)
    {
        string error;
        error += "The node must have a field \"" + fieldName + "\" of " + expectedFieldType + " type";
        throw ModelLoader::ModelLoaderException(error.c_str());
    }
}




void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, std::string& out_s)
{
    VrmlVariantField& f = fmap[name];
    switch(f.typeId()){
    case SFSTRING:
        out_s = f.sfString();
        break;
    case MFSTRING:
    {
        MFString& strings = f.mfString();
        out_s = "";
        for(size_t i=0; i < strings.size(); i++){
            out_s += strings[i] + "\n";
        }
    }
    break;
    default:
        throwException(name, "SFString or MFString");
    }
}


void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, DblSequence& out_v)
{
    VrmlVariantField& f = fmap[name];
    switch(f.typeId()){
    case MFFLOAT:
    {
        MFFloat& mf = f.mfFloat();
        CORBA::ULong n = mf.size();
        out_v.length(n);
        for(CORBA::ULong i=0; i < n; ++i){
            out_v[i] = mf[i];
        }
    }
    break;
    default:
        throwException(name, "MFFloat");
    }
}


void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, DblArray3& out_v)
{
    VrmlVariantField& f = fmap[name];
    switch(f.typeId()){
    case SFVEC3F:
    case SFCOLOR:
    {
        SFVec3f& v = f.sfVec3f();
        for(int i = 0; i < 3; ++i){
            out_v[i] = v[i];
        }
    }
    break;
    default:
        throwException(name, "SFVec3f or SFColor");
    }
}
    
void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, DblArray9& out_m)
{
    VrmlVariantField& f = fmap[name];
    switch(f.typeId()){
    case MFFLOAT:
    {
        MFFloat& mf = f.mfFloat();
        if(mf.size() == 9){
            for(int i=0; i < 9; ++i){
                out_m[i] = mf[i];
            }
        } else {
            throw ModelLoader::ModelLoaderException("illegal size of a matrix field");
        }
    }
    break;
    default:
        throwException(name, "MFFloat");
    }
}

void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, CORBA::Double& out_v)
{
    VrmlVariantField& f = fmap[name];
    switch(f.typeId()){
    case SFFLOAT:
        out_v = f.sfFloat();
        break;
    default:
        throwException(name, "SFFloat");
    }
}


void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, CORBA::Long& out_v)
{
    VrmlVariantField& f = fmap[name];
    switch(f.typeId()){
    case SFINT32:
        out_v = f.sfInt32();
        break;
    default:
        throwException(name, "SFInt32");
    }
}



//void copyVrmlField(TProtoFieldMap& fmap, const std::string& name, CORBA::Long& out_v)
//{
//	VrmlVariantField& f = fmap[name];
//	switch( f.typeId() )
//	{
//	case SFINT32:
//		out_v = f.sfInt32();
//		break;
//	default:
//		throwException( name, "SFInt32" );
//	}
//}



void copyVrmlRotationFieldToDblArray9
(TProtoFieldMap& fieldMap, const std::string name, DblArray9& out_R)
{
    VrmlVariantField& rotationField = fieldMap[name];

    if(rotationField.typeId() != SFROTATION){
        throwException(name, "SFRotation");
    }

    SFRotation& r = rotationField.sfRotation();

    const double& theta = r[3];
    const double sth = sin(theta);
    const double vth = 1.0 - cos(theta);

    double ax = r[0];
    double ay = r[1];
    double az = r[2];

    // normalization
    double l = sqrt(ax*ax + ay*ay + az*az);
	
    //if(fabs(l - 1.0) > 1.0e-6){
    //    throwException(name, "Not normalized axis");
    //}
	   
    ax /= l;
    ay /= l;
    az /= l;

    const double axx = ax*ax*vth;
    const double ayy = ay*ay*vth;
    const double azz = az*az*vth;
    const double axy = ax*ay*vth;
    const double ayz = ay*az*vth;
    const double azx = az*ax*vth;

    ax *= sth;
    ay *= sth;
    az *= sth;

    out_R[0] = 1.0 - azz - ayy; out_R[1] = -az + axy;       out_R[2] = ay + azx;
    out_R[3] = az + axy;        out_R[4] = 1.0 - azz - axx; out_R[5] = -ax + ayz;
    out_R[6] = -ay + azx;       out_R[7] = ax + ayz;        out_R[8] = 1.0 - ayy - axx;
}


void copyVrmlRotationFieldToDblArray4
(TProtoFieldMap& fieldMap, const std::string name, DblArray4& out_R)
{
    VrmlVariantField& rotationField = fieldMap[name];

    if(rotationField.typeId() != SFROTATION)
        {
            throwException( name, "SFRotation" );
        }

    SFRotation& r = rotationField.sfRotation();

    for( int i = 0 ; i < 4 ; i++ )
        {
            out_R[i] = r[i];
        }
}

string setTexturefileUrl(string modelfileDir, MFString urls){
    string retUrl("");
    //  ImageTextureに格納されている MFString url の数を確認 //
    if( 0 == urls.size() )
    {
        string error;
        error += "ImageTexture read error: No urls in ImageTexture node";
        throw ModelLoader::ModelLoaderException(error.c_str());
    }else{
        for(unsigned int i=0; i<urls.size(); i++){
        	getPathFromUrl( retUrl, modelfileDir, urls[i] );
        	if( !retUrl.empty() )
        		break;
        }
    }
    return retUrl;
}

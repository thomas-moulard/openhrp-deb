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
  @author K.Fukuda
*/

#include "VrmlParser.h"

#include <cmath>
#include <vector>
#include <list>
#include <iostream>
#include <hrpUtil/EasyScanner.h>
#include <hrpUtil/UrlUtil.h>

#include <boost/version.hpp>
#if (BOOST_VERSION <= 103301)
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#else
#include <boost/filesystem.hpp>
#endif

using namespace std;
using namespace boost;
using namespace hrp;

namespace {

    /**
       The definition of the reserved word IDs
    */
    enum {

        NO_SYMBOL = 0,

        // values
        V_TRUE,
        V_FALSE,
        V_NULL,

        // types
        T_SFINT32,
        T_MFINT32,
        T_SFFLOAT,
        T_MFFLOAT,
        T_SFVEC2F,
        T_MFVEC2F,
        T_SFVEC3F,
        T_MFVEC3F,
        T_SFSTRING,
        T_MFSTRING,
        T_SFROTATION,
        T_MFROTATION,
        T_SFTIME,
        T_MFTIME,
        T_SFCOLOR,
        T_MFCOLOR,
        T_SFNODE,
        T_MFNODE,
        T_SFBOOL,
	T_SFIMAGE,

        // Nodes
        N_PROTO,
        N_INLINE,
        N_BACKGROUND,
        N_NAVIGATION_INFO,
        N_VIEWPOINT,
        N_GROUP,
        N_TRANSFORM,
        N_APPEARANCE,
        N_MATERIAL,
        N_IMAGE_TEXTURE,
        N_TEXTURE_TRANSFORM,
        N_SHAPE,
        N_BOX,
        N_CONE,
        N_CYLINDER,
        N_SPHERE,
        N_TEXT,
        N_FONT_STYLE,
        N_INDEXED_LINE_SET,
        N_INDEXED_FACE_SET,
        N_COLOR,
        N_COORDINATE,
        N_TEXTURE_COORDINATE,
        N_NORMAL,
        N_CYLINDER_SENSOR,

        N_POINTSET,
	N_PIXEL_TEXTURE,
	N_MOVIE_TEXTURE,
	N_ELEVATION_GRID,
	N_EXTRUSION,
	N_SWITCH,
	N_LOD,
	N_COLLISION,
	N_ANCHOR,
	N_FOG,
	N_BILLBOARD,
	N_WORLD_INFO,
	N_POINT_LIGHT,
	N_DIRECTIONAL_LIGHT,
	N_SPOT_LIGHT,

        N_AUDIO_CLIP,
        N_SOUND,
        N_COLOR_INTERPOLATOR,
        N_COORDINATE_INTERPOLATOR,
        N_ORIENTATION_INTERPOLATOR,
        N_NORMAL_INTERPOLATOR,
        N_POSITION_INTERPOLATOR,
        N_SCALAR_INTERPOLATOR,
        N_PLANE_SENSOR,
        N_PROXIMITY_SENSOR,
        N_SPHERE_SENSOR,
        N_TIME_SENSOR,
        N_TOUCH_SENSOR,
        N_VISIBILITY_SENSOR,

        // Fields
        F_IS,

        F_URL,

        F_GROUND_ANGLE,
        F_GROUND_COLOR,
        F_SKY_ANGLE,
        F_SKY_COLOR,
        F_BACK_URL,
        F_BOTTOM_URL,
        F_FRONT_URL,
        F_LEFT_URL,
        F_RIGHT_URL,
        F_TOP_URL,

        F_AVATAR_SIZE,
        F_HEADLIGHT,
        F_SPEED,
        F_TYPE,
        F_VISIBILITY_LIMIT,

        F_FIELD_OF_VIEW,
        F_JUMP,
        F_ORIENTATION,
        F_POSITION,
        F_DESCRIPTION,

        F_CHILDREN,
        F_ADD_CHILDREN,
        F_REMOVE_CHILDREN,
        F_BBOX_CENTER,
        F_BBOX_SIZE,

        F_CENTER,
        F_ROTATION,
        F_SCALE,
        F_SCALE_ORIENTATION,
        F_TRANSLATION,

        F_APPEARANCE,
        F_GEOMETRY,

        F_MATERIAL,
        F_TEXTURE,
        F_TEXTURE_TRANSFORM,

        F_AMBIENT_INTENSITY,
        F_DIFFUSE_COLOR,
        F_EMISSIVE_COLOR,
        F_SHININESS,
        F_SPECULAR_COLOR,
        F_TRANSPARANCY,

        F_REPEAT_S,
        F_REPEAT_T,

        F_SIZE,

        F_BOTTOM,
        F_BOTTOM_RADIUS,
        F_HEIGHT,
        F_SIDE,

        F_RADIUS,
        F_TOP,

        F_STRING,
        F_FONT_STYLE,
        F_LENGTH,
        F_MAX_EXTENT,

        F_FAMILY,
        F_HORIZONTAL,
        F_JUSTIFY,
        F_LANGUAGE,
        F_LEFT_TO_RIGHT,
        F_SPACING,
        F_STYLE,
        F_TOP_TO_BOTTOM,

        F_COLOR,
        F_COORD,
        F_COLOR_INDEX,
        F_COLOR_PER_VERTEX,
        F_COORD_INDEX,

        F_CCW,
        F_CONVEX,
        F_SOLID,
        F_CREASE_ANGLE,
        F_NORMAL,
        F_NORMAL_INDEX,
        F_NORMAL_PER_VERTEX,
        F_TEX_COORD_INDEX,
        F_TEX_COORD,

        F_POINT,
        F_VECTOR,

        F_BBOXCENTER,
        F_BBOXSIZE,

        F_AUTO_OFFSET,
        F_DISK_ANGLE,
        F_ENABLED,
        F_MAX_ANGLE,
        F_MIN_ANGLE,
        F_OFFSET,

	F_IMAGE,

	F_X_DIMENSION,
	F_Z_DIMENSION,
	F_X_SPACING,
	F_Z_SPACING,

	F_LOOP,
	F_START_TIME,
	F_STOP_TIME,

	F_CROSS_SECTION,
	F_SPINE,
	F_BEGIN_CAP,
	F_END_CAP,

	F_CHOICE,
	F_WHICH_CHOICE,

	F_RANGE,
	F_LEVEL,

	F_COLLIDE,
	F_PROXY,

	F_PARAMETER,

	F_VISIBILITY_RANGE,
	F_FOG_TYPE,

	F_AXIS_OF_ROTATION,

	F_TITLE,
	F_INFO,

	F_LOCATION,
	F_ON,
	F_INTENSITY,
	F_ATTENUATION,
	F_DIRECTION,
	F_BEAM_WIDTH,
	F_CUT_OFF_RANGE,

        // event type
        E_FIELD,
        E_EXPOSED_FIELD,
        E_EVENTIN,
        E_EVENTOUT,

        // def & route
        D_DEF,
        D_USE,
        D_ROUTE,

        // unsupported keywords
        U_SCRIPT,
        U_EXTERNPROTO

    };
}


namespace hrp {

    class VrmlParserImpl
    {
    public:
        VrmlParserImpl(VrmlParser* self);
        VrmlParser* self;

        EasyScannerPtr scanner;
        VrmlProtoInstancePtr currentProtoInstance;

        bool protoInstanceActualNodeExtractionMode;

        typedef map<VrmlProto*, EasyScannerPtr> ProtoToEntityScannerMap;
        ProtoToEntityScannerMap protoToEntityScannerMap;

        typedef map<string, VrmlNodePtr> TDefNodeMap;
        typedef pair<string, VrmlNodePtr> TDefNodePair;
        typedef map<string, VrmlProtoPtr> TProtoMap;
        typedef pair<string, VrmlProtoPtr> TProtoPair;

        TProtoMap protoMap;
        TDefNodeMap defNodeMap;

        void load(const string& filename);
        VrmlNodePtr readSpecificNode(VrmlNodeCategory nodeCategory, int symbol, const std::string& symbolString);
        VrmlNodePtr readInlineNode(VrmlNodeCategory nodeCategory);
        VrmlNodePtr newInlineSource(std::string& io_filename);
        VrmlProtoPtr defineProto();
  
        VrmlNodePtr readNode(VrmlNodeCategory nodeCategory);
        VrmlProtoInstancePtr readProtoInstanceNode(const std::string& proto_name, VrmlNodeCategory nodeCategory);
        VrmlNodePtr evalProtoInstance(VrmlProtoInstancePtr proto, VrmlNodeCategory nodeCategory);
        VrmlUnsupportedNodePtr skipUnsupportedNode(const std::string& nodeTypeName);
        VrmlUnsupportedNodePtr skipScriptNode();
        VrmlUnsupportedNodePtr skipExternProto();

        VrmlViewpointPtr readViewpointNode();
        VrmlNavigationInfoPtr readNavigationInfoNode();
        VrmlBackgroundPtr readBackgroundNode();
        VrmlGroupPtr readGroupNode();
        VrmlTransformPtr readTransformNode();
        VrmlShapePtr readShapeNode();
        VrmlCylinderSensorPtr readCylinderSensorNode();
        VrmlBoxPtr readBoxNode();
        VrmlConePtr readConeNode();
        VrmlCylinderPtr readCylinderNode();

        VrmlPointSetPtr readPointSetNode();
        VrmlPixelTexturePtr readPixelTextureNode();
        VrmlMovieTexturePtr readMovieTextureNode();
        VrmlElevationGridPtr readElevationGridNode();
        VrmlExtrusionPtr readExtrusionNode();
        VrmlSwitchPtr readSwitchNode();
        VrmlLODPtr readLODNode();
        VrmlCollisionPtr readCollisionNode();
        VrmlAnchorPtr readAnchorNode();
        VrmlFogPtr readFogNode();
        VrmlBillboardPtr readBillboardNode();
        VrmlWorldInfoPtr readWorldInfoNode();
        VrmlPointLightPtr readPointLightNode();
        VrmlDirectionalLightPtr	readDirectionalLightNode();
        VrmlSpotLightPtr readSpotLightNode();

        VrmlSpherePtr readSphereNode();
        VrmlTextPtr readTextNode();
        VrmlFontStylePtr readFontStyleNode();
        VrmlIndexedLineSetPtr readIndexedLineSetNode();
        VrmlIndexedFaceSetPtr readIndexedFaceSetNode();
        void checkIndexedFaceSet(VrmlIndexedFaceSetPtr node);
        VrmlCoordinatePtr readCoordNode();
        VrmlTextureCoordinatePtr readTextureCoordinateNode();
        VrmlColorPtr readColorNode();
        VrmlAppearancePtr readAppearanceNode();
        VrmlMaterialPtr readMaterialNode();
        VrmlImageTexturePtr readImageTextureNode();
        VrmlTextureTransformPtr readTextureTransformNode();
        VrmlNormalPtr readNormalNode();
  
        VrmlVariantField& readProtoField(VrmlFieldTypeId fieldTypeId);
  
        void readSFInt32(SFInt32& out_value);
        void readSFFloat(SFFloat& out_value);
        void readSFString(SFString& out_value);
        void readMFInt32(MFInt32& out_value);
        void readMFFloat(MFFloat& out_value);
        void readSFColor(SFColor& out_value); 
        void readMFColor(MFColor& out_value); 
        void readMFString(MFString& out_value);
        void readSFVec2f(SFVec2f& out_value);
        void readMFVec2f(MFVec2f& out_value);
        void readSFVec3f(SFVec3f& out_value);
        void readMFVec3f(MFVec3f& out_value);
        void readSFRotation(SFRotation& out_value);
        void readMFRotation(MFRotation& out_value);
        void readSFBool(SFBool& out_value);
        void readSFTime(SFTime& out_value);
        void readMFTime(MFTime& out_value);
        void readSFNode(SFNode& out_node, VrmlNodeCategory nodeCategory);
        SFNode readSFNode(VrmlNodeCategory nodeCategory);
        void readMFNode(MFNode& out_nodes, VrmlNodeCategory nodeCategory);
        void readSFImage( SFImage& out_image );
    private:
        VrmlParserImpl(const VrmlParserImpl& self, const list< string >& ref);
        const list< string >* getAncestorPathsList() const {return &ancestorPathsList;}
        void setSymbols();
        void init();
        list< string >     ancestorPathsList;
    };
}


VrmlParser::VrmlParser()
{
    init();
}


VrmlParser::VrmlParser(const string& filename)
{
    init();
    load(filename);
}



void VrmlParser::init()
{

    impl = new VrmlParserImpl(this);
}


VrmlParserImpl::VrmlParserImpl(VrmlParser* self) : self(self)
{
    init();
}

VrmlParserImpl::VrmlParserImpl(const VrmlParserImpl& refThis, const list< string >& refSet)
: self(refThis.self), ancestorPathsList(refSet)
{
    init();
}


VrmlParser::~VrmlParser()
{
    delete impl;
}


void VrmlParser::setProtoInstanceActualNodeExtractionMode(bool isOn)
{
    impl->protoInstanceActualNodeExtractionMode = isOn;
}



/**
   This function throws EasyScanner::Exception when an error occurs.
*/
void VrmlParser::load(const string& filename)
{
    impl->load(filename);
}


void VrmlParserImpl::load(const string& filename)
{
    filesystem::path localPath(filename);
    localPath.normalize();

#if (BOOST_VERSION < 104600)
    ancestorPathsList.push_back(localPath.file_string());
    scanner->loadFile(localPath.file_string());
#else
    ancestorPathsList.push_back(localPath.string());
    scanner->loadFile(localPath.string());
#endif
    
    // header check
    scanner->setCommentChar(0);
    bool ok = scanner->readString("#VRML V2.0");
    if(ok){
	    scanner->skipLine();
    }
    
    scanner->setCommentChar('#');
    scanner->setQuoteChar('"');
    scanner->setWhiteSpaceChar(',');
    scanner->setLineOriented(false);
}


VrmlNodePtr VrmlParser::readNode()
{
    return impl->readNode(TOP_NODE);
}


VrmlNodePtr VrmlParserImpl::readNode(VrmlNodeCategory nodeCategory)
{
    VrmlNodePtr node;

    if(scanner->isEOF()){
        if(currentProtoInstance){
            scanner->throwException("Illegal proto instance node");
        }
    }

    if(!scanner->readWord()){
        return 0;
    }

    string def_name;
    string nodeTypeName(scanner->stringValue);
    int symbol = scanner->getSymbolID(scanner->stringValue);

    if(symbol){

        if(symbol==N_INLINE)
            return readInlineNode(nodeCategory);

        if(symbol==N_PROTO){
            if(nodeCategory == TOP_NODE){
                return defineProto();
            } else {
                scanner->throwException("PROTO node cannot be defined here");
            }
        }

        if(symbol==D_USE){
            scanner->readString();
            const string& label = scanner->stringValue;
            TDefNodeMap::iterator p = defNodeMap.find(label);
            if(p != defNodeMap.end()){
                return p->second;
            } else {
                scanner->throwException
                    (string("A node \"") + label + "\" specified by the USE directive does not exist");
            }
        }

        // ROUTE has 3 parameters to skip
        // return as VrmlUnsupportedNode
        if(symbol == D_ROUTE){
            scanner->readString();	// eventOut or exposedField
            if(!scanner->readString("TO"))	// "TO"
                scanner->throwException("Illegal ROUTE (without TO)");
            scanner->readString();	// eventIn or exposedField
            // recursive call to continue reading without node construction
            return readNode( nodeCategory );
        }

        // unsupported keywords
        if(symbol == U_SCRIPT){
            cerr << "Script is not supported. " << endl;
            skipScriptNode();
            return readNode( nodeCategory );
        }
        if(symbol == U_EXTERNPROTO){
            cerr << "ExternProto is not supported." << endl;
            skipExternProto();
            return readNode( nodeCategory );
        }

        if(symbol == D_DEF){
            def_name = scanner->readStringEx("Illegal DEF name");
            scanner->readWord();
            symbol = scanner->getSymbolID(scanner->stringValue);
            nodeTypeName = scanner->stringValue;
        }

    }

    if(!scanner->readChar('{')){
        scanner->throwException
            (string("The entity of a ") + nodeTypeName + " node does not correctly begin with '{'");
    }

    if(symbol){
        node = readSpecificNode(nodeCategory, symbol, nodeTypeName);
    } else {
        node = readProtoInstanceNode(scanner->stringValue, nodeCategory);
    }

    if(!scanner->readChar('}')){
        scanner->throwException
            (string("A ") + nodeTypeName + " node is not correctly closed with '}'");
    }

    if(def_name.size() > 0) {
        defNodeMap.insert(TDefNodePair(def_name, node));
        node->defName = def_name;
    }

    return node;
}


VrmlNodePtr VrmlParserImpl::readSpecificNode(VrmlNodeCategory nodeCategory, int symbol, const string& nodeTypeName)
{
    VrmlNodePtr node;

    switch(symbol){

    case N_BACKGROUND:         node = readBackgroundNode();        break;
    case N_NAVIGATION_INFO:    node = readNavigationInfoNode();    break;
    case N_VIEWPOINT:          node = readViewpointNode();         break;

    case N_GROUP:              node = readGroupNode();             break;
    case N_TRANSFORM:          node = readTransformNode();         break;
    case N_SHAPE:              node = readShapeNode();             break;
    case N_CYLINDER_SENSOR:    node = readCylinderSensorNode();    break;

    case N_POINTSET:           node = readPointSetNode();          break;
    case N_PIXEL_TEXTURE:      node = readPixelTextureNode();      break;
    case N_MOVIE_TEXTURE:      node = readMovieTextureNode();      break;
    case N_ELEVATION_GRID:     node = readElevationGridNode();     break;
    case N_EXTRUSION:          node = readExtrusionNode();         break;
    case N_SWITCH:             node = readSwitchNode();            break;
    case N_LOD:                node = readLODNode();               break;
    case N_COLLISION:          node = readCollisionNode();         break;
    case N_ANCHOR:             node = readAnchorNode();            break;
    case N_FOG:                node = readFogNode();               break;
    case N_BILLBOARD:          node = readBillboardNode();         break;
    case N_WORLD_INFO:         node = readWorldInfoNode();         break;
    case N_POINT_LIGHT:        node = readPointLightNode();        break;
    case N_DIRECTIONAL_LIGHT:  node = readDirectionalLightNode();  break;
    case N_SPOT_LIGHT:         node = readSpotLightNode();         break;

    case N_MATERIAL:           node = readMaterialNode();          break;
    case N_APPEARANCE:         node = readAppearanceNode();        break;
    case N_IMAGE_TEXTURE:      node = readImageTextureNode();      break;
    case N_TEXTURE_TRANSFORM:  node = readTextureTransformNode();  break;

    case N_BOX:                node = readBoxNode();               break;
    case N_CONE:               node = readConeNode();              break;
    case N_CYLINDER:           node = readCylinderNode();          break;
    case N_SPHERE:             node = readSphereNode();            break;
    case N_TEXT:               node = readTextNode();              break;
    case N_INDEXED_FACE_SET:   node = readIndexedFaceSetNode();    break;
    case N_INDEXED_LINE_SET:   node = readIndexedLineSetNode();    break;

    case N_COORDINATE:         node = readCoordNode();             break;
    case N_TEXTURE_COORDINATE: node = readTextureCoordinateNode(); break;
    case N_COLOR:              node = readColorNode();             break;
    case N_NORMAL:             node = readNormalNode();            break;
    case N_FONT_STYLE:         node = readFontStyleNode();         break;

        // unsupported nodes
    case N_AUDIO_CLIP:         node = skipUnsupportedNode("AudioClip");          break;
    case N_SOUND:              node = skipUnsupportedNode("Sound");              break;
    case N_COLOR_INTERPOLATOR: node = skipUnsupportedNode("ColorInterpolator");  break;
    case N_COORDINATE_INTERPOLATOR:  node = skipUnsupportedNode("CoordinateInterpolator");  break;
    case N_ORIENTATION_INTERPOLATOR: node = skipUnsupportedNode("OrientationInterpolator"); break;
    case N_NORMAL_INTERPOLATOR:      node = skipUnsupportedNode("NormalInterpolator");      break;
    case N_POSITION_INTERPOLATOR:    node = skipUnsupportedNode("PositionInterpolator");    break;
    case N_SCALAR_INTERPOLATOR:      node = skipUnsupportedNode("ScalarInterpolator");      break;
    case N_PLANE_SENSOR:       node = skipUnsupportedNode("PlaneSensor");        break;
    case N_PROXIMITY_SENSOR:   node = skipUnsupportedNode("ProximitySensor");    break;
    case N_SPHERE_SENSOR:      node = skipUnsupportedNode("SphereSensor");       break;
    case N_TIME_SENSOR:        node = skipUnsupportedNode("TimeSensor");         break;
    case N_TOUCH_SENSOR:       node = skipUnsupportedNode("TouchSensor");        break;
    case N_VISIBILITY_SENSOR:  node = skipUnsupportedNode("VisibilitySensor");   break;

    default: scanner->throwException
            (string("Node type \"") + nodeTypeName + "\" is not supported");

    }

    if(!node->isCategoryOf(nodeCategory)){
        scanner->throwException
            (string("A ") + nodeTypeName + " node is put in a illegal place");
    }

    return node;
}


VrmlUnsupportedNodePtr VrmlParserImpl::skipUnsupportedNode(const std::string& nodeTypeName)
{
    while(true){
        if(!scanner->readQuotedString()){
            if(scanner->peekChar() == '}'){
                break;
            } if(!scanner->readChar()){
                scanner->throwException( "Node is not closed." );
            }
        }
    }
    return new VrmlUnsupportedNode( nodeTypeName );
}


VrmlUnsupportedNodePtr VrmlParserImpl::skipScriptNode()
{
    // '}' appears twice in "Script" node
    for(int i=0; i<1; i++){
        while(true){
            if(!scanner->readQuotedString()){
                if(scanner->peekChar() == '}'){
                    scanner->readChar();
                    break;
                }
                if(!scanner->readChar()){
                    scanner->throwException( "Script is not closed." );
                }
            }
        }
    }

    //	return new VrmlUnsupportedNode( "Script" );
    return NULL;
}


VrmlUnsupportedNodePtr VrmlParserImpl::skipExternProto()
{
    // read untill ']' appears
    while(true){
        if(!scanner->readQuotedString()){
            if( scanner->peekChar() == ']' ){
                // read found ']' and break this loop
                scanner->readChar();
                break;
            }
            if(!scanner->readChar()){
                scanner->throwException( "EXTERNPROTO is not closed." );
            }
        }
    }
    // read URL after ']'
    SFString url;
    readSFString( url );

    //	return new VrmlUnsupportedNode( "EXTERNPROTO" );
    return NULL;
}


VrmlNodePtr VrmlParserImpl::readInlineNode(VrmlNodeCategory nodeCategory)
{
    scanner->readChar('{');

    if(scanner->readSymbol() && scanner->symbolValue == F_URL){
        MFString    inlineUrls;
        readMFString(inlineUrls);
        scanner->readCharEx('}', "syntax error 2");

        VrmlInlinePtr inlineNode = new VrmlInline();
        for( MFString::iterator ite = inlineUrls.begin(); ite != inlineUrls.end(); ++ite ){
            inlineNode->children.push_back( newInlineSource( *ite ) );
            inlineNode->urls.push_back(*ite);
        }
        return inlineNode;
    }
    return 0;
}


VrmlNodePtr VrmlParserImpl::newInlineSource(string& io_filename)
{
    filesystem::path localPath;
    string chkFile("");
    if( isFileProtocol( io_filename ) )
    {
        localPath = filesystem::path( deleteURLScheme(io_filename) );

        localPath.normalize();
        // Relative path check & translate to absolute path 
        if ( ! exists(localPath) ){

            filesystem::path parentPath( scanner->filename );
#if BOOST_VERSION < 103600
            localPath = parentPath.branch_path() / localPath;
#else
            localPath = parentPath.parent_path() / localPath;
#endif
            localPath.normalize();
        }
        chkFile = complete( localPath ).string();
    } else {
        // Not file protocol implements   
        chkFile = io_filename;
    }
    for( list<string>::const_iterator cIte = ancestorPathsList.begin(); cIte != ancestorPathsList.end(); ++cIte){
        if( chkFile == *cIte )
        {
            scanner->throwException("Infinity loop ! " + chkFile + " is included ancestor list");
        }
    }

    VrmlParserImpl  inlineParser( *this, ancestorPathsList );

    inlineParser.load( chkFile );
    io_filename = chkFile;

    VrmlGroupPtr group = new VrmlGroup();
    while(VrmlNodePtr node = inlineParser.readNode(TOP_NODE)){
        if(node->isCategoryOf(CHILD_NODE)){
            group->children.push_back(node);
        }
    }

    if(group->children.size() == 1){
        return group->children.front();
    } else {
        return group;
    }
}


VrmlProtoPtr VrmlParserImpl::defineProto()
{
    string proto_name = scanner->readWordEx("illegal PROTO name");
    scanner->readCharEx('[', "syntax error 3");

    VrmlProtoPtr proto = new VrmlProto(proto_name);

    while(!scanner->readChar(']')){
        int event_type = scanner->readSymbolEx("illegal field event type");
        int field_symbol = scanner->readSymbolEx("illegal field type");
        string field_name = scanner->readWordEx("syntax error 4");

        // insert a new empty field and get it, contents of which will be set below
        VrmlVariantField& field = proto->fields.insert(TProtoFieldPair(field_name, VrmlVariantField())).first->second;

        switch(event_type){

        case E_FIELD:
        case E_EXPOSED_FIELD:

            switch(field_symbol){

            case T_SFINT32:    field.setType(SFINT32);    readSFInt32(field.sfInt32());         break;
            case T_MFINT32:    field.setType(MFINT32);    readMFInt32(field.mfInt32());         break;
            case T_SFFLOAT:    field.setType(SFFLOAT);    readSFFloat(field.sfFloat());         break;
            case T_MFFLOAT:    field.setType(MFFLOAT);    readMFFloat(field.mfFloat());         break;
            case T_SFVEC3F:    field.setType(SFVEC3F);    readSFVec3f(field.sfVec3f());         break;
            case T_MFVEC3F:    field.setType(MFVEC3F);    readMFVec3f(field.mfVec3f());         break;
            case T_SFCOLOR:    field.setType(SFCOLOR);    readSFColor(field.sfColor());         break;
            case T_MFCOLOR:    field.setType(MFCOLOR);    readMFColor(field.mfColor());         break;
            case T_SFSTRING:   field.setType(SFSTRING);   readSFString(field.sfString());       break;
            case T_MFSTRING:   field.setType(MFSTRING);   readMFString(field.mfString());       break;
            case T_SFROTATION: field.setType(SFROTATION); readSFRotation(field.sfRotation());   break;
            case T_MFROTATION: field.setType(MFROTATION); readMFRotation(field.mfRotation());   break;
            case T_SFBOOL:     field.setType(SFBOOL);     readSFBool(field.sfBool());           break;
            case T_SFNODE:     field.setType(SFNODE);     readSFNode(field.sfNode(), ANY_NODE); break;
            case T_MFNODE:     field.setType(MFNODE);     readMFNode(field.mfNode(), ANY_NODE); break;
            case T_SFIMAGE:    field.setType(SFIMAGE);    readSFImage(field.sfImage());         break;


            default: scanner->throwException("illegal field type");
            }
            break;

        case E_EVENTIN:
        case E_EVENTOUT:

            switch(field_symbol){

            case T_SFINT32:    field.setType(SFINT32);    break;
            case T_MFINT32:    field.setType(MFINT32);    break;
            case T_SFFLOAT:    field.setType(SFFLOAT);    break;
            case T_MFFLOAT:    field.setType(MFFLOAT);    break;
            case T_SFVEC3F:    field.setType(SFVEC3F);    break;
            case T_MFVEC3F:    field.setType(MFVEC3F);    break;
            case T_SFCOLOR:    field.setType(SFCOLOR);    break;
            case T_MFCOLOR:    field.setType(MFCOLOR);    break;
            case T_SFSTRING:   field.setType(SFSTRING);   break;
            case T_MFSTRING:   field.setType(MFSTRING);   break;
            case T_SFROTATION: field.setType(SFROTATION); break;
            case T_MFROTATION: field.setType(MFROTATION); break;
            case T_SFBOOL:     field.setType(SFBOOL);     break;
            case T_SFNODE:     field.setType(SFNODE);     break;
            case T_MFNODE:     field.setType(MFNODE);     break;
            case T_SFIMAGE:    field.setType(SFIMAGE);    break;
            }
            break;

        default: scanner->throwException("illegal field event type");

        }
    }

    scanner->readCharEx('{', "A PROTO definition has no entity");
    char* begin = scanner->text;
    int brace_level = 1;
    while(true){
        int token = scanner->readToken();
        if(token == EasyScanner::T_NONE){
            scanner->throwException("syntax error 5");
        } else if(token == EasyScanner::T_SIGLUM){
            if(scanner->charValue == '{') {
                brace_level++;
            } else if(scanner->charValue == '}') {
                brace_level--;
                if(brace_level==0)
                    break;
            }
        }
    }

    EasyScannerPtr entityScanner(new EasyScanner(*scanner, false));
    entityScanner->setText(begin, scanner->text - begin - 1);
    entityScanner->setLineNumberOffset(scanner->lineNumber);

    protoToEntityScannerMap[proto.get()] = entityScanner;

    protoMap.insert(TProtoPair(proto_name, proto));

    return proto;
}


VrmlProtoInstancePtr VrmlParserImpl::readProtoInstanceNode(const string& proto_name, VrmlNodeCategory nodeCategory)
{
    TProtoMap::iterator p = protoMap.find(proto_name);
    if(p == protoMap.end()){
        scanner->throwException("undefined node");
    }

    VrmlProtoPtr proto = p->second;
    VrmlProtoInstancePtr protoInstance(new VrmlProtoInstance(proto));

    while(scanner->readWord()){
        TProtoFieldMap::iterator p = protoInstance->fields.find(scanner->stringValue);
        if(p == protoInstance->fields.end())
            scanner->throwException("undefined field");

        VrmlVariantField& field = p->second;

        switch(field.typeId()){

        case SFINT32:    readSFInt32(field.sfInt32());        break;
        case MFINT32:    readMFInt32(field.mfInt32());        break;
        case SFFLOAT:    readSFFloat(field.sfFloat());        break;
        case MFFLOAT:    readMFFloat(field.mfFloat());        break;
        case SFVEC2F:    readSFVec2f(field.sfVec2f());        break;
        case MFVEC2F:    readMFVec2f(field.mfVec2f());        break;
        case SFVEC3F:    readSFVec3f(field.sfVec3f());        break;
        case MFVEC3F:    readMFVec3f(field.mfVec3f());        break;
        case SFCOLOR:    readSFVec3f(field.sfColor());        break;
        case MFCOLOR:    readMFVec3f(field.mfColor());        break;
        case SFSTRING:   readSFString(field.sfString());      break;
        case MFSTRING:   readMFString(field.mfString());      break;
        case SFROTATION: readSFRotation(field.sfRotation());  break;
        case MFROTATION: readMFRotation(field.mfRotation());  break;
        case SFBOOL:     readSFBool(field.sfBool());           break;
        case SFNODE:     readSFNode(field.sfNode(), ANY_NODE); break;
        case MFNODE:     readMFNode(field.mfNode(), ANY_NODE); break;
        case SFIMAGE:    readSFImage(field.sfImage());         break;
        default:
            break;
        }
    }

    if(protoInstanceActualNodeExtractionMode){
        protoInstance->actualNode = evalProtoInstance(protoInstance, nodeCategory);
    }

    return protoInstance;
}


VrmlNodePtr VrmlParserImpl::evalProtoInstance(VrmlProtoInstancePtr protoInstance, VrmlNodeCategory nodeCategory)
{
    EasyScannerPtr orgScanner = scanner;
    ProtoToEntityScannerMap::iterator p;
    p = protoToEntityScannerMap.find(protoInstance->proto.get());
    if(p == protoToEntityScannerMap.end()){
        scanner->throwException("Undefined proto node instance");
    }
    scanner = p->second;
    scanner->moveToHead();

    VrmlProtoInstancePtr orgProtoInstance = currentProtoInstance;
    currentProtoInstance = protoInstance;
    
    VrmlNodePtr node = readNode(nodeCategory);
    node->defName = protoInstance->defName;
    
    scanner = orgScanner;
    currentProtoInstance = orgProtoInstance;
    
    return node;
}


VrmlViewpointPtr VrmlParserImpl::readViewpointNode()
{
    VrmlViewpointPtr node(new VrmlViewpoint);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_FIELD_OF_VIEW: readSFFloat(node->fieldOfView);    break;
        case F_JUMP:          readSFBool(node->jump);            break;
        case F_ORIENTATION:   readSFRotation(node->orientation); break;
        case F_POSITION:      readSFVec3f(node->position);       break;
        case F_DESCRIPTION:   readSFString(node->description);   break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlNavigationInfoPtr VrmlParserImpl::readNavigationInfoNode()
{
    VrmlNavigationInfoPtr node(new VrmlNavigationInfo);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_AVATAR_SIZE:      readMFFloat(node->avatarSize);         break;
        case F_HEADLIGHT:        readSFBool(node->headlight);        break;
        case F_SPEED:            readSFFloat(node->speed);           break;
        case F_TYPE:             readMFString(node->type);           break;
        case F_VISIBILITY_LIMIT: readSFFloat(node->visibilityLimit); break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlBackgroundPtr VrmlParserImpl::readBackgroundNode()
{
    VrmlBackgroundPtr node(new VrmlBackground);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_GROUND_ANGLE: readMFFloat(node->groundAngle); break;
        case F_GROUND_COLOR: readMFColor(node->groundColor); break;
        case F_SKY_ANGLE:    readMFFloat(node->skyAngle);    break;
        case F_SKY_COLOR:    readMFColor(node->skyColor);    break;
        case F_BACK_URL:     readMFString(node->backUrl);    break;
        case F_BOTTOM_URL:   readMFString(node->bottomUrl);  break;
        case F_FRONT_URL:    readMFString(node->frontUrl);   break;
        case F_LEFT_URL:     readMFString(node->leftUrl);    break;
        case F_RIGHT_URL:    readMFString(node->rightUrl);   break;
        case F_TOP_URL:      readMFString(node->topUrl);     break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlGroupPtr VrmlParserImpl::readGroupNode()
{
    VrmlGroupPtr node(new VrmlGroup);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_BBOX_CENTER:     readSFVec3f(node->bboxCenter);          break;
        case F_BBOX_SIZE:       readSFVec3f(node->bboxSize);            break;
        case F_CHILDREN:        readMFNode(node->children, CHILD_NODE); break;

        case F_ADD_CHILDREN:
        case F_REMOVE_CHILDREN:
        {
            MFNode dummy;
            readMFNode(dummy, CHILD_NODE);
        }
        break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlTransformPtr VrmlParserImpl::readTransformNode()
{
    VrmlTransformPtr node(new VrmlTransform);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_CENTER:            readSFVec3f(node->center);              break;
        case F_ROTATION:          readSFRotation(node->rotation);         break;
        case F_SCALE:             readSFVec3f(node->scale);               break;
        case F_SCALE_ORIENTATION: readSFRotation(node->scaleOrientation); break;
        case F_TRANSLATION:       readSFVec3f(node->translation);         break;
        case F_BBOX_CENTER:       readSFVec3f(node->bboxCenter);          break;
        case F_BBOX_SIZE:         readSFVec3f(node->bboxSize);            break;

        case F_CHILDREN:          readMFNode(node->children, CHILD_NODE); break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlShapePtr VrmlParserImpl::readShapeNode()
{
    VrmlShapePtr node(new VrmlShape);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_APPEARANCE: node->appearance = dynamic_pointer_cast<VrmlAppearance>(readSFNode(APPEARANCE_NODE)); break;
        case F_GEOMETRY:   node->geometry = readSFNode(GEOMETRY_NODE);       break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlCylinderSensorPtr VrmlParserImpl::readCylinderSensorNode()
{
    VrmlCylinderSensorPtr node(new VrmlCylinderSensor);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_AUTO_OFFSET: readSFBool(node->autoOffset); break;
        case F_DISK_ANGLE:  readSFFloat(node->diskAngle); break;
        case F_ENABLED:     readSFBool(node->enabled);    break;
        case F_MAX_ANGLE:   readSFFloat(node->maxAngle);  break;
        case F_MIN_ANGLE:   readSFFloat(node->minAngle);  break;
        case F_OFFSET:      readSFFloat(node->offset);    break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlPointSetPtr VrmlParserImpl::readPointSetNode()
{
    VrmlPointSetPtr node(new VrmlPointSet);

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_COORD:
            node->coord = dynamic_pointer_cast<VrmlCoordinate>( readSFNode( COORDINATE_NODE ) );
            break;
            
        case F_COLOR:
            node->color = dynamic_pointer_cast<VrmlColor>( readSFNode( COLOR_NODE ) );
            break;
            
        default:
            scanner->throwException( "Undefined field" );
        }
    }

    return node;
}


VrmlPixelTexturePtr VrmlParserImpl::readPixelTextureNode()
{
    VrmlPixelTexturePtr node(new VrmlPixelTexture);

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_IMAGE:
            readSFImage( node->image );
            break;
            
        case F_REPEAT_S:
            readSFBool( node->repeatS );
            break;
            
        case F_REPEAT_T:
            readSFBool( node->repeatT );
            break;
            
        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlMovieTexturePtr VrmlParserImpl::readMovieTextureNode()
{
    VrmlMovieTexturePtr node(new VrmlMovieTexture);

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_URL:
            readMFString( node->url );
            break;
            
        case F_LOOP:
            readSFBool( node->loop );
            break;
            
        case F_SPEED:
            readSFFloat( node->speed );
            break;
            
        case F_START_TIME:
            readSFTime( node->startTime );
            break;
            
        case F_STOP_TIME:
            readSFTime( node->stopTime );
            break;
            
        case F_REPEAT_S:
            readSFBool( node->repeatS );
            break;
            
        case F_REPEAT_T:
            readSFBool( node->repeatT );
            break;
            
        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlElevationGridPtr VrmlParserImpl::readElevationGridNode()
{
    VrmlElevationGridPtr node(new VrmlElevationGrid);

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_X_DIMENSION:
            readSFInt32( node->xDimension );
            break;
            
        case F_Z_DIMENSION:
            readSFInt32( node->zDimension );
            break;
            
        case F_X_SPACING:
            readSFFloat( node->xSpacing );
            break;
            
        case F_Z_SPACING:
            readSFFloat( node->zSpacing );
            break;
            
        case F_HEIGHT:
            readMFFloat( node->height );
            break;
            
        case F_CCW:
            readSFBool( node->ccw );
            break;
            
        case F_COLOR_PER_VERTEX:
            readSFBool( node->colorPerVertex );
            break;
            
        case F_CREASE_ANGLE:
            readSFFloat( node->creaseAngle );
            break;
            
        case F_NORMAL_PER_VERTEX:
            readSFBool( node->normalPerVertex );
            break;
            
        case F_SOLID:
            readSFBool( node->solid );
            break;
            
        case F_COLOR:
            node->color = dynamic_pointer_cast<VrmlColor>( readSFNode( COLOR_NODE ) );
            break;
            
        case F_NORMAL:
            node->normal = dynamic_pointer_cast<VrmlNormal>( readSFNode( NORMAL_NODE ) );
            break;
            
        case F_TEX_COORD:
            node->texCoord = dynamic_pointer_cast<VrmlTextureCoordinate>( readSFNode( TEXTURE_COORDINATE_NODE ) );
            break;
            
        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlExtrusionPtr VrmlParserImpl::readExtrusionNode()
{
    VrmlExtrusionPtr node( new VrmlExtrusion );

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_CROSS_SECTION:
            readMFVec2f( node->crossSection );
            break;
            
        case F_SPINE:
            readMFVec3f( node->spine );
            break;
            
        case F_SCALE:
            readMFVec2f( node->scale );
            break;
            
        case F_ORIENTATION:
            readMFRotation( node->orientation );
            break;
            
        case F_BEGIN_CAP:
            readSFBool( node->beginCap );
            break;
            
        case F_END_CAP:
            readSFBool( node->endCap );
            break;
            
        case F_SOLID:
            readSFBool( node->solid );
            break;
            
        case F_CCW:
            readSFBool( node->ccw );
            break;
            
        case F_CONVEX:
            readSFBool( node->convex );
            break;
            
        case F_CREASE_ANGLE:
            readSFFloat( node->creaseAngle );
            break;
            
        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlSwitchPtr VrmlParserImpl::readSwitchNode()
{
    VrmlSwitchPtr node( new VrmlSwitch );

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_CHOICE:
            readMFNode(node->choice, CHILD_NODE);
            break;
            
        case F_WHICH_CHOICE:
            readSFInt32(node->whichChoice);
            break;
            
        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlLODPtr VrmlParserImpl::readLODNode()
{
    VrmlLODPtr node( new VrmlLOD );

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_RANGE:
            readMFFloat(node->range);
            break;

        case F_CENTER:
            readSFVec3f(node->center);
            break;

        case F_LEVEL:
            readMFNode(node->level, ANY_NODE);
            break;

        default:
            scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlCollisionPtr VrmlParserImpl::readCollisionNode()
{
    VrmlCollisionPtr node(new VrmlCollision);

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_COLLIDE:
            readSFBool(node->collide);
            break;

        case F_CHILDREN:
            readMFNode(node->children, CHILD_NODE);
            break;

        case F_PROXY:
            readSFNode(node->proxy, SHAPE_NODE);
            break;
            
        case F_BBOX_CENTER:
            readSFVec3f(node->bboxCenter);
            break;
            
        case F_BBOX_SIZE:
            readSFVec3f(node->bboxSize);
            break;
            
        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlAnchorPtr VrmlParserImpl::readAnchorNode()
{
    VrmlAnchorPtr node(new VrmlAnchor);

    while(scanner->readSymbol()){
        switch( scanner->symbolValue){
        case F_CHILDREN:
            readMFNode(node->children, CHILD_NODE);
            break;
            
        case F_DESCRIPTION:
            readSFString(node->description);
            break;
            
        case F_PARAMETER:
            readMFString(node->parameter);
            break;
            
        case F_URL:
            readMFString(node->url);
            break;
            
        case F_BBOX_CENTER:
            readSFVec3f(node->bboxCenter);
            break;
            
        case F_BBOX_SIZE:
            readSFVec3f(node->bboxSize);
            break;

        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlFogPtr VrmlParserImpl::readFogNode()
{
    VrmlFogPtr node(new VrmlFog);

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_COLOR:
            readSFColor(node->color);
            break;
            
        case F_VISIBILITY_RANGE:
            readSFFloat(node->visibilityRange);
            break;
            
        case F_FOG_TYPE:
            readSFString(node->fogType);
                    break;
                    
        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlBillboardPtr VrmlParserImpl::readBillboardNode()
{
    VrmlBillboardPtr node( new VrmlBillboard );

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_AXIS_OF_ROTATION:
            readSFVec3f(node->axisOfRotation);
            break;

        case F_CHILDREN:
            readMFNode(node->children, CHILD_NODE);
            break;

        case F_BBOX_CENTER:
            readSFVec3f(node->bboxCenter);
            break;

        case F_BBOX_SIZE:
            readSFVec3f(node->bboxSize);
            break;

        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlWorldInfoPtr VrmlParserImpl::readWorldInfoNode()
{
    VrmlWorldInfoPtr node(new VrmlWorldInfo);

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_TITLE:
            readSFString(node->title);
            break;

        case F_INFO:
            readMFString(node->info);
            break;

        default:
            scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlPointLightPtr VrmlParserImpl::readPointLightNode()
{
    VrmlPointLightPtr node( new VrmlPointLight );

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_LOCATION:
            readSFVec3f(node->location);
            break;

        case F_ON:
            readSFBool(node->on);
            break;

        case F_INTENSITY:
            readSFFloat(node->intensity);
            break;
            
        case F_COLOR:
            readSFColor(node->color);
            break;
            
        case F_RADIUS:
            readSFFloat(node->radius);
            break;
            
        case F_AMBIENT_INTENSITY:
            readSFFloat(node->ambientIntensity);
            break;
            
        case F_ATTENUATION:
            readSFVec3f(node->attenuation);
            break;

        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}



VrmlDirectionalLightPtr VrmlParserImpl::readDirectionalLightNode()
{
    VrmlDirectionalLightPtr node(new VrmlDirectionalLight);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){
            
        case F_DIRECTION:         readSFVec3f(node->direction);        break;
        case F_ON:                readSFBool(node->on);                break;
        case F_INTENSITY:         readSFFloat(node->intensity);        break;
        case F_COLOR:             readSFColor(node->color);            break;
        case F_AMBIENT_INTENSITY: readSFFloat(node->ambientIntensity); break;
            
        default: scanner->throwException("Undefined field");
        }
    }
    
    return node;
}


VrmlSpotLightPtr VrmlParserImpl::readSpotLightNode()
{
    VrmlSpotLightPtr node(new VrmlSpotLight);

    while(scanner->readSymbol()){
        switch(scanner->symbolValue){
        case F_LOCATION:
            readSFVec3f(node->location);
            break;
            
        case F_DIRECTION:
            readSFVec3f(node->direction);
            break;
            
        case F_ON:
            readSFBool(node->on);
            break;
            
        case F_COLOR:
            readSFColor(node->color);
            break;
            
        case F_INTENSITY:
            readSFFloat(node->intensity);
            break;
            
        case F_RADIUS:
            readSFFloat(node->radius);
            break;
            
        case F_AMBIENT_INTENSITY:
            readSFFloat(node->ambientIntensity);
            break;
            
        case F_ATTENUATION:
            readSFVec3f(node->attenuation);
            break;
            
        case F_BEAM_WIDTH:
            readSFFloat(node->beamWidth);
            break;
            
        case F_CUT_OFF_RANGE:
            readSFFloat(node->cutOffAngle);
            break;
            
        default:
            scanner->throwException( "Undefined field" );
        }
    }
    
    return node;
}


VrmlBoxPtr VrmlParserImpl::readBoxNode()
{
    VrmlBoxPtr node(new VrmlBox);

    if(scanner->readSymbol()){
        if(scanner->symbolValue != F_SIZE)
            scanner->throwException("Undefined field");
        readSFVec3f(node->size);
    }

    return node;
}


VrmlConePtr VrmlParserImpl::readConeNode()
{
    VrmlConePtr node(new VrmlCone);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_BOTTOM:        readSFBool(node->bottom);        break;
        case F_BOTTOM_RADIUS: readSFFloat(node->bottomRadius); break;
        case F_HEIGHT:        readSFFloat(node->height);       break;
        case F_SIDE:          readSFBool(node->side);          break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlCylinderPtr VrmlParserImpl::readCylinderNode()
{
    VrmlCylinderPtr node(new VrmlCylinder);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_BOTTOM: readSFBool(node->bottom);  break;
        case F_HEIGHT: readSFFloat(node->height); break;
        case F_RADIUS: readSFFloat(node->radius); break;
        case F_SIDE:   readSFBool(node->side);    break;
        case F_TOP:    readSFBool(node->top);     break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlSpherePtr VrmlParserImpl::readSphereNode()
{
    VrmlSpherePtr node(new VrmlSphere);

    if(scanner->readSymbol()){
        if(scanner->symbolValue != F_RADIUS)
            scanner->throwException("Undefined field");
        readSFFloat(node->radius);
    }

    return node;
}


VrmlTextPtr VrmlParserImpl::readTextNode()
{
    VrmlTextPtr node(new VrmlText);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_STRING:     readMFString(node->fstring);  break;
        case F_LENGTH:     readMFFloat(node->length);    break;
        case F_MAX_EXTENT: readSFFloat(node->maxExtent); break;
        case F_FONT_STYLE: node->fontStyle = dynamic_pointer_cast<VrmlFontStyle>(readSFNode(FONT_STYLE_NODE)); break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlFontStylePtr VrmlParserImpl::readFontStyleNode()
{
    VrmlFontStylePtr node(new VrmlFontStyle);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_FAMILY:        readMFString(node->family);    break;
        case F_HORIZONTAL:    readSFBool(node->horizontal);  break;
        case F_JUSTIFY:       readMFString(node->justify);   break;
        case F_LANGUAGE:      readSFString(node->language);  break;
        case F_LEFT_TO_RIGHT: readSFBool(node->leftToRight); break;
        case F_SIZE:          readSFFloat(node->size);       break;
        case F_SPACING:       readSFFloat(node->spacing);    break;
        case F_STYLE:         readSFString(node->style);     break;
        case F_TOP_TO_BOTTOM: readSFBool(node->topToBottom); break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlIndexedLineSetPtr VrmlParserImpl::readIndexedLineSetNode()
{
    VrmlIndexedLineSetPtr node(new VrmlIndexedLineSet);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_COLOR: node->color = dynamic_pointer_cast<VrmlColor>(readSFNode(COLOR_NODE)); break;
        case F_COORD: node->coord = dynamic_pointer_cast<VrmlCoordinate>(readSFNode(COORDINATE_NODE)); break;
        case F_COLOR_INDEX:      readMFInt32(node->colorIndex);    break;
        case F_COLOR_PER_VERTEX: readSFBool(node->colorPerVertex); break;
        case F_COORD_INDEX:      readMFInt32(node->coordIndex);    break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlIndexedFaceSetPtr VrmlParserImpl::readIndexedFaceSetNode()
{
    VrmlIndexedFaceSetPtr node(new VrmlIndexedFaceSet);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_COLOR: node->color = dynamic_pointer_cast<VrmlColor>(readSFNode(COLOR_NODE)); break;
        case F_COORD: node->coord = dynamic_pointer_cast<VrmlCoordinate>(readSFNode(COORDINATE_NODE)); break;

        case F_COLOR_INDEX:       readMFInt32(node->colorIndex);    break;
        case F_COLOR_PER_VERTEX:  readSFBool(node->colorPerVertex); break;
        case F_COORD_INDEX:       readMFInt32(node->coordIndex);    break;

        case F_CCW:               readSFBool(node->ccw);              break;
        case F_CONVEX:            readSFBool(node->convex);           break;
        case F_SOLID:             readSFBool(node->solid);            break;
        case F_CREASE_ANGLE:      readSFFloat(node->creaseAngle);     break;
        case F_NORMAL_INDEX:      readMFInt32(node->normalIndex);     break;
        case F_NORMAL_PER_VERTEX: readSFBool(node->normalPerVertex);  break;
        case F_TEX_COORD_INDEX:   readMFInt32(node->texCoordIndex);   break;

        case F_TEX_COORD:
            node->texCoord = dynamic_pointer_cast<VrmlTextureCoordinate>(readSFNode(TEXTURE_COORDINATE_NODE));
            break;
        case F_NORMAL:
            node->normal = dynamic_pointer_cast<VrmlNormal>(readSFNode(NORMAL_NODE));
            break;

        default: scanner->throwException("Undefined field");
        }
    }

    //checkIndexedFaceSet(node);

    return node;
}


void VrmlParserImpl::checkIndexedFaceSet(VrmlIndexedFaceSetPtr node)
{
    MFInt32& index = node->coordIndex;
    MFVec3f& coord = node->coord->point;

    int numUsedVertices = 0;
    vector<bool> usedVertices(coord.size(), false);

    int n = index.size();

    int i = 0;
    vector<int> polygon;
    while(i < n){
        polygon.resize(0);
        bool isSeparated = false;
        while(i < n){
            if(index[i] < 0){
                isSeparated = true;
                i++;
                break;
            }
            polygon.push_back(index[i]);
            if(!usedVertices[index[i]]){
                usedVertices[index[i]] = true;
                numUsedVertices++;
            }
            i++;
        }

        const int numVertices = polygon.size();

        if(numVertices < 3){
            cerr << "Number of vertices is less than 3 !" << endl;
        }
        if(numVertices > 3){
            cerr << "Polygon is not a triangle in ";
            cerr << scanner->filename << endl;
            for(int j=0; j < numVertices; j++){
                cerr << polygon[j] << ",";
            }
            cerr << endl;
        }
        if(!isSeparated){
            cerr << "Vertex index is not correctly separated by '-1'" << endl;
        }

        int n = coord.size();
        for(int j=0; j < numVertices; j++){
            if(polygon[j] >= n){
                cerr << "index " << polygon[j] << " is over the number of vertices" << endl;
            }
        }

        bool isIndexOverlapped = false;
        bool isVertexOverlapped = false;
        for(int j = 0; j < numVertices - 1; j++){
            for(int k = j+1; k < numVertices; k++){
                if(polygon[j] == polygon[k]){
                    isIndexOverlapped = true;
                }
                SFVec3f& v1 = coord[polygon[j]];
                SFVec3f& v2 = coord[polygon[k]];
                if(v1[0] == v2[0] && v1[1] == v2[1] && v1[2] == v2[2]){
                    isVertexOverlapped = true;
                }
            }
        }
        if(isIndexOverlapped){
            cerr << "overlapped vertex index in one polygon: ";
            for(int l = 0; l < numVertices; l++){
                cerr << polygon[l] << ",";
            }
            cerr << endl;
        }

        if(isVertexOverlapped){
            cerr << "In " << scanner->filename << ":";
            cerr << "two vertices in one polygon have the same position\n";

            for(int l = 0; l < numVertices; l++){
                SFVec3f& v = coord[polygon[l]];
                cerr << polygon[l] << " = (" << v[0] << "," << v[1] << "," << v[2] << ") ";
            }
            cerr << endl;
        }
    }

    if(numUsedVertices < static_cast<int>(coord.size())){
        cerr << "There are vertices which are not used in" << scanner->filename << ".\n";
        cerr << "Number of vertices is " << coord.size();
        cerr << ", Number of used ones is " << numUsedVertices << endl;
    }
}


VrmlCoordinatePtr VrmlParserImpl::readCoordNode()
{
    VrmlCoordinatePtr node(new VrmlCoordinate);

    if(scanner->readSymbol()){
        if(scanner->symbolValue != F_POINT)
            scanner->throwException("Undefined field");
        readMFVec3f(node->point);
    }

    return node;
}


VrmlTextureCoordinatePtr VrmlParserImpl::readTextureCoordinateNode()
{
    VrmlTextureCoordinatePtr node(new VrmlTextureCoordinate);

    if(scanner->readSymbol()){
        if(scanner->symbolValue != F_POINT)
            scanner->throwException("Undefined field");
        readMFVec2f(node->point);
    }

    return node;
}


VrmlColorPtr VrmlParserImpl::readColorNode()
{
    VrmlColorPtr node(new VrmlColor);

    if(scanner->readSymbol()){
        if(scanner->symbolValue != F_COLOR)
            scanner->throwException("Undefined field");
        readMFColor(node->color);
    }

    return node;
}


VrmlNormalPtr VrmlParserImpl::readNormalNode()
{
    VrmlNormalPtr node(new VrmlNormal);

    if(scanner->readSymbol()){
        if(scanner->symbolValue != F_VECTOR)
            scanner->throwException("Undefined field");
        readMFVec3f(node->vector);
    }

    return node;
}


VrmlAppearancePtr VrmlParserImpl::readAppearanceNode()
{
    VrmlAppearancePtr node(new VrmlAppearance);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_MATERIAL: node->material = dynamic_pointer_cast<VrmlMaterial>(readSFNode(MATERIAL_NODE)); break;
        case F_TEXTURE:  node->texture = dynamic_pointer_cast<VrmlTexture>(readSFNode(TEXTURE_NODE)); break;
        case F_TEXTURE_TRANSFORM:
            node->textureTransform = dynamic_pointer_cast<VrmlTextureTransform>(readSFNode(TEXTURE_TRANSFORM_NODE)); break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlMaterialPtr VrmlParserImpl::readMaterialNode()
{
    VrmlMaterialPtr node(new VrmlMaterial);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){

        case F_AMBIENT_INTENSITY: readSFFloat(node->ambientIntensity); break;
        case F_DIFFUSE_COLOR:     readSFColor(node->diffuseColor);     break;
        case F_EMISSIVE_COLOR:    readSFColor(node->emissiveColor);    break;
        case F_SHININESS:         readSFFloat(node->shininess);        break;
        case F_SPECULAR_COLOR:    readSFColor(node->specularColor);    break;
        case F_TRANSPARANCY:      readSFFloat(node->transparency);     break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlImageTexturePtr VrmlParserImpl::readImageTextureNode()
{
    VrmlImageTexturePtr node = new VrmlImageTexture;

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){
        case F_URL:      readMFString(node->url);   break;
        case F_REPEAT_S: readSFBool(node->repeatS); break;
        case F_REPEAT_T: readSFBool(node->repeatT); break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlTextureTransformPtr VrmlParserImpl::readTextureTransformNode()
{
    VrmlTextureTransformPtr node(new VrmlTextureTransform);

    while(scanner->readSymbol()){

        switch(scanner->symbolValue){
        case F_CENTER:      readSFVec2f(node->center);      break;
        case F_ROTATION:    readSFFloat(node->rotation);    break;
        case F_SCALE:       readSFVec2f(node->scale);       break;
        case F_TRANSLATION: readSFVec2f(node->translation); break;

        default: scanner->throwException("Undefined field");
        }
    }

    return node;
}


VrmlVariantField& VrmlParserImpl::readProtoField(VrmlFieldTypeId fieldTypeId)
{
    if(!currentProtoInstance){
        scanner->throwException("cannot use proto field value here");
    }
    scanner->readWordEx("illegal field");
    TProtoFieldMap::iterator p = currentProtoInstance->fields.find(scanner->stringValue);

    if(p == currentProtoInstance->fields.end()){
        string msg = "This field(";
        msg += scanner->stringValue +") does not exist in proto node";
        scanner->throwException(msg);
    }
    if(p->second.typeId() != fieldTypeId){
        scanner->throwException("Unmatched field type");
    }

    return p->second;
}


void VrmlParserImpl::readSFInt32(SFInt32& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFINT32);
        out_value = field.sfInt32();
    } else {
        out_value = scanner->readIntEx("illegal int value");
    }
}


void VrmlParserImpl::readMFInt32(MFInt32& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(MFINT32);
        out_value = field.mfInt32();
    } else {
        int v;
        out_value.clear();
        if(!scanner->readChar('[')){
            readSFInt32(v);
            out_value.push_back(v);
        } else {
            while(!scanner->readChar(']')){
                readSFInt32(v);
                out_value.push_back(v);
            }
        }
    }
}


void VrmlParserImpl::readSFFloat(SFFloat& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFFLOAT);
        out_value = field.sfFloat();
    } else {
        out_value = scanner->readDoubleEx("illegal float value");
    }
}


void VrmlParserImpl::readMFFloat(MFFloat& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(MFFLOAT);
        out_value = field.mfFloat();
    } else {
        SFFloat v;
        out_value.clear();
        if(!scanner->readChar('[')){
            readSFFloat(v);
            out_value.push_back(v);
        } else {
            while(!scanner->readChar(']')){
                readSFFloat(v);
                out_value.push_back(v);
            }
        }
    }
}


void VrmlParserImpl::readSFString(SFString& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFSTRING);
        out_value = field.sfString();
    } else {
        out_value = scanner->readQuotedStringEx("illegal string");
    }
}


void VrmlParserImpl::readMFString(MFString& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(MFSTRING);
        out_value = field.mfString();
    } else {
        string s;
        out_value.clear();
        if(!scanner->readChar('[')){
            readSFString(s);
            out_value.push_back(s);
        } else {
            while(!scanner->readChar(']')){
                readSFString(s);
                out_value.push_back(s);
            }
        }
    }
}


void VrmlParserImpl::readSFVec2f(SFVec2f& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFVEC2F);
        out_value = field.sfVec2f();
    } else {
        readSFFloat(out_value[0]);
        readSFFloat(out_value[1]);
    }
}


void VrmlParserImpl::readMFVec2f(MFVec2f& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(MFVEC2F);
        out_value = field.mfVec2f();
    } else {
        SFVec2f v;
        out_value.clear();
        if(!scanner->readChar('[')){
            readSFVec2f(v);
            out_value.push_back(v);
        } else {
            while(!scanner->readChar(']')){
                readSFVec2f(v);
                out_value.push_back(v);
            }
        }
    }
}


void VrmlParserImpl::readSFVec3f(SFVec3f& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFVEC3F);
        out_value = field.sfVec3f();
    } else {
        readSFFloat(out_value[0]);
        readSFFloat(out_value[1]);
        readSFFloat(out_value[2]);
    }
}


void VrmlParserImpl::readMFVec3f(MFVec3f& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(MFVEC3F);
        out_value = field.mfVec3f();
    } else {
        SFVec3f v;
        out_value.clear();
        if(!scanner->readChar('[')){
            readSFVec3f(v);
            out_value.push_back(v);
        } else {
            while(!scanner->readChar(']')){
                readSFVec3f(v);
                out_value.push_back(v);
            }
        }
    }
}


void VrmlParserImpl::readSFColor(SFColor& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFCOLOR);
        out_value = field.sfColor();
    } else {
        readSFVec3f(out_value);
    }
}



void VrmlParserImpl::readMFColor(MFColor& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(MFCOLOR);
        out_value = field.mfColor();
    } else {
        readMFVec3f(out_value);
    }
}


void VrmlParserImpl::readSFRotation(SFRotation& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFROTATION);
        out_value = field.sfRotation();
    } else {
        double len2 = 0.0;
        // axis
        for(int i=0; i < 3; i++){
            readSFFloat(out_value[i]);
            len2 += out_value[i] * out_value[i];
        }
        readSFFloat(out_value[3]); // angle

        const double len = sqrt(len2);

        if(fabs(len) < 1.0e-6){
            scanner->throwException("Rotation axis is zero vector");
        }

        // force normalize
        for(int i=0; i < 3; i++){
            out_value[i] /= len;
        }
    }
}


void VrmlParserImpl::readMFRotation(MFRotation& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(MFROTATION);
        out_value = field.mfRotation();
    } else {
        SFRotation r;
        out_value.clear();
        if(!scanner->readChar('[')){
            readSFRotation(r);
            out_value.push_back(r);
        } else {
            while(!scanner->readChar(']')){
                readSFRotation(r);
                out_value.push_back(r);
            }
        }
    }
}


void VrmlParserImpl::readSFBool(SFBool& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFBOOL);
        out_value = field.sfBool();
    } else {
        switch(scanner->readSymbolEx("no bool value")){
        case V_TRUE:  out_value = true;  break;
        case V_FALSE: out_value = false; break;

        default: scanner->throwException("no bool value");
        }
    }
}



// #####
//==================================================================================================
/*!
  @brief      read "SFImage" node

  @note       <BR>

  @date       2008-03-13 K.FUKUDA <BR>

  @return     void
*/
//==================================================================================================
void VrmlParserImpl::readSFImage(
    SFImage& out_image )		//!< to return read SFImage
{
    if( scanner->readSymbol( F_IS ) )
	{
            VrmlVariantField& field = readProtoField( SFIMAGE );
            out_image = field.sfImage();	//##### 要チェック
	}
    else
	{
            SFInt32			pixelValue;
            unsigned char	componentValue;

            readSFInt32( out_image.width );
            readSFInt32( out_image.height );
            readSFInt32( out_image.numComponents );

            //! start reading pixel values per component.
            //! numComponents means:
            //!   1:grayscale, 2:grayscale with transparency
            //!   3:RGB components, 4:RGB components with transparency
            int	w, h, i;
            int	shift;
            const SFInt32	comps = out_image.numComponents;

            for( h=0; h<out_image.height; h++ )
		{
                    for( w=0; w<out_image.width; w++ )
			{
                            readSFInt32( pixelValue );
                            for( i=0, shift=8*( comps - 1 ); i<comps; i++, shift-=8 )
				{
                                    // get each component values from left 8 bytes
                                    componentValue = (unsigned char)( (pixelValue>>shift) & 0x000000FF );
                                    out_image.pixels.push_back( componentValue );
				}
			}
		}
	}
}


void VrmlParserImpl::readSFTime(SFTime& out_value)
{
    if(scanner->readSymbol( F_IS )){
        VrmlVariantField& field = readProtoField( SFTIME );
        out_value = field.sfFloat();
    } else {
        out_value = scanner->readDoubleEx( "illegal time value" );
    }
}


void VrmlParserImpl::readMFTime(MFTime& out_value)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField( MFTIME );
        out_value = field.mfFloat();
    } else {
        SFFloat v;
        out_value.clear();
        if(!scanner->readChar('[' )){
            readSFTime( v );
            out_value.push_back( v );
        } else {
            while(!scanner->readChar(']')){
                readSFTime(v);
                out_value.push_back(v);
            }
        }
    }
}


// This API should be obsolete
void VrmlParserImpl::readSFNode(SFNode& out_node, VrmlNodeCategory nodeCategory)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFNODE);
        out_node = field.sfNode();
    } else if(scanner->readSymbol(V_NULL)){
        out_node = 0;
    } else {
        out_node = readNode(nodeCategory);
    }
}


SFNode VrmlParserImpl::readSFNode(VrmlNodeCategory nodeCategory)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(SFNODE);
        return field.sfNode();
    } else if(scanner->readSymbol(V_NULL)){
        return 0;
    } else {
        return readNode(nodeCategory);
    }
}


void VrmlParserImpl::readMFNode(MFNode& out_nodes, VrmlNodeCategory nodeCategory)
{
    if(scanner->readSymbol(F_IS)){
        VrmlVariantField& field = readProtoField(MFNODE);
        out_nodes = field.mfNode();
    } else {
        SFNode sfnode;
        out_nodes.clear();
        if(!scanner->readChar('[')){
            readSFNode(sfnode, nodeCategory);
            out_nodes.push_back(sfnode);
        } else {
            while(true) {
                readSFNode(sfnode, nodeCategory);
                if(sfnode){
                    out_nodes.push_back(sfnode);
                }
                bool closed = scanner->readChar(']');
                if(closed){
                    break;
                }
                if(!sfnode && !closed){
                    scanner->throwException("syntax error");
                }
            }
        }
    }
}

void VrmlParserImpl::init()
{
    currentProtoInstance = 0;
    protoInstanceActualNodeExtractionMode = true;

    scanner = boost::shared_ptr<EasyScanner>( new EasyScanner() );
    setSymbols();
}

void VrmlParserImpl::setSymbols()
{
    struct TSymbol {
        int id;
        const char* symbol;
    };

    static TSymbol symbols[] = {

        // values
        { V_TRUE, "TRUE"  },
        { V_FALSE, "FALSE" },
        { V_NULL, "NULL" },

        // types
        { T_SFINT32, "SFInt32" },
        { T_MFINT32, "MFInt32" },
        { T_SFFLOAT, "SFFloat" },
        { T_MFFLOAT, "MFFloat" },
        { T_SFVEC2F, "SFVec2f" },
        { T_MFVEC2F, "MFVec2f" },
        { T_SFVEC3F, "SFVec3f" },
        { T_MFVEC3F, "MFVec3f" },
        { T_SFROTATION, "SFRotation" },
        { T_MFROTATION, "MFRotation" },
        { T_SFTIME, "SFTime" },
        { T_MFTIME, "MFTime" },
        { T_SFCOLOR, "SFColor" },
        { T_MFCOLOR, "MFColor" },
        { T_SFSTRING, "SFString" },
        { T_MFSTRING, "MFString" },
        { T_SFNODE, "SFNode" },
        { T_MFNODE, "MFNode" },
        { T_SFBOOL, "SFBool" },
        { T_SFIMAGE, "SFImage" },

        // Nodes
        { N_PROTO, "PROTO" },
        { N_INLINE, "Inline" },
        { N_BACKGROUND, "Background" },
        { N_NAVIGATION_INFO, "NavigationInfo" },
        { N_VIEWPOINT, "Viewpoint" },
        { N_GROUP, "Group" },
        { N_TRANSFORM, "Transform" },
        { N_SHAPE, "Shape" },
        { N_APPEARANCE, "Appearance" },
        { N_MATERIAL, "Material" },
        { N_IMAGE_TEXTURE, "ImageTexture" },
        { N_TEXTURE_TRANSFORM, "TextureTransform" },
        { N_BOX, "Box" },
        { N_CONE, "Cone" },
        { N_CYLINDER, "Cylinder" },
        { N_SPHERE, "Sphere" },
        { N_TEXT, "Text" },
        { N_FONT_STYLE, "FontStyle" },
        { N_INDEXED_LINE_SET, "IndexedLineSet" },
        { N_INDEXED_FACE_SET, "IndexedFaceSet" },
        { N_COLOR, "Color" },
        { N_COORDINATE, "Coordinate" },
        { N_TEXTURE_COORDINATE, "TextureCoordinate" },
        { N_NORMAL, "Normal" },
        { N_CYLINDER_SENSOR, "CylinderSensor" },

        { N_POINTSET, "PointSet" },
        { N_PIXEL_TEXTURE,"PixelTexture" },
        { N_MOVIE_TEXTURE, "MovieTexture" },
        { N_ELEVATION_GRID, "ElevationGrid" },
        { N_EXTRUSION, "Extrusion" },
        { N_SWITCH, "Switch" },
        { N_LOD,"LOD" },
        { N_COLLISION,			"Collision" },
        { N_ANCHOR,				"Anchor" },
        { N_FOG,					"Fog" },
        { N_BILLBOARD,			"Billboard" },
        { N_WORLD_INFO,			"WorldInfo" },
        { N_POINT_LIGHT,			"PointLight" },
        { N_DIRECTIONAL_LIGHT,	"DirectionalLight" },
        { N_SPOT_LIGHT,			"SpotLight" },

        // unsupported nodes
        { N_AUDIO_CLIP,				"AudioClip" },
        { N_SOUND,					"Sound" },
        { N_COLOR_INTERPOLATOR,		"ColorInterpolator" },
        { N_COORDINATE_INTERPOLATOR,	"CoordinateInterpolator" },
        { N_ORIENTATION_INTERPOLATOR,	"OrientationInterpolator" },
        { N_NORMAL_INTERPOLATOR,		"NormalInterpolator" },
        { N_POSITION_INTERPOLATOR,	"PositionInterpolator" },
        { N_SCALAR_INTERPOLATOR,		"ScalarInterpolator" },
        { N_PLANE_SENSOR,				"PlaneSensor" },
        { N_PROXIMITY_SENSOR,			"ProximitySensor" },
        { N_SPHERE_SENSOR,			"SphereSensor" },
        { N_TIME_SENSOR,				"TimeSensor" },
        { N_TOUCH_SENSOR,				"TouchSensor" },
        { N_VISIBILITY_SENSOR,		"VisibilitySensor" },

        // Fields
        { F_IS, "IS" },

        { F_URL, "url" },

        { F_GROUND_ANGLE, "groundAngle" },
        { F_GROUND_COLOR, "groundColor" },
        { F_SKY_ANGLE, "skyAngle" },
        { F_SKY_COLOR, "skyColor" },
        { F_BACK_URL, "backUrl" },
        { F_BOTTOM_URL, "bottomUrl" },
        { F_FRONT_URL, "frontUrl" },
        { F_LEFT_URL, "leftUrl" },
        { F_RIGHT_URL, "rightUrl" },
        { F_TOP_URL, "topUrl" },

        { F_AVATAR_SIZE, "avatarSize" },
        { F_HEADLIGHT, "headlight" },
        { F_SPEED, "speed" },
        { F_TYPE, "type" },
        { F_VISIBILITY_LIMIT, "visibilityLimit" },

        { F_FIELD_OF_VIEW, "fieldOfView" },
        { F_JUMP, "jump" },
        { F_ORIENTATION, "orientation" },
        { F_POSITION, "position" },
        { F_DESCRIPTION, "description" },

        { F_CHILDREN, "children" },
        { F_ADD_CHILDREN, "addChildren" },
        { F_REMOVE_CHILDREN, "removeChildren" },
        { F_BBOX_CENTER, "bboxCenter" },
        { F_BBOX_SIZE, "bboxSize" },

        { F_CENTER, "center" },
        { F_ROTATION, "rotation" },
        { F_SCALE, "scale" },
        { F_SCALE_ORIENTATION, "scaleOrientation" },
        { F_TRANSLATION, "translation" },

        { F_APPEARANCE, "appearance" },
        { F_GEOMETRY, "geometry" },

        { F_MATERIAL, "material" },
        { F_TEXTURE, "texture" },
        { F_TEXTURE_TRANSFORM, "textureTransform" },

        { F_AMBIENT_INTENSITY, "ambientIntensity" },
        { F_DIFFUSE_COLOR, "diffuseColor" },
        { F_EMISSIVE_COLOR, "emissiveColor" },
        { F_SHININESS, "shininess" },
        { F_SPECULAR_COLOR, "specularColor" },
        { F_TRANSPARANCY, "transparency" },
        { F_DIRECTION, "direction" },

        { F_REPEAT_S, "repeatS" },
        { F_REPEAT_T, "repeatT" },

        { F_SIZE, "size" },

        { F_BOTTOM, "bottom" },
        { F_BOTTOM_RADIUS, "bottomRadius" },
        { F_HEIGHT, "height" },
        { F_SIDE, "side" },

        { F_RADIUS, "radius" },
        { F_TOP, "top" },

        { F_STRING, "string" },
        { F_FONT_STYLE, "fontStyle" },
        { F_LENGTH, "length" },
        { F_MAX_EXTENT, "maxExtent" },

        { F_FAMILY, "family" },
        { F_HORIZONTAL, "horizontal" },
        { F_JUSTIFY, "justify" },
        { F_LANGUAGE, "language" },
        { F_LEFT_TO_RIGHT, "leftToRight" },
        { F_SPACING, "spacing" },
        { F_STYLE, "style" },
        { F_TOP_TO_BOTTOM, "topToBottom" },

        { F_COLOR, "color" },
        { F_COORD, "coord" },
        { F_COLOR_INDEX, "colorIndex" },
        { F_COLOR_PER_VERTEX, "colorPerVertex" },
        { F_COORD_INDEX, "coordIndex" },

        { F_CCW, "ccw" },
        { F_CONVEX, "convex" },
        { F_SOLID, "solid" },
        { F_CREASE_ANGLE, "creaseAngle" },
        { F_NORMAL_INDEX, "normalIndex" },
        { F_NORMAL, "normal" },
        { F_NORMAL_PER_VERTEX, "normalPerVertex" },
        { F_TEX_COORD_INDEX, "texCoordIndex" },
        { F_TEX_COORD, "texCoord" },

        { F_POINT, "point" },
        { F_VECTOR, "vector" },

        { F_AUTO_OFFSET, "autoOffset" },
        { F_DISK_ANGLE, "diskAngle" },
        { F_ENABLED, "enabled" },
        { F_MAX_ANGLE, "maxAngle" },
        { F_MIN_ANGLE, "minAngle" },
        { F_OFFSET, "offset" },

        { F_IMAGE, "image" },

        { F_X_DIMENSION, "xDimension" },
        { F_Z_DIMENSION, "zDimension" },
        { F_X_SPACING, "xSpacing" },
        { F_Z_SPACING, "zSpacing" },

        { F_CROSS_SECTION, "crossSection" },
        { F_SPINE, "spine" },
        { F_BEGIN_CAP, "beginCap" },
        { F_END_CAP, "endCap" },

        { F_CHOICE, "choice" },
        { F_WHICH_CHOICE, "whichChoice" },

        { F_COLLIDE, "collide" },
        { F_PROXY, "proxy" },

        { F_PARAMETER, "parameter" },

        { F_VISIBILITY_RANGE, "visibilityRange" },
        { F_FOG_TYPE, "fogType" },

        { F_AXIS_OF_ROTATION, "axisOfRotation" },

        { F_TITLE, "title" },
        { F_INFO, "info" },

        // event type
        { E_FIELD, "field" },
        { E_EXPOSED_FIELD, "exposedField" },
        { E_EVENTIN, "eventIn" },
        { E_EVENTOUT, "eventOut" },

        // def & route
        { D_DEF, "DEF" },
        { D_USE, "USE" },
        { D_ROUTE, "ROUTE" },

        // unsupported keywords
        { U_SCRIPT, "Script" },
        { U_EXTERNPROTO, "EXTERNPROTO" },

        { 0, "" }
    };

    for(int i=0; symbols[i].id != 0; i++){
        scanner->registerSymbol(symbols[i].id, symbols[i].symbol);
    }
}

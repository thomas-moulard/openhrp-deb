/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * National Institute of Advanced Industrial Science and Technology (AIST)
 * General Robotix Inc. 
 */
/**
   \file
   \author Shin'ichiro Nakaoka
*/

#include "BridgeConf.h"

#include <iostream>
#include <fstream>
#include <boost/regex.hpp>

#if (BOOST_VERSION <= 103301)
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp> 
#else
#include <boost/filesystem.hpp>
#endif


using namespace std;
using namespace boost;


namespace {
  BridgeConf* bridgeConf = 0;
}


BridgeConf* BridgeConf::initialize(int argc, char* argv[])
{
  bridgeConf = new BridgeConf(argc ,argv);
  return bridgeConf;
}

  
BridgeConf* BridgeConf::instance()
{
  return bridgeConf;
}


BridgeConf::BridgeConf(int argc, char* argv[]) :
  options("Allowed options"),
  commandLineOptions("Allowed options")
{
  isReady_ = false;
  initOptionsDescription();
  initLabelToDataTypeMap();

  setPreLoadModuleInfo();
  parseCommandLineOptions(argc, argv);
}


BridgeConf::~BridgeConf()
{

}


void BridgeConf::initOptionsDescription()
{
  options.add_options()

    ("name-server",
     program_options::value<string>()->default_value("127.0.0.1:2809"),
     "Nameserver used by OpenHRP (hostname:port)")

    ("server-name",
     program_options::value<string>()->default_value("ControllerBridge"),
     "Name of the OpenHRP controller factory server")

    ("robot-name",
     program_options::value<string>()->default_value(""),
     "Name of the virtual robot RTC type")

    ("module",
     program_options::value<vector<string> >(),
     "Module filename")

    ("out-port",
     program_options::value<vector<string> >(),
     "Set an out-port that transfers data from the simulator to the controller")

    ("in-port",
     program_options::value<vector<string> >(),
     "Set an in-port transfers data from the controller to the simulator")

    ("connection",
     program_options::value<vector<string> >(),
     "Port connection setting")

    ("periodic-rate",
     program_options::value<vector<string> >(), 
     "Periodic rate of execution context (INSTANCE_NAME:TIME_RATE[<=1.0])");

  commandLineOptions.add(options).add_options()

    ("config-file",
     program_options::value<string>(), 
     "configuration file of the controller bridge")
    
    ("help,h", "Show this help");
}


void BridgeConf::initLabelToDataTypeMap()
{
  LabelToDataTypeIdMap& m = labelToDataTypeIdMap;
  
  m["JOINT_VALUE"]         = JOINT_VALUE;
  m["JOINT_VELOCITY"]      = JOINT_VELOCITY;
  m["JOINT_ACCELERATION"]  = JOINT_ACCELERATION;
  m["JOINT_TORQUE"]        = JOINT_TORQUE;
  m["EXTERNAL_FORCE"]      = EXTERNAL_FORCE;
  m["ABS_TRANSFORM"]       = ABS_TRANSFORM;
  m["ABS_VELOCITY"]        = ABS_VELOCITY;
  m["ABS_ACCELERATION"]    = ABS_ACCELERATION;
  m["FORCE_SENSOR"]        = FORCE_SENSOR;
  m["RATE_GYRO_SENSOR"]    = RATE_GYRO_SENSOR;
  m["ACCELERATION_SENSOR"] = ACCELERATION_SENSOR;
  m["COLOR_IMAGE"]         = COLOR_IMAGE;
  m["GRAYSCALE_IMAGE"]     = GRAYSCALE_IMAGE;
  m["DEPTH_IMAGE"]         = DEPTH_IMAGE;
  m["RANGE_SENSOR"]	       = RANGE_SENSOR;
  m["CONSTRAINT_FORCE"]    = CONSTRAINT_FORCE;
  m["RATE_GYRO_SENSOR2"]    = RATE_GYRO_SENSOR2;
  m["ACCELERATION_SENSOR2"] = ACCELERATION_SENSOR2;
  m["ABS_TRANSFORM2"]       = ABS_TRANSFORM2;
}


void BridgeConf::parseCommandLineOptions(int argc, char* argv[])
{
  isProcessingConfigFile = false;

  program_options::parsed_options parsed = 
      program_options::command_line_parser(argc, argv).options(commandLineOptions).allow_unregistered().run();      
  program_options::store(parsed, vmap); 

  if(vmap.count("help")){
    cout << commandLineOptions << endl;
  } else {

    if(vmap.count("config-file")){
      string fileName(vmap["config-file"].as<string>());
      ifstream ifs(fileName.c_str());
      
      if (ifs.fail()) {
	    throw invalid_argument(string("cannot open the config file"));
      }
      program_options::store(program_options::parse_config_file(ifs, options), vmap);
      
      isProcessingConfigFile = true;
    }
    program_options::notify(vmap);
    
    parseOptions();

    isReady_ = true;
  }
}


void BridgeConf::parseOptions()
{
  if(vmap.count("module")){
    vector<string> values = vmap["module"].as<vector<string> >();
    for(size_t i=0; i < values.size(); ++i){
      string modulePath( values[i] );
      if( filesystem::extension(filesystem::path( modulePath )).empty() )
      {
        modulePath += string( SUFFIX_SHARED_EXT );
      }
      addModuleInfo( modulePath );
    }
  }

  if(vmap.count("out-port")){
    setPortInfos("out-port", outPortInfos);
  }

  if(vmap.count("in-port")){
    setPortInfos("in-port", inPortInfos);
  }

  if(vmap.count("connection")){
    vector<string> values = vmap["connection"].as<vector<string> >();
    for(size_t i=0; i < values.size(); ++i){
      addPortConnection(values[i]);
    }
  }

  string server(expandEnvironmentVariables(vmap["name-server"].as<string>()));
  nameServerIdentifier = string("corbaloc:iiop:") + server + "/NameService";

  controllerName = expandEnvironmentVariables(vmap["server-name"].as<string>());

  virtualRobotRtcTypeName = expandEnvironmentVariables(vmap["robot-name"].as<string>());
  if(virtualRobotRtcTypeName==""){
      virtualRobotRtcTypeName=controllerName;
      virtualRobotRtcTypeName.append("(Robot)");
  }

  if(vmap.count("periodic-rate")){
    vector<string> values = vmap["periodic-rate"].as<vector<string> >();
    for(size_t i=0; i < values.size(); ++i){
      addTimeRateInfo(values[i]);
    }
  }
}


void BridgeConf::setPortInfos(const char* optionLabel, PortInfoMap& portInfos)
{
  vector<string> ports = vmap[optionLabel].as<vector<string> >();
  for(size_t i=0; i < ports.size(); ++i){
    vector<string> parameters = extractParameters(ports[i]);
    int n = parameters.size();
    if(n < 2 || n > 4){
      throw invalid_argument(string("invalid in port setting"));
    }
    PortInfo info;

    info.portName = parameters[0];
    int j;
    for(j=1; j<3; j++){
        LabelToDataTypeIdMap::iterator it = labelToDataTypeIdMap.find(parameters[j]);
        if(it == labelToDataTypeIdMap.end() ){     // �v���p�e�B���łȂ��̂Ŏ��ʖ��@ //
            if(j==2)    // 3�Ԗڂ܂łɃv���p�e�B�����Ȃ��̂ŃG���[�@//
                throw invalid_argument(string("invalid data type"));
            string st=parameters[j];
            vector<string> owners = extractParameters(st, ',');
            for(size_t i=0; i<owners.size(); i++){
                bool digit=true;
                for(string::iterator itr=owners[i].begin(); itr!=owners[i].end(); itr++){
                    if(!isdigit(*itr)){
                        digit = false;             
                        break;
                    }
                }
                if(digit){  //���ʖ��������Ȃ̂ŉ摜�f�[�^�@�Q�ȏ�w�肷��ƃG���[�@//
                    info.dataOwnerId = atoi(owners[i].c_str());
                    info.dataOwnerName.clear();
                    if(owners.size() > 1){
                        throw invalid_argument(string("invalid VisionSensor setting"));
                    }
                }else{      //���ʖ������O  //
                    info.dataOwnerId = -1;
                    info.dataOwnerName.push_back(owners[i]);
                }
            }
        }else{  // �v���p�e�B�� //
            info.dataTypeId = it->second;
            j++;
            break;
        }
    }
    if(j<n){    // �܂��p�����^�[������Ȃ�T���v�����O���ԁ@//
        info.stepTime = atof(parameters[j].c_str());
    }else{
        info.stepTime = 0;
    }

    // �v���p�e�B��CONSTRAINT_FORCE�̂Ƃ��@���ʖ����Q�ȏ゠���Ă͂����Ȃ��@   //
    if(info.dataTypeId==CONSTRAINT_FORCE && info.dataOwnerName.size() > 1 )
        throw invalid_argument(string("invalid in port setting"));
    portInfos.insert(make_pair(info.portName, info));
  }
}


void BridgeConf::addPortConnection(const std::string& value)
{
  vector<string> parameters = extractParameters(value);
  int n = parameters.size();
  if(n < 2 || n > 3){
    throw std::invalid_argument(string("Invalied port connection"));
  }
  PortConnection connection;
  connection.robotPortName = parameters[0];
  
  if(parameters.size() == 2){
    if(moduleInfoList.size() != 1){
      throw std::invalid_argument(string("RTC instance name must be specified in a connection option"));
    }
    connection.controllerPortName = parameters[1];
  } else {
    connection.controllerInstanceName = parameters[1];
    connection.controllerPortName = parameters[2];
  }
  portConnections.push_back(connection);
}

void BridgeConf::setPreLoadModuleInfo()
{
    RTC::Manager& rtcManager = RTC::Manager::instance();
    std::vector<RTC::RtcBase*> components = rtcManager.getComponents();
    for(std::vector<RTC::RtcBase*>::iterator it=components.begin(); it != components.end(); ++it){ 
        ModuleInfo info;
        info.componentName = (*it)->get_component_profile()->type_name;
        info.fileName = "";
        info.initFuncName = "";
        info.isLoaded = true;
        info.rtcServant = *it;
        moduleInfoList.push_back(info);
    }
}

void BridgeConf::addModuleInfo(const std::string& value)
{
  vector<string> parameters = extractParameters(value);

  if(parameters.size() == 0 || parameters.size() > 2){
    throw std::invalid_argument(std::string("invalid module set"));
  } else {
    ModuleInfo info;
    info.fileName = parameters[0];
    info.componentName = filesystem::basename(filesystem::path(info.fileName));
    if(parameters.size() == 1){
      info.initFuncName = info.componentName + "Init";
    } else {
      info.initFuncName = parameters[1];
    }
    info.isLoaded = false;
    
    moduleInfoList.push_back(info);
  }
}

void BridgeConf::addTimeRateInfo(const std::string& value) 
{
  vector<string> parameters = extractParameters(value);
  if (parameters.size() == 0 || parameters.size() > 2) {
    throw std::invalid_argument(std::string("invalid time rate set"));
  } else {
    timeRateMap.insert( std::map<std::string, double>::value_type(parameters[0], atof(parameters[1].c_str())) );
  }
}

void BridgeConf::setupModules()
{
  RTC::Manager& rtcManager = RTC::Manager::instance();
  ModuleInfoList::iterator moduleInfo = moduleInfoList.begin();
  while(moduleInfo != moduleInfoList.end()){
    if(!moduleInfo->isLoaded){
      rtcManager.load(moduleInfo->fileName.c_str(), moduleInfo->initFuncName.c_str());
      moduleInfo->isLoaded = true;
      moduleInfo->rtcServant = rtcManager.createComponent(moduleInfo->componentName.c_str());
    }
    ++moduleInfo;
  }
}


const char* BridgeConf::getOpenHRPNameServerIdentifier()
{
  return nameServerIdentifier.c_str();
}


const char* BridgeConf::getControllerName()
{
  return controllerName.c_str();
}


const char* BridgeConf::getVirtualRobotRtcTypeName()
{
  return virtualRobotRtcTypeName.c_str();
}

std::vector<std::string> BridgeConf::extractParameters(const std::string& str, const char delimiter )
{
  vector<string> result;
  string::size_type sepPos = 0;
  string::size_type nowPos = 0;
  
  while (sepPos != string::npos) {
    sepPos = str.find(delimiter, nowPos);

    if(isProcessingConfigFile){
      result.push_back(expandEnvironmentVariables(str.substr(nowPos, sepPos-nowPos)));
    } else {
      result.push_back(str.substr(nowPos, sepPos-nowPos));
    }
      
    nowPos = sepPos+1;
  }
  
  return result;
}


std::string BridgeConf::expandEnvironmentVariables(std::string str)
{
  regex variablePattern("\\$([A-z][A-z_0-9]*)");
    
  match_results<string::const_iterator> result; 
  match_flag_type flags = match_default; 
    
  string::const_iterator start, end;
  start = str.begin();
  end = str.end();
  int pos = 0;

  vector< pair< int, int > > results;

  while ( regex_search(start, end, result, variablePattern, flags) ) {
    results.push_back(std::make_pair(pos+result.position(1), result.length(1)));

    // seek to the remaining part
    start = result[0].second;
    pos += result.length(0);
		
    flags |= boost::match_prev_avail; 
    flags |= boost::match_not_bob; 
  }

  // replace the variables in reverse order
  while (!results.empty()) {
    int begin = results.back().first;
    int length = results.back().second;

    string envName = str.substr(begin, length);
    char* envValue = getenv(envName.c_str());

    if(envValue){
      str.replace(begin-1, length+1, string(envValue));
    }
    results.pop_back();
  }

  return str;
}

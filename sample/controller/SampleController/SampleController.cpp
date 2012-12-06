// -*- C++ -*-
/*
 * Copyright (c) 2008, AIST, the University of Tokyo and General Robotix Inc.
 * All rights reserved. This program is made available under the terms of the
 * Eclipse Public License v1.0 which accompanies this distribution, and is
 * available at http://www.eclipse.org/legal/epl-v10.html
 * Contributors:
 * National Institute of Advanced Industrial Science and Technology (AIST)
 * General Robotix Inc. 
 */
/*!
 * @file  SampleController.cpp
 * @brief Sample component
 * $Date$
 *
 * $Id$
 */

#include "SampleController.h"
#include <iostream>

#define TIMESTEP 0.002

#define ANGLE_FILE "etc/Sample.pos"
#define VEL_FILE   "etc/Sample.vel"
#define GAIN_FILE  "etc/SR_PDgain.dat"

#define RARM_SHOULDER_P 6
#define RARM_SHOULDER_R 7
#define RARM_ELBOW 9
#define RARM_WRIST_Y 10
#define RARM_WRIST_R 12

//#define SC_DEBUG

using namespace std;

// Module specification
// <rtc-template block="module_spec">
static const char* samplecontroller_spec[] =
  {
    "implementation_id", "SampleController",
    "type_name",         "SampleController",
    "description",       "Sample component",
    "version",           "0.1",
    "vendor",            "AIST",
    "category",          "Generic",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables

    ""
  };
// </rtc-template>

SampleController::SampleController(RTC::Manager* manager)
  : RTC::DataFlowComponentBase(manager),
    // <rtc-template block="initializer">
    m_angleIn("angle", m_angle),
    m_rhsensorIn("rhsensor", m_rhsensor),
    m_torqueOut("torque", m_torque),
//    m_qOut("q", m_q),
//    m_dqOut("dq", m_dq),
//    m_ddqOut("ddq", m_ddq),
    // </rtc-template>
    dummy(0)
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>
  
}

SampleController::~SampleController()
{
}


RTC::ReturnCode_t SampleController::onInitialize()
{
  // <rtc-template block="bind_config">
  // Bind variables and configuration variable

  // Set InPort buffers
  addInPort("angle", m_angleIn);
  addInPort("rhsensor", m_rhsensorIn);
  
  // Set OutPort buffer
  addOutPort("torque", m_torqueOut);
//  addOutPort("q", m_qOut);
//  addOutPort("dq", m_dqOut);
//  addOutPort("ddq", m_ddqOut);
  // </rtc-template>

  Pgain = new double[DOF];
  Dgain = new double[DOF];
  std::ifstream gain;

  gain.open(GAIN_FILE);
  if (gain.is_open()){
    for (int i=0; i<DOF; i++){
      gain >> Pgain[i];
      gain >> Dgain[i];
#ifdef SC_DEBUG
      cout << Pgain[i] << " " << Dgain[i] << " ";
#endif
    }
    gain.close();
#ifdef SC_DEBUG
    cout << endl;
#endif
  }else{
    cerr << GAIN_FILE << " not opened" << endl;
  }

  m_torque.data.length(DOF);
  m_rhsensor.data.length(6);
  m_angle.data.length(DOF);
//  m_q.data.length(DOF);
//  m_dq.data.length(DOF);
//  m_ddq.data.length(DOF);

  qold = new double[DOF];

#ifdef SC_DEBUG
  out.open("debug.log");
#endif	

  return RTC::RTC_OK;
}

RTC::ReturnCode_t SampleController::onFinalize()
{
  closeFiles();
  delete [] Pgain;
  delete [] Dgain;
  delete[] qold;
#ifdef SC_DEBUG
  out.close();
#endif	
  return RTC::RTC_OK;
}


/*
RTC::ReturnCode_t SampleController::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t SampleController::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

RTC::ReturnCode_t SampleController::onActivated(RTC::UniqueId ec_id)
{
  std::cout << "on Activated" << std::endl;
  goal_set = true;
  pattern = false;
  remain_t = 2.0;
  step = 0;
  openFiles();

  if(m_angleIn.isNew()){
    m_angleIn.read();
  }

  for(int i=0; i < DOF; ++i){
    qold[i] = m_angle.data[i];
    q_ref[i] = dq_ref[i] = q_goal[i] = dq_goal[i] = 0.0;
  }

  return RTC::RTC_OK;
}


RTC::ReturnCode_t SampleController::onDeactivated(RTC::UniqueId ec_id)
{
  std::cout << "on Deactivated" << std::endl;
  closeFiles();
  return RTC::RTC_OK;
}


RTC::ReturnCode_t SampleController::onExecute(RTC::UniqueId ec_id)
{
  if(m_angleIn.isNew()){
    m_angleIn.read();
  }
  if(m_rhsensorIn.isNew()){
    m_rhsensorIn.read();
  }
/*
  static double q_ref[DOF], dq_ref[DOF];
  static double remain_t = 2.0;
  static double q_goal[DOF], dq_goal[DOF];
  static int step = 0;
*/

  if( goal_set ){
    goal_set = false;
    switch(step){
      case 0 :
        remain_t = 2.0;
        double dumy;
        angle >> dumy; vel >> dumy;// skip time
        for (int i=0; i<DOF; i++){
          angle >> q_goal[i];
          vel >> dq_goal[i];
          q_ref[i] = m_angle.data[i];
        }
        break;
      case 1 :
        pattern = true;
        break;
      case 2 : 
        remain_t = 3.0;
        for(int i=0; i<DOF; i++){
          q_goal[i] = q_ref[i] = m_angle.data[i];
        }
        q_goal[RARM_SHOULDER_R] = -0.4;
        q_goal[RARM_SHOULDER_P] = 0.75;
        q_goal[RARM_ELBOW] = -2.0;
        break;
      case 3 : 
        remain_t = 2.0;
        q_goal[RARM_ELBOW] = -1.57;
        q_goal[RARM_SHOULDER_P] = -0.2;
        q_goal[RARM_WRIST_R] = 1.5;
        break;
      case 4 :
        remain_t =1.0;
        q_goal[RARM_ELBOW] = -1.3;
        break;
      case 5 :
        remain_t =5.0;
        q_goal[RARM_SHOULDER_R] = 0.1;
        break;
      case 6 :
        remain_t =2.0;
        q_goal[RARM_WRIST_R] = -0.3;
        break;
      case 7 :
        remain_t =0.5;
        break;
      case 8 :
        remain_t =1.0;
        q_goal[RARM_SHOULDER_P] = 0.13;
        q_goal[RARM_ELBOW] = -1.8;
        break;
      case 9 :
        remain_t = 0.0;
        step = -2;
        break;
    }
    step++;
  }

  if( step==6 && m_rhsensor.data[1] < -2 ) { 
    remain_t = 0;
    q_goal[RARM_SHOULDER_R] = m_angle.data[RARM_SHOULDER_R];
  }

  if( !pattern ){
    if (remain_t > TIMESTEP){
      for(int i=0; i<DOF; i++){
        dq_ref[i] = (q_goal[i]-q_ref[i])/remain_t;
        q_ref[i] = q_ref[i] + dq_ref[i]*TIMESTEP;
      }
      remain_t -= TIMESTEP;
    }else{
      for(int i=0; i<DOF; i++){
        dq_ref[i]= 0;
        q_ref[i] = q_goal[i];
      }
      if(step >=0 ) goal_set = true;
    }
  }else{
    angle >> dq_ref[0]; vel >> dq_ref[0];// skip time
    for (int i=0; i<DOF; i++){
      angle >> q_ref[i];
      vel >> dq_ref[i];
    }
    if( angle.eof() ) { 
      pattern = false;
      goal_set = true;
    }
  }

  for(int i=0; i<DOF; i++){
    double dq = (m_angle.data[i] - qold[i]) / TIMESTEP;
    m_torque.data[i] = -(m_angle.data[i] - q_ref[i]) * Pgain[i] - (dq - dq_ref[i]) * Dgain[i];
    qold[i] = m_angle.data[i];
#ifdef SC_DEBUG
    out << m_angle.data[i] << " " <<   q_ref[i] << " " << dq << " " << dq_ref[i] << " " << m_torque.data[i] << " ";
#endif
  }
  m_torqueOut.write();

#ifdef SC_DEBUG
  out << endl;
#endif

  return RTC::RTC_OK;
}


/*
RTC::ReturnCode_t SampleController::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t SampleController::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t SampleController::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t SampleController::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t SampleController::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

void SampleController::openFiles()
{
  angle.open(ANGLE_FILE);
  if (!angle.is_open()){
    std::cerr << ANGLE_FILE << " not opened" << std::endl;
  }

  vel.open(VEL_FILE);
  if (!vel.is_open()){
    std::cerr << VEL_FILE << " not opened" << std::endl;
  }
}
void SampleController::closeFiles()
{
  if(angle.is_open())
  {
    angle.close();
    angle.clear();
  }
  if(vel.is_open()){
    vel.close();
    vel.clear();
  }
}


extern "C"
{
 
  DLL_EXPORT void SampleControllerInit(RTC::Manager* manager)
  {
    coil::Properties profile(samplecontroller_spec);
    manager->registerFactory(profile,
                             RTC::Create<SampleController>,
                             RTC::Delete<SampleController>);
  }
  
};



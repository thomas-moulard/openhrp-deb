// -*- mode: c++; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-
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
 * @file  SampleLF.cpp
 * @brief Sample LF component
 * $Date$
 *
 * $Id$
 */

#include "SampleLF.h"

#include <iostream>
#include <math.h>

#define DOF (29)
#define TIMESTEP 0.002

#define GAIN_FILE  "etc/PDgain.dat"


namespace {
  const bool CONTROLLER_BRIDGE_DEBUG = false;
}


// Module specification
// <rtc-template block="module_spec">
static const char* samplepd_spec[] =
  {
    "implementation_id", "SampleLF",
    "type_name",         "SampleLF",
    "description",       "Sample LF component",
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

SampleLF::SampleLF(RTC::Manager* manager)
  : RTC::DataFlowComponentBase(manager),
    // <rtc-template block="initializer">
    m_angleIn("angle", m_angle),
    m_torqueInL("r_torque_out",m_torqueL),
    m_torqueInR("l_torque_out",m_torqueR),
    m_torqueOut("torque", m_torque),
    
    // </rtc-template>
    dummy(0),
    qold(DOF),
    qref_old(DOF)
{
  if( CONTROLLER_BRIDGE_DEBUG )
  {
    std::cout << "SampleLF::SampleLF" << std::endl;
  }
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">

  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>
}

SampleLF::~SampleLF()
{
  closeFiles();
  delete [] Pgain;
  delete [] Dgain;
}


RTC::ReturnCode_t SampleLF::onInitialize()
{
  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  if( CONTROLLER_BRIDGE_DEBUG )
  {
    std::cout << "onInitialize" << std::endl;
  }

  // Set InPort buffers
  addInPort("angle", m_angleIn);
  addInPort("r_torque_out", m_torqueInL);
  addInPort("l_torque_out", m_torqueInR);

  // Set OutPort buffer
  addOutPort("torque", m_torqueOut);
  // </rtc-template>

  Pgain = new double[DOF];
  Dgain = new double[DOF];

  gain.open(GAIN_FILE);
  if (gain.is_open()){
    for (int i=0; i<DOF; i++){
      gain >> Pgain[i];
      gain >> Dgain[i];
    }
    gain.close();
  }else{
    std::cerr << GAIN_FILE << " not opened" << std::endl;
  }
  m_torque.data.length(DOF);
  m_torqueL.data.length(1);
  m_torqueR.data.length(1);
  m_angle.data.length(DOF);

  return RTC::RTC_OK;
}


/*
RTC::ReturnCode_t SampleLF::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t SampleLF::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t SampleLF::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

RTC::ReturnCode_t SampleLF::onActivated(RTC::UniqueId ec_id)
{

  std::cout << "on Activated" << std::endl;
  check = true;
  file = 1;

  openFiles();

  if(m_angleIn.isNew()){
    m_angleIn.read();
  }

  for(int i=0; i < DOF; ++i){
    qold[i] = m_angle.data[i];
    qref_old[i] = 0.0;
  }

  return RTC::RTC_OK;
}


RTC::ReturnCode_t SampleLF::onDeactivated(RTC::UniqueId ec_id)
{
  std::cout << "on Deactivated" << std::endl;
  closeFiles();
  return RTC::RTC_OK;
}



RTC::ReturnCode_t SampleLF::onExecute(RTC::UniqueId ec_id)
{
  if( CONTROLLER_BRIDGE_DEBUG )
  {
    std::cout << "SampleLF::onExecute" << std::endl;
  }

  // この関数の振る舞いはController_impl::controlの派生先仮想関数に対応する //
  if(m_angleIn.isNew()){
    m_angleIn.read();
  }

  if(m_torqueInL.isNew()){
    m_torqueInL.read();
  }

  if(m_torqueInR.isNew()){
    m_torqueInR.read();
  }

  double q_ref, dq_ref;
  double threshold = 30.0;


  // *.datの読み込み //
  // 行頭の時間データのスキップと行の存在チェックを兼ねた処理 //
  // 行が存在しなければ次の行を読み込む //
  if(file==1)
  {
    if( !(angle1 >> dq_ref &&  vel1 >> dq_ref) )// skip time
    {
      file=2;
    }
  }

  if(file==2)
  {
    if( !(angle2 >> dq_ref &&  vel2 >> dq_ref) )// skip time
    {
      file=3;
    }

    if( ( fabs( m_torqueR.data[0] ) < threshold || fabs( m_torqueL.data[0] ) < threshold ) &&  check )
    {
      file=3;
    }

    check = false;
  }

  // *.dat一行の読み込み//
  for (int i=0; i<DOF; i++)
  {
    switch(file)
    {
      case 1:
        angle1 >> q_ref;
        vel1 >> dq_ref;
        break;
      case 2:
        angle2 >> q_ref;
        vel2 >> dq_ref;
        break;
      case 3:
        q_ref = qref_old[i];
        dq_ref=0.0;
        break;
    }//switch(file)
    double q = m_angle.data[i];
    double dq = (q - qold[i]) / TIMESTEP;

    m_torque.data[i] = -(q - q_ref) * Pgain[i] - (dq - dq_ref) * Dgain[i];

    qold[i] = q;
    if(file !=3)
      qref_old[i] = q_ref;
  }//for (int i=0; i<DOF; i++)

  m_torqueOut.write();
  
  return RTC::RTC_OK;
}


/*
  RTC::ReturnCode_t SampleLF::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t SampleLF::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t SampleLF::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t SampleLF::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t SampleLF::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/
void SampleLF::openFiles()
{
  angle1.open("etc/LFangle1.dat");
  if (!angle1.is_open()){
    std::cerr << "etc/LFangle1.dat not opened" << std::endl;
  }

  angle2.open("etc/LFangle2.dat");
  if (!angle2.is_open()){
    std::cerr << "etc/LFangle2.dat not opened" << std::endl;
  }

  vel1.open("etc/LFvel1.dat");
  if (!vel1.is_open()){
    std::cerr << "etc/LFvel1.dat not opened" << std::endl;
  }

  vel2.open("etc/LFvel2.dat");
  if (!vel2.is_open()){
    std::cerr << "etc/LFvel2.dat not opened" << std::endl;
  }
}

void SampleLF::closeFiles()
{
  if( angle1.is_open() ){
    angle1.close();
    angle1.clear();
  }
  if( angle2.is_open() ){
    angle2.close();
    angle2.clear();
  }
  if( vel1.is_open() ){
    vel1.close();
    vel1.clear();
  }
  if( vel2.is_open() ){
    vel2.close();
    vel2.clear();
  }
}



extern "C"
{

  DLL_EXPORT void SampleLFInit(RTC::Manager* manager)
  {
    coil::Properties profile(samplepd_spec);
    manager->registerFactory(profile,
                             RTC::Create<SampleLF>,
                             RTC::Delete<SampleLF>);
  }

};


# @author Shin'ichiro Nakaoka

set(program openhrp-model-loader)
set(exporter export-collada)

set(sources
  ShapeSetInfo_impl.cpp
  SceneInfo_impl.cpp
  BodyInfo_impl.cpp
  ModelLoader_impl.cpp
  VrmlUtil.cpp
  server.cpp )

set(sources2
  exportCollada.cpp
  BodyInfo_impl.cpp
  ShapeSetInfo_impl.cpp
  VrmlUtil.cpp )

set(sources3
  exportVrml.cpp
  VrmlWriter.cpp )

if( COLLADA_DOM_FOUND )
  set(sources ${sources} BodyInfoCollada_impl.cpp)
  include_directories(${COLLADA_DOM_INCLUDE_DIRS})
  link_directories(${COLLADA_DOM_LIBRARY_DIRS})
  add_executable(${exporter} ${sources2})
  if(WIN32)
    set(extralibraries  ${COLLADA_DOM_LIBRARIES})
    find_package(Boost ${COLLADA_DOM_Boost_VERSION} EXACT COMPONENTS signals system)
    set(extralibraries_win  optimized ${extralibraries} debug ${extralibraries}d
      ${Boost_DATE_TIME_LIBRARY}
      ${Boost_THREAD_LIBRARY}
      ${Boost_SIGNALS_LIBRARY}
      ${Boost_SYSTEM_LIBRARY} )
  else(WIN32)
    set(extralibraries  ${COLLADA_DOM_LIBRARIES} boost_system-mt)
  endif(WIN32)
else( COLLADA_DOM_FOUND )
  set(extralibraries)
endif( COLLADA_DOM_FOUND )

add_executable(${program} ${sources})

if(UNIX)
  target_link_libraries(${program}
    hrpUtil-${OPENHRP_LIBRARY_VERSION}
    hrpModel-${OPENHRP_LIBRARY_VERSION}
    hrpCorbaStubSkel-${OPENHRP_LIBRARY_VERSION}
    #${OMNIORB_LIBRARIES}
    ${OPENRTM_LIBRARIES}
    ${extralibraries}
    )

  if( COLLADA_DOM_FOUND )
    target_link_libraries(${exporter}
      hrpUtil-${OPENHRP_LIBRARY_VERSION}
      hrpModel-${OPENHRP_LIBRARY_VERSION}
      hrpCorbaStubSkel-${OPENHRP_LIBRARY_VERSION}
      #${OMNIORB_LIBRARIES}
      ${OPENRTM_LIBRARIES}
      ${extralibraries}
      )
    set_target_properties(${exporter} PROPERTIES COMPILE_FLAGS "${COLLADA_DOM_CXX_FLAGS}" LINK_FLAGS "${COLLADA_DOM_LINK_FLAGS}")
  endif( COLLADA_DOM_FOUND )

elseif(WIN32)
  set_target_properties(${program} PROPERTIES DEBUG_POSTFIX d)
  target_link_libraries(${program}
    optimized hrpUtil-${OPENHRP_LIBRARY_VERSION} 
    optimized hrpModel-${OPENHRP_LIBRARY_VERSION}
    optimized hrpCorbaStubSkel-${OPENHRP_LIBRARY_VERSION} 
    optimized hrpCollision-${OPENHRP_LIBRARY_VERSION} 
    debug hrpUtil-${OPENHRP_LIBRARY_VERSION}d
    debug hrpModel-${OPENHRP_LIBRARY_VERSION}d
    debug hrpCorbaStubSkel-${OPENHRP_LIBRARY_VERSION}d
    debug hrpCollision-${OPENHRP_LIBRARY_VERSION}d
    ${OMNIORB_LIBRARIES}
    ${extralibraries_win}
    )
    
  if( COLLADA_DOM_FOUND )
    target_link_libraries(${exporter}
    optimized hrpUtil-${OPENHRP_LIBRARY_VERSION} 
    optimized hrpModel-${OPENHRP_LIBRARY_VERSION}
    optimized hrpCorbaStubSkel-${OPENHRP_LIBRARY_VERSION} 
    debug hrpUtil-${OPENHRP_LIBRARY_VERSION}d
    debug hrpModel-${OPENHRP_LIBRARY_VERSION}d
    debug hrpCorbaStubSkel-${OPENHRP_LIBRARY_VERSION}d
    debug hrpCollision-${OPENHRP_LIBRARY_VERSION}d
    #${OMNIORB_LIBRARIES}
    ${OPENRTM_LIBRARIES}
    ${extralibraries_win}
    )
    set_target_properties(${exporter} PROPERTIES COMPILE_FLAGS "${COLLADA_DOM_CXX_FLAGS}" LINK_FLAGS "${COLLADA_DOM_LINK_FLAGS}")
  endif( COLLADA_DOM_FOUND )
endif()

if( COLLADA_DOM_FOUND )
  set_target_properties(${program} PROPERTIES COMPILE_FLAGS "${COLLADA_DOM_CXX_FLAGS}" LINK_FLAGS "${COLLADA_DOM_LINK_FLAGS}")
endif( COLLADA_DOM_FOUND )

add_executable(export-vrml ${sources3})

target_link_libraries(export-vrml
  hrpModel-${OPENHRP_LIBRARY_VERSION}
  hrpCorbaStubSkel-${OPENHRP_LIBRARY_VERSION}
  )

if(WIN32)
  install(TARGETS ${program} DESTINATION ${PROJECT_BINARY_DIR}/bin CONFIGURATIONS Release )
endif()

install(TARGETS ${program} DESTINATION bin CONFIGURATIONS Release Debug)
if( COLLADA_DOM_FOUND )
  install(TARGETS ${exporter} DESTINATION bin CONFIGURATIONS Release Debug)
endif( COLLADA_DOM_FOUND )
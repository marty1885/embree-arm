## ======================================================================== ##
## Copyright 2009-2016 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

SOURCE_GROUP("Source Files\\bvh4" "/bvh4/" )
SOURCE_GROUP("Source Files\\bvh8" "/bvh8/" )
SOURCE_GROUP("Source Files\\geometry" "/geometry/" )
SOURCE_GROUP("Source Files\\builders" "/builders.*/" )
SOURCE_GROUP("Source Files\\common" "/kernels/common/" )
SOURCE_GROUP("Source Files\\tasking" "/tasking/" )
SOURCE_GROUP("Source Files\\algorithms" "/algorithms/")
SOURCE_GROUP("" FILES embree.rc)

SET(EMBREE_LIBRARY_FILES
 
  embree.rc

  common/device.cpp
  common/stat.cpp
  common/accel.cpp
  common/acceln.cpp
  common/accelset.cpp
  common/state.cpp
  common/rtcore.cpp
  common/buffer.cpp
  common/scene.cpp
  common/alloc.cpp
  common/geometry.cpp
  common/tasksys.cpp
  common/scene_user_geometry.cpp
  common/scene_instance.cpp
  common/scene_geometry_instance.cpp
  common/scene_triangle_mesh.cpp
  common/scene_quad_mesh.cpp
  common/scene_bezier_curves.cpp
  common/scene_line_segments.cpp
  common/scene_subdiv_mesh.cpp
  subdiv/tessellation_cache.cpp
  subdiv/subdivpatch1base.cpp
  subdiv/catmullclark_coefficients.cpp
  subdiv/bezier_curve.cpp

  geometry/primitive.cpp
  geometry/instance_intersector1.cpp
  geometry/instance_intersector.cpp
  geometry/grid_soa.cpp	
  subdiv/subdivpatch1base_eval.cpp	
  builders/primrefgen.cpp

  bvh/bvh.cpp
  bvh/bvh_statistics.cpp
  bvh/bvh4_factory.cpp
  bvh/bvh8_factory.cpp

  bvh/bvh_rotate.cpp
  bvh/bvh_refit.cpp
  bvh/bvh_builder.cpp
  bvh/bvh_builder_hair.cpp
  bvh/bvh_builder_morton.cpp
  bvh/bvh_builder_sah.cpp
  bvh/bvh_builder_twolevel.cpp
  bvh/bvh_builder_instancing.cpp
  bvh/bvh_builder_subdiv.cpp

  bvh/bvh_intersector1.cpp
  )

IF (EMBREE_RAY_PACKETS)
  SET(EMBREE_LIBRARY_FILES ${EMBREE_LIBRARY_FILES}

  common/rtcore_ispc.cpp
  common/rtcore_ispc.ispc

  bvh/bvh_intersector_single.cpp
  bvh/bvh_intersector_hybrid_bvh4.cpp
  bvh/bvh_intersector_stream_filters.cpp)
ENDIF()

SET(EMBREE_LIBRARY_FILES_SSE42
    geometry/grid_soa.cpp
    subdiv/subdivpatch1base_eval.cpp
    bvh/bvh_intersector1.cpp)

IF (EMBREE_RAY_PACKETS)
    SET(EMBREE_LIBRARY_FILES_SSE42 ${EMBREE_LIBRARY_FILES_SSE42}
    bvh/bvh_intersector_single.cpp
    bvh/bvh_intersector_hybrid_bvh4.cpp
    bvh/bvh_intersector_stream.cpp
    bvh/bvh_intersector_stream_filters.cpp)
ENDIF()

SET(EMBREE_LIBRARY_FILES_AVX
    
    common/scene_subdiv_mesh_avx.cpp
  
    geometry/instance_intersector1.cpp
    
    geometry/grid_soa.cpp
    subdiv/subdivpatch1base_eval.cpp
    builders/primrefgen.cpp

    bvh/bvh_rotate.cpp
    bvh/bvh_refit.cpp
    bvh/bvh_builder.cpp
    bvh/bvh_builder_hair.cpp
    bvh/bvh_builder_morton.cpp
    bvh/bvh_builder_sah.cpp
    bvh/bvh_builder_twolevel.cpp
    bvh/bvh_builder_instancing.cpp
    bvh/bvh_builder_subdiv.cpp
    bvh/bvh_intersector1.cpp
    
    bvh/bvh.cpp
    bvh/bvh_statistics.cpp)

IF (EMBREE_RAY_PACKETS)
  SET(EMBREE_LIBRARY_FILES_AVX ${EMBREE_LIBRARY_FILES_AVX}

    geometry/instance_intersector.cpp

    bvh/bvh_intersector_single.cpp
    bvh/bvh_intersector_hybrid_bvh4.cpp
    bvh/bvh_intersector_hybrid_bvh8.cpp
    bvh/bvh_intersector_stream.cpp
    bvh/bvh_intersector_stream_filters.cpp)
ENDIF()

SET(EMBREE_LIBRARY_FILES_AVX2

    geometry/instance_intersector1.cpp
    geometry/grid_soa.cpp
    subdiv/subdivpatch1base_eval.cpp

    bvh/bvh_intersector1.cpp)

IF (EMBREE_RAY_PACKETS)
  SET(EMBREE_LIBRARY_FILES_AVX2 ${EMBREE_LIBRARY_FILES_AVX2}
    geometry/instance_intersector.cpp

    bvh/bvh_intersector_single.cpp
    bvh/bvh_intersector_hybrid_bvh4.cpp
    bvh/bvh_intersector_hybrid_bvh8.cpp
    bvh/bvh_intersector_stream.cpp
    bvh/bvh_intersector_stream_filters.cpp)
ENDIF()

SET(EMBREE_LIBRARY_FILES_AVX512

    geometry/instance_intersector1.cpp
    geometry/grid_soa.cpp    
    subdiv/subdivpatch1base_eval.cpp

    bvh/bvh_refit.cpp
    bvh/bvh_rotate.cpp
    bvh/bvh_builder_subdiv.cpp
    bvh/bvh_intersector1.cpp

    builders/primrefgen.cpp
    bvh/bvh_builder.cpp
    bvh/bvh_builder_sah.cpp
    bvh/bvh_builder_twolevel.cpp
    bvh/bvh_builder_instancing.cpp
    bvh/bvh_builder_morton.cpp
    bvh/bvh_builder_subdiv.cpp)

IF (EMBREE_RAY_PACKETS)
  SET(EMBREE_LIBRARY_FILES_AVX512 ${EMBREE_LIBRARY_FILES_AVX512}

    geometry/instance_intersector.cpp

    bvh/bvh_intersector_single.cpp
    bvh/bvh_intersector_hybrid_bvh4.cpp
    bvh/bvh_intersector_hybrid_bvh8.cpp
    bvh/bvh_intersector_stream.cpp
    bvh/bvh_intersector_stream_filters.cpp)
ENDIF()

SET(EMBREE_LIBRARY_FILES_AVX512KNL ${EMBREE_LIBRARY_FILES_AVX512})
SET(EMBREE_LIBRARY_FILES_AVX512SKX ${EMBREE_LIBRARY_FILES_AVX512})

# replaces all .cpp files with a dummy file that includes that .cpp file
# this is to work around an ICC name mangling issue related to lambda functions under windows
MACRO (CreateISADummyFiles list isa)
  SET(${list})
  FOREACH(src ${ARGN})
    SET(src_file ${CMAKE_CURRENT_SOURCE_DIR}/${src})
    SET(dst_file ${CMAKE_BINARY_DIR}/${src}.${isa}.cpp)
    SET(${list} ${${list}} ${dst_file})
    ADD_CUSTOM_COMMAND(OUTPUT ${dst_file}
      COMMAND ${CMAKE_COMMAND} 
       -D src=${src_file}
       -D dst=${dst_file}
       -P ${PROJECT_SOURCE_DIR}/common/cmake/create_isa_dummy_file.cmake
      DEPENDS ${src_file})
  ENDFOREACH()
ENDMACRO()
CreateISADummyFiles(EMBREE_LIBRARY_FILES_SSE42     sse42     ${EMBREE_LIBRARY_FILES_SSE42})
CreateISADummyFiles(EMBREE_LIBRARY_FILES_AVX       avx       ${EMBREE_LIBRARY_FILES_AVX})
CreateISADummyFiles(EMBREE_LIBRARY_FILES_AVX2      avx2      ${EMBREE_LIBRARY_FILES_AVX2})
CreateISADummyFiles(EMBREE_LIBRARY_FILES_AVX512KNL avx512knl ${EMBREE_LIBRARY_FILES_AVX512KNL})
CreateISADummyFiles(EMBREE_LIBRARY_FILES_AVX512SKX avx512skx ${EMBREE_LIBRARY_FILES_AVX512SKX})

ADD_ISPC_LIBRARY(embree ${EMBREE_LIB_TYPE} ${EMBREE_LIBRARY_FILES} $<TARGET_OBJECTS:algorithms>)
SET_PROPERTY(TARGET embree PROPERTY FOLDER kernels)

IF (TARGET_SSE42 AND EMBREE_LIBRARY_FILES_SSE42)
  ADD_LIBRARY(embree_sse42 STATIC ${EMBREE_LIBRARY_FILES_SSE42})
  SET_TARGET_PROPERTIES(embree_sse42 PROPERTIES COMPILE_FLAGS "${FLAGS_SSE42}")
  SET_PROPERTY(TARGET embree_sse42 PROPERTY FOLDER kernels)
  SET(EMBREE_LIBRARIES ${EMBREE_LIBRARIES} embree_sse42)
ENDIF ()

IF (TARGET_AVX  AND EMBREE_LIBRARY_FILES_AVX)
  ADD_LIBRARY(embree_avx STATIC ${EMBREE_LIBRARY_FILES_AVX})
  SET_TARGET_PROPERTIES(embree_avx PROPERTIES COMPILE_FLAGS "${FLAGS_AVX}")
  SET_PROPERTY(TARGET embree_avx PROPERTY FOLDER kernels)
  SET(EMBREE_LIBRARIES ${EMBREE_LIBRARIES} embree_avx)
 ENDIF()

IF (TARGET_AVX2 AND EMBREE_LIBRARY_FILES_AVX2)
  ADD_LIBRARY(embree_avx2 STATIC ${EMBREE_LIBRARY_FILES_AVX2})
  SET_TARGET_PROPERTIES(embree_avx2 PROPERTIES COMPILE_FLAGS "${FLAGS_AVX2}")
  SET_PROPERTY(TARGET embree_avx2 PROPERTY FOLDER kernels)
  SET(EMBREE_LIBRARIES ${EMBREE_LIBRARIES} embree_avx2)
ENDIF()

IF (TARGET_AVX512KNL AND EMBREE_LIBRARY_FILES_AVX512KNL)
  ADD_LIBRARY(embree_avx512knl STATIC ${EMBREE_LIBRARY_FILES_AVX512KNL})
  SET_TARGET_PROPERTIES(embree_avx512knl PROPERTIES COMPILE_FLAGS "${FLAGS_AVX512KNL}")
  SET_PROPERTY(TARGET embree_avx512knl PROPERTY FOLDER kernels)
  SET(EMBREE_LIBRARIES ${EMBREE_LIBRARIES} embree_avx512knl)
ENDIF()

IF (TARGET_AVX512SKX AND EMBREE_LIBRARY_FILES_AVX512SKX)
  ADD_LIBRARY(embree_avx512skx STATIC ${EMBREE_LIBRARY_FILES_AVX512SKX})
  SET_TARGET_PROPERTIES(embree_avx512skx PROPERTIES COMPILE_FLAGS "${FLAGS_AVX512SKX}")
  SET_PROPERTY(TARGET embree_avx512skx PROPERTY FOLDER kernels)
  SET(EMBREE_LIBRARIES ${EMBREE_LIBRARIES} embree_avx512skx)
ENDIF()

TARGET_LINK_LIBRARIES(embree ${EMBREE_LIBRARIES} sys simd lexers tasking)

IF (WIN32)
ELSEIF (APPLE)
  SET_TARGET_PROPERTIES(embree PROPERTIES LINK_FLAGS -Wl,-exported_symbols_list,${PROJECT_SOURCE_DIR}/kernels/export.macosx.map)
  SET_SOURCE_FILES_PROPERTIES(common/rtcore.cpp PROPERTIES OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/kernels/export.macosx.map) 
ELSE()
  SET_TARGET_PROPERTIES(embree PROPERTIES LINK_FLAGS -Wl,--version-script=${PROJECT_SOURCE_DIR}/kernels/export.linux.map)
  SET_SOURCE_FILES_PROPERTIES(common/rtcore.cpp PROPERTIES OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/kernels/export.linux.map)
ENDIF()

IF (EMBREE_ZIP_MODE)
  SET_TARGET_PROPERTIES(embree PROPERTIES VERSION ${EMBREE_VERSION_MAJOR} SOVERSION ${EMBREE_VERSION_MAJOR})
ELSE()
  SET_TARGET_PROPERTIES(embree PROPERTIES VERSION ${EMBREE_VERSION} SOVERSION ${EMBREE_VERSION_MAJOR})
ENDIF()

IF (WIN32)
  INSTALL(TARGETS embree DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib)
ELSE()
  INSTALL(TARGETS embree DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib)
ENDIF()

IF (WIN32)
  INSTALL(TARGETS embree DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT examples)
ENDIF()

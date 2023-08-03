set(CORE_SOURCES
  core/Matrix4.cpp
  core/material.cpp
  core/BBox.cpp
  core/shape.cpp)

set(CAMERA_SOURCES)
set(INTEGRATOR_SOURCES)
set(LIGHT_SOURCES)
set(MATERIAL_SOURCES)
set(RENDERER_SOURCES)

add_library(miao
  ${CORE_SOURCES}
  ${CAMERA_SOURCES}
  ${INTEGRATOR_SOURCES}
  ${LIGHT_SOURCES}
  ${MATERIAL_SOURCES}
  ${RENDERER_SOURCES}
  )

set(CORE_SOURCES
  core/Matrix4.cpp
  core/material.cpp
  core/BBox.cpp
  core/primitive.cpp
  core/shape.cpp)

set(CAMERA_SOURCES
  cameras/camera.cpp
  cameras/film.cpp)
set(INTEGRATOR_SOURCES
  integrators/integrator.cpp)
set(LIGHT_SOURCES)
set(MATERIAL_SOURCES)
set(RENDERER_SOURCES
  renderers/progressive.cpp)

add_library(miao STATIC
  ${CORE_SOURCES}
  ${CAMERA_SOURCES}
  ${INTEGRATOR_SOURCES}
  ${LIGHT_SOURCES}
  ${MATERIAL_SOURCES}
  ${RENDERER_SOURCES}
  )

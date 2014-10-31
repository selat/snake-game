set(SRCS
  main.c
  glslprogram.c
  scene.c
  smm.c
  srm.c
)

add_executable(${PROJECT_NAME} ${SRCS})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX _d)

target_link_libraries(${PROJECT_NAME} ${${PROJECT_NAME}_ADDITIONAL_LIBS})
set(DOCKER_MAKE_ARGS -j8 CACHE STRING "GCC builder make extra arguments")

execute_process(
    COMMAND sh -c "id -u $ENV{USER}"
    OUTPUT_VARIABLE UID
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE RETC)

execute_process(
    COMMAND sh -c "id -g $ENV{USER}"
    OUTPUT_VARIABLE GID
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE RETC)

add_custom_target(xdev-environment
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/build-docker
  COMMAND
    docker build . -f ${CMAKE_CURRENT_LIST_DIR}/Dockerfile
      --rm=false
      --build-arg MAKE_ARGS=${DOCKER_MAKE_ARGS}
      -t xdev-environment:latest
  COMMAND
    ${CMAKE_COMMAND} -E env UID=${UID} GID=${GID} SOURCE_DIR=${CMAKE_SOURCE_DIR}
      docker-compose -f ${CMAKE_CURRENT_LIST_DIR}/docker-compose.yml up -d xdev-builder
  COMMAND
    docker exec -it xdev-builder bash
  # COMMAND
  #   ${CMAKE_COMMAND} -E env UID=${UID} GID=${GID} SOURCE_DIR=${CMAKE_SOURCE_DIR}
  #     docker-compose -f ${CMAKE_CURRENT_LIST_DIR}/docker-compose.yml run xdev-builder
)

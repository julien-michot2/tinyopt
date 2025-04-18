set(TINYOPT_VERSION_MAJOR 0)
set(TINYOPT_VERSION_MINOR 5)
set(TINYOPT_VERSION_PATCH 0)

math(EXPR TINYOPT_VERSION "10000 * ${TINYOPT_VERSION_MAJOR} + 100 * ${TINYOPT_VERSION_MINOR} + ${TINYOPT_VERSION_PATCH}")
set(TINYOPT_VERSION_STRING "${TINYOPT_VERSION_MAJOR}.${TINYOPT_VERSION_MINOR}.${TINYOPT_VERSION_PATCH}")

set (CMAKE_PROJECT_VERSION_MAJOR ${TINYOPT_VERSION_MAJOR})
set (CMAKE_PROJECT_VERSION_MINOR ${TINYOPT_VERSION_MINOR})
set (CMAKE_PROJECT_VERSION_PATCH ${TINYOPT_VERSION_PATCH})


add_library(FrameworkMod INTERFACE)
target_link_libraries(FrameworkMod INTERFACE FrameworkCore FrameworkFrontend IRGenerator)

include_directories(include)
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(FrameworkMod INTERFACE "-pthread")

add_subdirectory(core)
add_subdirectory(frontend)
add_subdirectory(framework_ir)

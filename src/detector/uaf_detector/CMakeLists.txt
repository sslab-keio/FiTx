add_library(UAFDetectorMod MODULE
    UAF_Detector.cpp
)

#Use C++ 11 to compile our pass(i.e., supply - std = c++ 11).
target_compile_features(UAFDetectorMod PRIVATE cxx_range_for cxx_auto_type cxx_std_17)

#LLVM is(typically) built with no C++ RTTI.We need to match that;
#otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(UAFDetectorMod PROPERTIES COMPILE_FLAGS "-fno-rtti")
include_directories(include ${FRAMEWORK_DIR}/include ${DETECTOR_DIR}/include)

target_link_libraries(UAFDetectorMod FrameworkMod UAFUtils)

# UAFUtils
add_library(UAFUtils STATIC UAFUtils.cpp)
target_compile_features(UAFUtils PRIVATE cxx_range_for cxx_auto_type cxx_std_17)
set_target_properties(UAFUtils PROPERTIES COMPILE_FLAGS "-fno-rtti -fPIC")
target_link_libraries(UAFUtils FrameworkMod)

#Get proper shared - library behavior(where symbols are not necessarily
#resolved when the shared library is linked) on OS X.
if (APPLE)
set_target_properties(
    UAFDetectorMod PROPERTIES LINK_FLAGS
    "-undefined dynamic_lookup")
endif(APPLE)

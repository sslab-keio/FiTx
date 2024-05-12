add_library(DULDetectorMod MODULE
    DUL_Detector.cpp
)

#Use C++ 11 to compile our pass(i.e., supply - std = c++ 11).
target_compile_features(DULDetectorMod PRIVATE cxx_range_for cxx_auto_type cxx_std_17)

#LLVM is(typically) built with no C++ RTTI.We need to match that;
#otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(DULDetectorMod PROPERTIES COMPILE_FLAGS "-fno-rtti")
include_directories(include ${FRAMEWORK_DIR}/include ${DETECTOR_DIR}/include)

target_link_libraries(DULDetectorMod FrameworkMod DULUtils)

# DLUtils
add_library(DULUtils STATIC DULUtils.cpp)
target_compile_features(DULUtils PRIVATE cxx_range_for cxx_auto_type cxx_std_17)
set_target_properties(DULUtils PROPERTIES COMPILE_FLAGS "-fno-rtti -fPIC")
target_link_libraries(DULUtils FrameworkMod)

#Get proper shared - library behavior(where symbols are not necessarily
#resolved when the shared library is linked) on OS X.
if (APPLE)
set_target_properties(
    DULDetectorMod PROPERTIES LINK_FLAGS
    "-undefined dynamic_lookup")
endif(APPLE)

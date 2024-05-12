add_library(IRGenerator SHARED
    Analyzer.cpp
    IRGenerator.cpp 
    Utils.cpp
)

#Use C++ 11 to compile our pass(i.e., supply - std = c++ 11).
target_compile_features(IRGenerator PRIVATE cxx_range_for cxx_auto_type cxx_std_17)

#LLVM is(typically) built with no C++ RTTI.We need to match that;
#otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(IRGenerator PROPERTIES COMPILE_FLAGS "-fno-rtti")
include_directories(${INCLUDE_DIR}/framework_ir)

target_link_libraries(IRGenerator FrameworkCore)

#Get proper shared - library behavior(where symbols are not necessarily
#resolved when the shared library is linked) on OS X.
if (APPLE)
set_target_properties(
    IRGenerator PROPERTIES LINK_FLAGS
    "-undefined dynamic_lookup")
endif(APPLE)

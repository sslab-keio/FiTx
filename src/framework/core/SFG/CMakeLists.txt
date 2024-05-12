add_library(StateFlowGraphLib STATIC
    Converter.cpp
)

#Use C++ 11 to compile our pass(i.e., supply - std = c++ 11).
target_compile_features(StateFlowGraphLib PRIVATE cxx_range_for cxx_auto_type cxx_std_17)

#LLVM is(typically) built with no C++ RTTI.We need to match that;
#otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(StateFlowGraphLib PROPERTIES COMPILE_FLAGS "-fno-rtti -fPIC")

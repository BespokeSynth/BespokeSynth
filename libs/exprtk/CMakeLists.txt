add_library(exprtk INTERFACE)
target_include_directories(exprtk INTERFACE include)
add_library(bespoke::exprtk ALIAS exprtk)

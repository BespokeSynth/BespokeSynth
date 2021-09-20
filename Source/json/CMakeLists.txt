project(json VERSION 0.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME}
        include/json/autolink.h
        include/json/config.h
        include/json/features.h
        include/json/forwards.h
        include/json/json.h
        include/json/reader.h
        include/json/value.h
        include/json/writer.h
        src/lib_json/json_batchallocator.h
        src/lib_json/json_internalarray.inl
        src/lib_json/json_internalmap.inl
        src/lib_json/json_reader.cpp
        src/lib_json/json_value.cpp
        src/lib_json/json_valueiterator.inl
        src/lib_json/json_writer.cpp
        src/lib_json/sconscript
        README.txt
        version
        )

target_include_directories(${PROJECT_NAME} PUBLIC include)

add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
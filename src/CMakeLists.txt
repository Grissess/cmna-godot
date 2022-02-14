add_library(cmna-godot SHARED cmna_godot.c "${cmna-godot_SOURCE_DIR}/godot-headers/gdnative_api_struct.gen.h")
target_include_directories(cmna-godot PRIVATE "${cmna-godot_SOURCE_DIR}/godot-headers/" cmna::cmna-shared)
target_compile_options(cmna-godot PRIVATE -std=c11)
target_link_libraries(cmna-godot PRIVATE cmna::cmna-shared)

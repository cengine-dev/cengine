# -------------------------------------------------------------
# cengine::grid2d — módulo opcional de conversão célula <-> pixel (task 26)
#
# Não depende de core nem de routing: é aritmética pura, em PIXELS. Vizinho da
# camera2d (que trabalha em unidades de mundo) e não concorrente dela. O jogo
# escolhe linkar — ADR 0001, core mínimo + módulos opt-in.
# -------------------------------------------------------------
add_library(cengine_grid2d STATIC
    src/Grid.cpp
)
add_library(cengine::grid2d ALIAS cengine_grid2d)

target_include_directories(cengine_grid2d PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(cengine_grid2d PUBLIC cxx_std_23)

set_target_properties(cengine_grid2d PROPERTIES OUTPUT_NAME "cengine_grid2d")

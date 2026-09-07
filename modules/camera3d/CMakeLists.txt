# -------------------------------------------------------------
# cengine::camera3d — módulo opcional de câmera 3D (mecanismo)
#
# Matrizes de vista/projeção e a mira inversa (pixel -> ponto do mundo).
# NÃO contém seguimento — para onde a câmera olha é política de cada jogo, o
# mesmo corte da camera2d (task 23). Sem dependência de outro módulo: os tipos
# (Vec3/Mat4) são o vocabulário mínimo, declarados aqui. O jogo escolhe linkar
# (ou não) — ADR 0001.
# -------------------------------------------------------------
add_library(cengine_camera3d STATIC
    src/Camera.cpp
)
add_library(cengine::camera3d ALIAS cengine_camera3d)

target_include_directories(cengine_camera3d PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Propaga o requisito de padrão C++ para quem linkar o módulo (ver tarefa 07).
target_compile_features(cengine_camera3d PUBLIC cxx_std_23)

set_target_properties(cengine_camera3d PROPERTIES OUTPUT_NAME "cengine_camera3d")

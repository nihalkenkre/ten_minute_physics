project(tmp_vk LANGUAGES C CXX)

file(GLOB PROJECT_SRCS src/c/* src/vma.cpp src/gui.cpp)
list(FILTER PROJECT_SRCS EXCLUDE REGEX "imgui.ini")
file(GLOB IMGUI_SRCS ../imgui/*.cpp ../imgui/*.h ../imgui/backends/imgui_impl_sdl3.* ../imgui/backends/imgui_impl_vulkan.*)
file(GLOB IMGUI_FILE_DLG_SRCS ../ImGuiFileDialog/ImGuiFileDialog*.h ../ImGuiFileDialog/ImGuiFileDialog*.cpp)
file(GLOB VOLK_SRCS volk/volk.c)

source_group("ImGui" FILES ${IMGUI_SRCS})
source_group("ImGuiFileDialog" FILES ${IMGUI_FILE_DLG_SRCS})

add_executable(${PROJECT_NAME} ${PROJECT_SRCS} ${IMGUI_SRCS} ${IMGUI_FILE_DLG_SRCS} ${VOLK_SRCS})

add_compile_definitions(_HAS_EXCEPTIONS=0 SDL_MAIN_USE_CALLBACKS IMGUI_DEFINE_MATH_OPERATORS _CRT_SECURE_NO_WARNINGS GLM_FORCE_ZERO_TO_ONE GLM_ENABLE_EXPERIMENTAL WIN32_LEAN_AND_MEAN)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
   target_compile_options(${PROJECT_NAME} PRIVATE "/EHs-" "/EHc-" "/GR-")
   add_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
   target_compile_options(${PROJECT_NAME} PRIVATE "-fno-exceptions" "-fno-rtti")
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 17)

#target_precompile_headers(${PROJECT_NAME} PRIVATE 
#                                       <vector>
#                                       <memory>
#                                       <glm/ext/matrix_clip_space.hpp>
#                                       <glm/ext/matrix_transform.hpp>
#                                       <glm/gtc/type_ptr.hpp>
#                          )
target_include_directories(${PROJECT_NAME} PRIVATE ../SDL/include ../imgui ../imgui/backends ../ImGuiFileDialog ../glm ../cglm/include ../cgltf ../stb src/c ../../Vulkan-Headers/include. volk vma/include)

target_link_directories(${PROJECT_NAME} PRIVATE $<IF:$<CONFIG:Debug>,${CMAKE_SOURCE_DIR}/SDL/build/Debug,> $<IF:$<CONFIG:MinSizeRel>,${CMAKE_SOURCE_DIR}/SDL/build/MinSizeRel,>)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3-static winmm imm32 setupapi version)
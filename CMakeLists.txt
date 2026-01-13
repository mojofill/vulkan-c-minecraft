cmake_minimum_required(VERSION 3.21)
project(vk_app C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

find_package(glfw3 CONFIG REQUIRED)

add_executable(
    vk_app 
    ./src/main.c
    ./src/renderer.c
    ./src/camera.c
    ./src/game.c
    ./src/swapchain.c
    ./src/graphics_pipeline.c
    ./src/commands.c
    ./src/buffer.c
    ./src/depth_buffer.c
    ./src/image.c
    ./src/stb_image_impl.c
)

target_include_directories(vk_app PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cglm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(vk_app PRIVATE
    Vulkan::Vulkan
    glfw
)

# Windows: avoid console popup if you want a window-only app
# set_target_properties(vk_app PROPERTIES WIN32_EXECUTABLE TRUE)

cmake_minimum_required(VERSION 3.21)
project(vk_app C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(vk_app
    src/main.c

    src/renderer/renderer.c
    src/renderer/swapchain.c
    src/renderer/graphics_pipeline.c
    src/renderer/commands.c
    src/renderer/buffer.c
    src/renderer/depth_buffer.c
    src/renderer/image.c
    src/renderer/stb_image_impl.c

    src/mesh/chunk_mesh.c
    src/mesh/mesh_pool.c
    
    src/world/camera.c
    src/world/world.c
    src/world/chunk.c
    src/world/chunk_map.c
    src/world/chunk_pool.c
    src/world/block.c
    
    src/streamer/streamer.c
)

target_include_directories(vk_app PRIVATE
    ${Vulkan_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cglm/include
)

target_link_libraries(vk_app PRIVATE
    Vulkan::Vulkan
    glfw
)

# macOS needs this so MoltenVK can create a window
if(APPLE)
    target_compile_definitions(vk_app PRIVATE VK_USE_PLATFORM_MACOS_MVK)
endif()

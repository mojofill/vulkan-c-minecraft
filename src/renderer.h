#ifndef RENDERER_H
#define RENDERER_H

#include "vk_types.h"
#include "swapchain.h"
#include "graphics_pipeline.h"
#include "commands.h"
#include "buffer.h"
#include "depth_buffer.h"
#include "image.h"

typedef struct Game Game; // forward

#define MAX_FRAMES_IN_FLIGHT 2

static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
static void initWindow(vk_context *vko);
static void createInstance(vk_context *vko);
static void createSurface(vk_context *vko);
static void pickPhysicalDevice(vk_context *vko);
static void findQueueFamilies(vk_context *vko);
static void createLogicalDevice(vk_context *vko);
static void createRenderpass(vk_context *vko);
static void createDescriptorSetLayout(vk_context *vko);
static void createUniformBuffers(vk_context *vko);
static void createDescriptorPool(vk_context *vko);
static void createDescriptorSets(vk_context *vko);
static void createSyncObjects(vk_context *vko);
static void initVulkan(vk_context *vko, VertexBufferContext *vbo);
static void updateCameraUniforms(vk_context *vko, uint32_t currentFrame);
static void drawFrame(vk_context *vko, uint32_t *currentFrame);
static void mainLoop(vk_context *vko);
static void cleanup(vk_context *vko);
void run(vk_context *vko);

#endif
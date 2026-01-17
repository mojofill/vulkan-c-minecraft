#ifndef VK_TYPES_H
#define VK_TYPES_H

#include "stb_image.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_beta.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "geometries/vertex.h"

typedef struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

typedef struct VertexBufferContext {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    uint32_t indexCount;
    int vertexCount;
    VkVertexInputBindingDescription bindingDesc; // for now this is good enough. in the future with more vertices i need a better system
    VkVertexInputAttributeDescription attrDescs[3]; // for now only two attributes
    char *test;
} VertexBufferContext;

typedef struct vk_context {
    uint64_t start_time;

    VkDebugUtilsMessengerEXT debugMessenger;

    // physical + logical devices
    GLFWwindow *window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    uint32_t queueFamilyCount;
    uint32_t graphicsFamilyIndex; // any queue of the graphics or present family also supports transfering data
    uint32_t presentFamilyIndex;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    // swapchain and swapchain support
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkSwapchainKHR swapchain;
    uint32_t swapchainImageCount;
    VkImage *swapchainImages;
    VkImageView *swapchainImageViews;

    // vertex buffer context
    VertexBufferContext *vbo;

    // render pass + graphics pipeline
    VkRenderPass renderPass;
    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer *swapchainFramebuffers;

    // desciptors (uniforms)
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;

    // uniform buffers (model + view + proj mat4)
    VkBuffer *cameraUniformBuffer;
    VkDeviceMemory *cameraUniformBufferMemories;
    void* *cameraUniformBufferMapped; // create pointer instantly after gpu memory creation for a persistent pointer for use later
    uint32_t uniformBufferCount;

    // commands
    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;

    // dynamic pipeline bindings
    VkViewport viewport;
    VkRect2D scissor;

    // sync objects
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;
    int framebufferResized;

    // textures. in the very near future i have to create an array of these or somehow keep track of all my textures
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    // depth buffering
    VkImage depthImage; // only need one, because only one draw operation at a time. do not need for all MAX_FRAMES_IN_FLIGHT
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
} vk_context;

#endif
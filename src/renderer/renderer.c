#include "renderer.h"
#include <time.h>
#include <stdint.h>

uint64_t time_ms() {
    // static LARGE_INTEGER freq;
    // static int initialized = 0;

    // if (!initialized) {
    //     QueryPerformanceFrequency(&freq);
    //     initialized = 1;
    // }

    // LARGE_INTEGER counter;
    // QueryPerformanceCounter(&counter);

    // return (uint64_t)((counter.QuadPart * 1000ULL) / freq.QuadPart);
    return 0;
}

static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    vk_context *vko = glfwGetWindowUserPointer(window);
    vko->framebufferResized = 1;
}

static void initWindow(vk_context *vko) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    vko->window = glfwCreateWindow(800, 600, "Vulkan Tutorial", NULL, NULL);
    if (!vko->window) {
        fprintf(stderr, "Failed to create GLFW window\n");
    }

    glfwSetWindowUserPointer(vko->window, vko); // yoo interesting. glfw itself holds a special pointer just for me
    glfwSetFramebufferSizeCallback(vko->window, framebufferResizeCallback);
    glfwMaximizeWindow(vko->window);
}

static void createInstance(vk_context *vko) {
    VkApplicationInfo appInfo = {0}; 
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan minecraft";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;
    
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    uint32_t glfwExtensionCount;
    
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    uint32_t extensionCount = glfwExtensionCount + 1;
    const char **extensions = malloc(sizeof(char*) * extensionCount);

    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        extensions[i] = glfwExtensions[i];
    }

    #ifdef __APPLE__
        extensions[glfwExtensionCount] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
    #endif
    
    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
    
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = 1;
    // validation layers 
    createInfo.ppEnabledLayerNames = layers;
    #ifdef __APPLE__
        createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    #endif

    if (vkCreateInstance(&createInfo, NULL, &vko->instance) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan instance.\n"); exit(1);
    } 

    // initializations
    vko->framebufferResized = 0;
    vko->start_time = time_ms();
}

static void createSurface(vk_context *vko) {
    if (glfwCreateWindowSurface(vko->instance, vko->window, NULL, &vko->surface) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create surface\n");
        exit(1);
    }
}

static void pickPhysicalDevice(vk_context *vko) {
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(vko->instance, &physicalDeviceCount, NULL);
    VkPhysicalDevice *physicalDevices = malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    vkEnumeratePhysicalDevices(vko->instance, &physicalDeviceCount, physicalDevices);

    vko->physicalDevice = physicalDevices[0];
    free(physicalDevices);
}

static void findQueueFamilies(vk_context *vko) {
    vkGetPhysicalDeviceQueueFamilyProperties(vko->physicalDevice, &vko->queueFamilyCount, NULL);
    VkQueueFamilyProperties *queueFamilyProperties = malloc(sizeof(VkQueueFamilyProperties) * vko->queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vko->physicalDevice, &vko->queueFamilyCount, queueFamilyProperties);

    vko->graphicsFamilyIndex = -1;
    vko->presentFamilyIndex = -1;

    for (uint32_t i = 0; i < vko->queueFamilyCount; i++) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            vko->graphicsFamilyIndex = i;
        }

        VkBool32 presentSupport;
        vkGetPhysicalDeviceSurfaceSupportKHR(vko->physicalDevice, i, vko->surface, &presentSupport);
        if (presentSupport) {
            vko->presentFamilyIndex = i;
        }

        if (vko->graphicsFamilyIndex != -1 && vko->presentFamilyIndex != -1) break;
    }

    if (vko->graphicsFamilyIndex == -1 || vko->presentFamilyIndex == -1) {
        fprintf(stderr, "Failed to find required queue families\ngraphics family: %d\npresent family: %d", vko->graphicsFamilyIndex, vko->presentFamilyIndex);
        exit(1);
    }
}

static void createLogicalDevice(vk_context *vko) {
    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME }; // must add swapchain extension

    // create device queue
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfos[2] = {0};
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = vko->graphicsFamilyIndex;
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {0};
    deviceFeatures.samplerAnisotropy = VK_TRUE; // must add device feature sampler anisotropy

    // create logical device
    VkDeviceCreateInfo deviceCreateInfo = {0};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.enabledExtensionCount = 2;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(vko->physicalDevice, &deviceCreateInfo, NULL, &vko->device) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device\n");
        exit(1);
    }

    // must grab queue handle from logical device
    vkGetDeviceQueue(vko->device, vko->graphicsFamilyIndex, 0, &vko->graphicsQueue);
}

static void createRenderpass(vk_context *vko) {
    // color attachment = placeholder/variable for image view
    // image = raw memory source
    // image view = structured way to view/access image/raw memory
    // (color) attachment = specific role an image view (processed image data) plays in a render pass
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = vko->surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear previous data
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // image will be used for presentation

    // image layout = physical memory arrangement (what bytes go where)

    VkAttachmentReference colorAttachmentRef = {0}; // this is how a subpass gets access to the attachment
    colorAttachmentRef.attachment = 0; // pAttachments[0] - there is an array of attachments, our attachment index is 0 b/c we only have one attachment right now
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // create the subpass. right now, we just have one subpass. this is the vertex -> fragment stuff
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // running GRAPHICS pipeline - not a compute pipeline
    subpass.colorAttachmentCount = 1; // one SINGULAR color output, which is attachment #0. attachment = "logical" i
    subpass.pColorAttachments = &colorAttachmentRef; // array decay, for single length array just pointer to first

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // think negative 1 when srcSubpass is VK_SUBPASS_EXTERNAL, and max+1 when it is dstSubpass (dst = destination)
    dependency.dstSubpass = 0; // this is for the first implicit subpass from initial layout to render pass layout
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // only when subpass 0 finished can pipeline move onto this stage
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // tells last implicit subpass to wait for subpass 0 t finish before writing to color attachment

    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(vko->device, &renderPassInfo, NULL, &vko->renderPass) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create render pass\n");
        exit(1);
    }
}

static void createDescriptorSetLayout(vk_context *vko) {
    // create layout binding (for vertex shader access)
    // these are camera uniforms
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    uboLayoutBinding.binding = 0; // layout (binding = 0) uniform UniformBufferObject... in vertex shader
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // adding into vertex shader
    uboLayoutBinding.pImmutableSamplers = NULL; // only for image samplers

    // sampler uniform (textures)
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {0};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[2] = {uboLayoutBinding, samplerLayoutBinding};

    // create descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(vko->device, &layoutInfo, NULL, &vko->descriptorSetLayout) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create descriptor set layout\n");
        exit(1);
    }
}

static void createcameraUniformBuffer(vk_context *vko) {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject); // should make a better name for this - like cameraUniforms or something

    vko->uniformBufferCount = MAX_FRAMES_IN_FLIGHT;
    vko->cameraUniformBuffer = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkBuffer));
    vko->cameraUniformBufferMemories = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkDeviceMemory));
    vko->cameraUniformBufferMapped = malloc(MAX_FRAMES_IN_FLIGHT * bufferSize); // i think so?
    
    for (uint32_t i = 0; i < vko->uniformBufferCount; i++) {
        createBuffer(vko, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vko->cameraUniformBuffer[i], &vko->cameraUniformBufferMemories[i]);

        // persistent mapping - create pointer that persists throughout application lifetime for ease of access
        vkMapMemory(vko->device, vko->cameraUniformBufferMemories[i], 0, bufferSize, 0, &vko->cameraUniformBufferMapped[i]);
    }
}

static void createDescriptorPool(vk_context *vko) {
    VkDescriptorPoolSize poolSizes[2];
    // camera uniforms
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;
    // sampler
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = (uint32_t) 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = (uint32_t) MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(vko->device, &poolInfo, NULL, &vko->descriptorPool) != VK_SUCCESS) {
        fprintf(stderr, "failed to create descriptor pool!");
        exit(1);
    }
}

static void createDescriptorSets(vk_context *vko) {
    VkDescriptorSetAllocateInfo allocInfo = {0};

    // need identical layouts for each MAX_FRAMES_IN_FLIGHT descriptor set
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT] = {0};
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = vko->descriptorSetLayout;

    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vko->descriptorPool;
    allocInfo.descriptorSetCount = (uint32_t) MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    vko->descriptorSets = malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkDescriptorSet));

    if (vkAllocateDescriptorSets(vko->device, &allocInfo, vko->descriptorSets) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create descriptor sets\n");
        exit(1);
    }

    // populate descriptors (uniform buffers)
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // add buffer to descriptor

        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = vko->cameraUniformBuffer[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {0};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vko->textureImageView;
        imageInfo.sampler = vko->textureSampler;

        // write to descriptor
        VkWriteDescriptorSet descriptorWrites[2] = {0};

        // camera uniform
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = vko->descriptorSets[i]; // write to this descriptor set
        descriptorWrites[0].dstBinding = 0; // write to this binding (binding = 0)
        descriptorWrites[0].dstArrayElement = 0; // can have descriptor arrays, but we are not, so put first; ie index=0
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1; // only updating one descriptor
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = NULL; // Optional
        descriptorWrites[0].pTexelBufferView = NULL; // Optional

        // sampler
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = vko->descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(vko->device, (uint32_t) 2, descriptorWrites, 0, NULL);
    }
}

static void createSyncObjects(vk_context *vko) {
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // signal start so that main loop doesnt keep waiting for it to start

    // allocate space for the sync objects
    vko->imageAvailableSemaphores = malloc(sizeof(VkSemaphore) * vko->swapchainImageCount);
    vko->renderFinishedSemaphores = malloc(sizeof(VkSemaphore) * vko->swapchainImageCount);
    vko->inFlightFences = malloc(sizeof(VkFence) * vko->swapchainImageCount);

    for (uint32_t i = 0; i < 2; i++) {
        if (vkCreateSemaphore(vko->device, &semaphoreInfo, NULL, &vko->imageAvailableSemaphores[i]) != VK_SUCCESS || 
            vkCreateSemaphore(vko->device, &semaphoreInfo, NULL, &vko->renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vko->device, &fenceInfo, NULL, &vko->inFlightFences[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create sync objects.\n");
            exit(1);
        }
    }
}

static void initVulkan(vk_context *vko, VertexBufferContext *vbo) {
    createInstance(vko);
    pickPhysicalDevice(vko);
    createSurface(vko);
    findQueueFamilies(vko);
    createLogicalDevice(vko);
    createSwapchain(vko);
    createImageViews(vko); // image view = how render pipeline accesses swapchain images

    createCommandPool(vko); // because of staging to vertex buffer copy command, must create command pool before creating vertex buffer context
    createTextureImage(vko, "./src/renderer/king_trump_cropped.png");
    createTextureImageView(vko);
    createTextureSampler(vko);
    createVertexBufferContext(vko, vbo);

    // fixed function stages = can tweak behavior via parameters, but the way they work is predefined (ie vulkan does the dirty work for us)
    // 3 fixed function stages we will make = input assembler, rasterization, and color blending
    // programmable stages = shaders

    // need to do a createVertexBuffers() call BEFORE creating graphics pipeline
    createRenderpass(vko);
    createcameraUniformBuffer(vko); // actually populates data into uniform layouts
    createDescriptorSetLayout(vko); // creates uniform layouts
    createDescriptorPool(vko);
    createDescriptorSets(vko); // binds memory to descriptors
    createGraphicsPipeline(vko);
    createFramebuffers(vko);
    createCommandBuffers(vko);

    createSyncObjects(vko);
}

void drawFrame(vk_context *vko, uint32_t *currentFrame, MeshPool pool) {
    // wait for previous frame to finish
    vkWaitForFences(vko->device, 1, &vko->inFlightFences[*currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(vko->device, vko->swapchain, UINT64_MAX, vko->imageAvailableSemaphores[*currentFrame], VK_NULL_HANDLE, &imageIndex); // signal imageAvailableSemaphore when image is available.

    if (result == VK_ERROR_OUT_OF_DATE_KHR) { // usually happens after window resize
        recreateSwapchain(vko);
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        fprintf(stderr, "Failed to acquire swapchain image.\n");
        exit(1);
    }

    // only reset to use fence again if we are submitting work
    vkResetFences(vko->device, 1, &vko->inFlightFences[*currentFrame]);

    // if i were recording every frame, i would need to 1. vkResetCommandBuffer(commandBuffer, 0); to clear command buffer and 2. recordCommandBuffer(commandBuffer, imageIndex);
    // but since everything is pre recorded, i dont need to

    recordCommands(vko, *currentFrame, pool);

    // submit commands to queue
    VkSubmitInfo submitInfo = {0};
    // explicitly tell gpu to wait until imageAvailableSemaphore signals ready before pipeline outputs to color attachment
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; // first stage synced first semaphore.
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vko->commandBuffers[imageIndex];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vko->imageAvailableSemaphores[*currentFrame]; // wait for imageAvailable semaphore to signal done before rendering
    submitInfo.pWaitDstStageMask = waitStages; // pipeline stages are synced with wait semaphores - doesnt continue onto stage until wait semaphore signals done
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vko->renderFinishedSemaphores[*currentFrame]; // signal renderFinished semaphore when done

    if (vkQueueSubmit(vko->graphicsQueue, 1, &submitInfo, vko->inFlightFences[*currentFrame]) != VK_SUCCESS) {
        fprintf(stderr, "Failed to submit draw command buffer to graphics queue\n");
        exit(1);
    }

    // 3. Present the rendered image
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vko->renderFinishedSemaphores[*currentFrame]; // wait until rendering finished
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vko->swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    result = vkQueuePresentKHR(vko->graphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vko->framebufferResized) {
        vko->framebufferResized = 0;
        recreateSwapchain(vko);
        // rerecord command buffers
        resetCommands(vko);
        recordCommands(vko, *currentFrame, pool);
    } else if (result != VK_SUCCESS) {
        printf("Failed to present swapchain image!\n");
        exit(1);
    }

    *currentFrame = (*currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    // Optional: wait for queue idle (simpler for learning)
    vkQueueWaitIdle(vko->graphicsQueue);
}

void cleanupRenderer(vk_context *vko) {
    // clock_t start, end;
    // printf("starting time diagnostic...\n");
    // start = clock();
    vkDestroySampler(vko->device, vko->textureSampler, NULL);
    vkDestroyImage(vko->device, vko->textureImage, NULL);
    vkDestroyImageView(vko->device, vko->textureImageView, NULL);
    vkFreeMemory(vko->device, vko->textureImageMemory, NULL);
    for (uint32_t i = 0; i < vko->uniformBufferCount; i++) {
        vkDestroyBuffer(vko->device, vko->cameraUniformBuffer[i], NULL);
        vkFreeMemory(vko->device, vko->cameraUniformBufferMemories[i], NULL);
    }
    free(vko->cameraUniformBuffer);
    free(vko->cameraUniformBufferMemories);
    free(vko->cameraUniformBufferMapped); // might not need this - actually yes i do, b/c this memory will be reused every frame
    vkDestroyDescriptorPool(vko->device, vko->descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(vko->device, vko->descriptorSetLayout, NULL);
    vkDestroyBuffer(vko->device, vko->vbo->vertexBuffer, NULL);
    vkFreeMemory(vko->device, vko->vbo->vertexBufferMemory, NULL);
    vkDestroyBuffer(vko->device, vko->vbo->indexBuffer, NULL);
    vkFreeMemory(vko->device, vko->vbo->indexBufferMemory, NULL);
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(vko->device, vko->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(vko->device, vko->renderFinishedSemaphores[i], NULL);
        vkDestroyFence(vko->device, vko->inFlightFences[i], NULL);
    }
    vkDestroyCommandPool(vko->device, vko->commandPool, NULL);
    vkDestroyPipeline(vko->device, vko->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(vko->device, vko->pipelineLayout, NULL);
    vkDestroyRenderPass(vko->device, vko->renderPass, NULL);
    cleanupSwapchain(vko);
    vkDestroySurfaceKHR(vko->instance, vko->surface, NULL);
    vkDestroyDevice(vko->device, NULL);
    vkDestroyInstance(vko->instance, NULL);

    glfwDestroyWindow(vko->window);
    glfwTerminate();

    // end = clock();

    // double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    // printf("Time elapsed: %f ms\n", elapsed_ms);
}

void init_renderer(vk_context *vko) {
    initWindow(vko);
    initVulkan(vko, vko->vbo);
}

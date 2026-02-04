#include "buffer.h"

void setVertexBindingDescription(vk_context *vko) {
    vko->bindingDesc = (VkVertexInputBindingDescription) {0};
    
    vko->bindingDesc.binding = 0; // vertex buffer binding
    vko->bindingDesc.stride = sizeof(Vertex);
    vko->bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // other option = instance
}

void setVertexAttributeDescriptions(vk_context *vko) {
    // attr 0: vec3 inPosition
    vko->attrDescs[0].binding = 0; // 0th vertex buffer
    vko->attrDescs[0].location = 0;
    vko->attrDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT; // s stands for signed
    vko->attrDescs[0].offset = offsetof(Vertex, pos);

    // attr 1: vec3 inColor
    vko->attrDescs[1].binding = 0;
    vko->attrDescs[1].location = 1;
    vko->attrDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vko->attrDescs[1].offset = offsetof(Vertex, color);

    vko->attrDescs[2].binding = 0;
    vko->attrDescs[2].location = 2;
    vko->attrDescs[2].format = VK_FORMAT_R32G32_SFLOAT;
    vko->attrDescs[2].offset = offsetof(Vertex, texCoord);

    vko->attrDescs[3].binding = 0;
    vko->attrDescs[3].location = 3;
    vko->attrDescs[3].format = VK_FORMAT_R8_UNORM;
    vko->attrDescs[3].offset = offsetof(Vertex, light);
}

uint32_t findMemoryType(vk_context *vko, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vko->physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    fprintf(stderr, "Failed to find suitable memory\n");
    exit(1);
}

// create buffer + allocates memory, does not map/upload data
void createBuffer(vk_context *vko, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *pBuffer, VkDeviceMemory *pBufferMemory) {
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size; // for now, 3 vertices
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vko->device, &bufferInfo, NULL, pBuffer) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vertex buffer\n");
        exit(1);
    }

    // allocate memory for buffer
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vko->device, *pBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(vko, memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(vko->device, &allocInfo, NULL, pBufferMemory) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate memory for vertex buffer\n");
        exit(1);
    }

    vkBindBufferMemory(vko->device, *pBuffer, *pBufferMemory, 0);
}

VkCommandBuffer beginSingleTimeCommands(vk_context *vko) {
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vko->commandPool; // may want to create separate command pool for short lived commands
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vko->device, &allocInfo, &commandBuffer);

    // begin recording copy command
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // only using buffer once, may help give vulkan optimizations

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(vk_context *vko, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer); // end recording

    // must submit command
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vko->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vko->graphicsQueue);

    // one time use, thus immediately free buffer
    vkFreeCommandBuffers(vko->device, vko->commandPool, 1, &commandBuffer);
}

void copyBuffer(vk_context *vko, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    // must create, record, and submit a copy buffer command
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vko);

    VkBufferCopy copyRegion = {0};
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    
    endSingleTimeCommands(vko, commandBuffer);
}

void generateChunkIndices(uint32_t* outIndices, uint32_t cubeCount) {
    uint32_t indexOffset = 0;

    for (uint32_t cube = 0; cube < cubeCount; cube++) {
        uint32_t baseVertex = cube * CUBE_SIZE;

        for (uint32_t i = 0; i < CUBE_INDEX_COUNT; i++) {
            outIndices[indexOffset++] = baseVertex + cube_indices[i];
        }
    }
}

void createIndexBuffer(vk_context *vko) {
    uint32_t totalIndexCount = MAX_BLOCKS_PER_CHUNK * INDICES_PER_CUBE;

    uint32_t *indices = malloc(totalIndexCount * sizeof(uint32_t));
    generateChunkIndices(indices, MAX_BLOCKS_PER_CHUNK);
    
    VkDeviceSize bufferSize = totalIndexCount * sizeof(uint32_t);

    vko->indexCount = totalIndexCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(vko, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(vko->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, (size_t) bufferSize);
    vkUnmapMemory(vko->device, stagingBufferMemory);

    createBuffer(vko, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vko->indexBuffer, &vko->indexBufferMemory);

    copyBuffer(vko, stagingBuffer, vko->indexBuffer, bufferSize);
    vkQueueWaitIdle(vko->graphicsQueue);

    vkDestroyBuffer(vko->device, stagingBuffer, NULL);
    vkFreeMemory(vko->device, stagingBufferMemory, NULL);
}

void createVertexBufferContext(vk_context *vko) {
    setVertexBindingDescription(vko);
    setVertexAttributeDescriptions(vko);
    createIndexBuffer(vko);
}

void copyBufferToImage(vk_context *vko, VkBuffer srcBuffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vko);

    VkBufferImageCopy region = {0};
    region.bufferOffset = 0; // no offsets
    region.bufferRowLength = 0; // no padding x
    region.bufferImageHeight = 0; // no padding y

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // doesnt have to match, but will just be blank if image itself does not have this aspect mask
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageSubresource.mipLevel = 0;

    region.imageOffset = (VkOffset3D){0, 0, 0}; // cuz vulkan supports 3d images ig?
    region.imageExtent = (VkExtent3D){width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, srcBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(vko, commandBuffer);
}

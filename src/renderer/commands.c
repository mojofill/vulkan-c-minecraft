#include "commands.h"

void createCommandPool(vk_context *vko) {
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = vko->graphicsFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(vko->device, &poolInfo, NULL, &vko->commandPool) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create command pool\n");
        exit(1);
    }
}

void createCommandBuffers(vk_context *vko) {
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vko->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vko->commandBuffers = malloc(sizeof(VkCommandBuffer) * vko->swapchainImageCount);

    for (uint32_t i = 0; i < vko->swapchainImageCount; i++) {
        vkAllocateCommandBuffers(vko->device, &allocInfo, &vko->commandBuffers[i]);
    }
}

void resetCommands(vk_context *vko) {
    for (uint32_t i = 0; i < vko->swapchainImageCount; i++) {
        vkResetCommandBuffer(vko->commandBuffers[i], 0);
    }
}

void recordCommands(vk_context *vko, uint32_t currentFrame, MeshPool pool) {
    // record commands (per swapchain image)
    for (uint32_t i = 0; i < vko->swapchainImageCount; i++) {
        VkCommandBufferBeginInfo beginInfo = {0};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(vko->commandBuffers[i], &beginInfo);

        VkRenderPassBeginInfo renderPassInfoCmd = {0}; // most basic command is starting a render pass
        renderPassInfoCmd.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfoCmd.renderPass = vko->renderPass;
        renderPassInfoCmd.framebuffer = vko->swapchainFramebuffers[i];
        renderPassInfoCmd.renderArea.offset = (VkOffset2D){0,0};
        renderPassInfoCmd.renderArea.extent = vko->surfaceCapabilities.currentExtent;

        VkClearValue clearColors[2] = {{0}, {0}};
        clearColors[0].color = (VkClearColorValue) {0.0f, 0.7f, 0.75f, 1.0f};
        clearColors[1].depthStencil = (VkClearDepthStencilValue) {1.0f, 0};
    
        renderPassInfoCmd.clearValueCount = 2;
        renderPassInfoCmd.pClearValues = clearColors;

        // render pass = the "plan" that says what it will do after it gets the image data. its currently working with a color attachment as a placeholder. needs pipeline to actually supply the data to the color attachment

        vkCmdBeginRenderPass(vko->commandBuffers[i], &renderPassInfoCmd, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(vko->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vko->graphicsPipeline);

        // specified viewport + scissor to be dynamic => must explicity set them here
        vko->viewport = (VkViewport) {0};
        vko->viewport.x = 0.0f;
        vko->viewport.y = 0.0f;
        vko->viewport.width = (float) vko->surfaceCapabilities.currentExtent.width;
        vko->viewport.height = (float) vko->surfaceCapabilities.currentExtent.height;
        vko->viewport.minDepth = 0.0f;
        vko->viewport.maxDepth = 1.0f;
        vkCmdSetViewport(vko->commandBuffers[i], 0, 1, &vko->viewport);

        // must specify scissor as well b/c we've set pipeline to be in dynamic state
        vko->scissor = (VkRect2D) {0};
        vko->scissor.offset = (VkOffset2D){0,0};
        vko->scissor.extent = vko->surfaceCapabilities.currentExtent;
        vkCmdSetScissor(vko->commandBuffers[i], 0, 1, &vko->scissor);

        // draw triangle
        // must bind vertex buffer before draw (like opengl)

        vkCmdBindVertexBuffers(vko->commandBuffers[i], 0, 1, &vko->vbo->vertexBuffer, (VkDeviceSize[]){0});
        // must bind descriptor sets as well
        vkCmdBindDescriptorSets(vko->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vko->pipelineLayout, 0, 1, &vko->descriptorSets[currentFrame], 0, NULL);
        vkCmdBindIndexBuffer(vko->commandBuffers[i], vko->vbo->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(vko->commandBuffers[i], vko->vbo->indexCount, 1, 0, 0, 0);

        // OMG - i can FINALLY start working on the mesh pool...
        for (int j = 0; j < pool.count; j++) {
            // printf("drawing mesh %d\n", j);
            ChunkMesh *mesh = &pool.meshes[j];
            
            if (mesh->vertexBuffer == VK_NULL_HANDLE) continue;
            
            vkCmdBindVertexBuffers(vko->commandBuffers[i], 0, 1, &mesh->vertexBuffer, (VkDeviceSize[]){0});
            vkCmdBindDescriptorSets(vko->commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vko->pipelineLayout, 0, 1, &vko->descriptorSets[currentFrame], 0, NULL);
            vkCmdBindIndexBuffer(vko->commandBuffers[i], vko->vbo->indexBuffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(vko->commandBuffers[i], vko->vbo->indexCount, 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(vko->commandBuffers[i]);
        vkEndCommandBuffer(vko->commandBuffers[i]);
    }
}
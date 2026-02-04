#include "image.h"

void createTextureImage(vk_context *vko, const char* imagePath) {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(imagePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4; // 4 bytes for each channel

    if (!pixels) {
        fprintf(stderr, "Failed to load image texture at path %s\n", imagePath);
        exit(1);
    }

    // want to keep this in local gpu memory for efficient acces => must create a staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(vko, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

    void *data;
    vkMapMemory(vko->device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vkUnmapMemory(vko->device, stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(vko, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vko->textureImage, &vko->textureImageMemory);

    // step 1: transition image layout to be TRANSFER_DST image layout (prepare it for transfering data from staging buffer to image)
    // step 2: copy pixel (staging) buffer to image
    // step 3: transition image layout to shader src access to prepare it for entering the pipeline

    transitionImageLayout(vko, vko->textureImage, VK_FORMAT_R8G8B8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    copyBufferToImage(vko, stagingBuffer, vko->textureImage, (uint32_t) texWidth, (uint32_t) texHeight);

    transitionImageLayout(vko, vko->textureImage, VK_FORMAT_R8G8B8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(vko->device, stagingBuffer, NULL);
    vkFreeMemory(vko->device, stagingBufferMemory, NULL);
}

void createImageView(vk_context *vko, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView) {
    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // rgba -> rgba. pure identity, no alteration
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseMipLevel = 0;

    if (vkCreateImageView(vko->device, &viewInfo, NULL, imageView) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create image view\n");
        exit(1);
    }
}

void createTextureImageView(vk_context *vko) {
    createImageView(vko, vko->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &vko->textureImageView);
}

void createTextureSampler(vk_context *vko) {
    VkSamplerCreateInfo samplerInfo = {0};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    // VK_FILTER_NEAREST for minecraft
    samplerInfo.magFilter = VK_FILTER_NEAREST; // undersampling: linear interpolation
    samplerInfo.minFilter = VK_FILTER_NEAREST; // oversampling: anisotropic. same filter - VK_FILTER_LINEAR
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // texel coordinates: uvw, corresponds to xyz, convention for texel space coordiantes
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // address mode = what do to when sampling outside of original texture image
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; // simple repeat = same orientation repetition
    // VK_SAMPLER_ADDRESS_MODE_REPEAT
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties = {0};
    vkGetPhysicalDeviceProperties(vko->physicalDevice, &properties);

    // higher anisotropy = "worse" performace, higher results
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE; // want normalized [0,1) range
    
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // not useful for regular texture images
    
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // will come back to this later. no mipmapping for now
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(vko->device, &samplerInfo, NULL, &vko->textureSampler) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create texture image sampler\n");
        exit(1);
    }
}

void createImage(vk_context *vko, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *imageMemory) {
    VkImageCreateInfo imageInfo = {0};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = (uint32_t) width;
    imageInfo.extent.height = (uint32_t) height;
    imageInfo.extent.depth = 1; // 2d image thus just one z depth layer
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling; // if u want access, then put VK_IMAGE_TILING_LINEAR - will be slower
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // since we are using staging buffer not staging image, can leave layout undefined
    imageInfo.usage = usage; // dst = destination during transfer, sampled bit = will be sampled by shader
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // only relevant for attachments (ie swapchain -> framebuffer -> render pass attachments, not relevant for texture images)
    imageInfo.flags = 0; // Optional

    if (vkCreateImage(vko->device, &imageInfo, NULL, image) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create image\n");
        exit(1);
    }

    // allocate space for image
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vko->device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(vko, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vko->device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate memory for texture image\n");
        exit(1);
    }

    vkBindImageMemory(vko->device, *image, *imageMemory, 0);
}

// must transition image layout to be suitable for gpu device memory manipulation
void transitionImageLayout(vk_context *vko, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(vko);
    
    VkImageMemoryBarrier barrier = {0}; // must synchronize image resource sharing. make pipeline wait until image memory is fully transitioned via an image memory barrier
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // not transfering image between queues
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // aspect mask = what type/aspect of the image do you want. color bit = want rgba channels
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;

    // two types of (image layout) transitions to be worried bout rn: undefined -> transfer dst, and transfer dst -> shader access
    // undefined -> transfer dst: no need to wait, can just go ahead
    // transfer dst -> shader access: need to wait for transfer *write* (copy buffer to image) to finish

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // these are the actual *memory* barriers. for this case, theres no actual memory barrier

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // this is for the *pipeline* barrier.
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // this one actually is what makes the *pipeline* wait until we are at the transfer bti stage
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // fragment shader will need this so wait til then
    }

    barrier.srcAccessMask = 0; // TODO: must define where in pipeline stage should barrier be inserted
    barrier.dstAccessMask = 0; // TODO: read above

    vkCmdPipelineBarrier(
        commandBuffer, 
        sourceStage, destinationStage, // pipeline src and dst stages
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    endSingleTimeCommands(vko, commandBuffer);
}

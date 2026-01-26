#ifndef IMAGE_H
#define IMAGE_H

#include "vk_types.h"
#include "buffer.h"
#include <string.h>

void createTextureImage(vk_context *vko, const char* imagePath);
void createTextureImageView(vk_context *vko);
void createTextureSampler(vk_context *vko);
void createImage(vk_context *vko, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *imageMemory);
void createImageView(vk_context *vko, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView *imageView);
void transitionImageLayout(vk_context *vko, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

#endif
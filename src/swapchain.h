#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "vk_types.h"

void createSwapchainSupport(vk_context *vko);
void createSwapchain(vk_context *vko);
void recreateSwapchain(vk_context *vko);
void cleanupSwapchain(vk_context *vko);
void createImageViews(vk_context *vko);
void createFramebuffers(vk_context *vko);

#endif

#include "vulkan_texture.h"

void vulkan_texture_create_sampler(vulkan_context *context,
                                   VkSampler* out_texture_sampler) {
  VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;

  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

  sampler_info.anisotropyEnable = VK_TRUE;
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(context->device.physical_device, &properties);
  sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE; // TODO: Revist for shadow maps
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

  // TODO: Revist for mipmapping
  sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler_info.mipLodBias = 0.0f;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = 0.0f;

  VK_CHECK(vkCreateSampler(context->device.logical_device, &sampler_info,
                           context->allocator, out_texture_sampler));
}

void vulkan_texture_create() {}
#include "vulkan_image.h"
#include "vulkan_command_buffer.h"
#include "vulkan_device.h"

#include "core/logger.h"
#include "core/omemory.h"

i32 temp_find_memory_index(vulkan_context* context, u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(context->device.physical_device,
                                      &memory_properties);

  for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
    // Check each memory type to see if its bit is set to 1.
    if (type_filter & (1 << i) &&
        (memory_properties.memoryTypes[i].propertyFlags & property_flags) ==
            property_flags) {
      return i;
    }
  }

  OWARN("Unable to find suitable memory type!");
  return -1;
}

void vulkan_image_create(vulkan_context *context, VkImageType image_type,
                         u32 width, u32 height, VkFormat format,
                         VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags memory_flags, b32 create_view,
                         VkImageAspectFlags view_aspect_flags,
                         vulkan_image *out_image) {

  // Copy params
  out_image->width = width;
  out_image->height = height;

  // Creation info.
  VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  image_create_info.imageType = VK_IMAGE_TYPE_2D;
  image_create_info.extent.width = width;
  image_create_info.extent.height = height;
  image_create_info.extent.depth = 1; // TODO: Support configurable depth.
  image_create_info.mipLevels = 4;    // TODO: Support mip mapping
  image_create_info.arrayLayers =
      1; // TODO: Support number of layers in the image.
  image_create_info.format = format;
  image_create_info.tiling = tiling;
  image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_create_info.usage = usage;
  image_create_info.samples =
      VK_SAMPLE_COUNT_1_BIT; // TODO: Configurable sample count.
  image_create_info.sharingMode =
      VK_SHARING_MODE_EXCLUSIVE; // TODO: Configurable sharing mode.

  VK_CHECK(vkCreateImage(context->device.logical_device, &image_create_info,
                         context->allocator, &out_image->handle));

  // Query memory requirements.
  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(context->device.logical_device,
                               out_image->handle, &memory_requirements);

  i32 memory_type = context->find_memory_index(
      memory_requirements.memoryTypeBits, memory_flags);
  if (memory_type == -1) {
    OERROR("Required memory type not found. Image not valid.");
  }

  // Allocate memory
  VkMemoryAllocateInfo memory_allocate_info = {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  memory_allocate_info.allocationSize = memory_requirements.size;
  memory_allocate_info.memoryTypeIndex = memory_type;
  VK_CHECK(vkAllocateMemory(context->device.logical_device,
                            &memory_allocate_info, context->allocator,
                            &out_image->memory));

  // Bind the memory
  VK_CHECK(vkBindImageMemory(context->device.logical_device, out_image->handle,
                             out_image->memory,
                             0)); // TODO: configurable memory offset.

  // Create view
  if (create_view) {
    out_image->view = 0;
    vulkan_image_view_create(context, format, out_image, view_aspect_flags);
  }
}

void vulkan_image_view_create(vulkan_context *context, VkFormat format,
                              vulkan_image *image,
                              VkImageAspectFlags aspect_flags) {
  VkImageViewCreateInfo view_create_info = {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view_create_info.image = image->handle;
  view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Make configurable.
  view_create_info.format = format;
  view_create_info.subresourceRange.aspectMask = aspect_flags;

  // TODO: Make configurable
  view_create_info.subresourceRange.baseMipLevel = 0;
  view_create_info.subresourceRange.levelCount = 1;
  view_create_info.subresourceRange.baseArrayLayer = 0;
  view_create_info.subresourceRange.layerCount = 1;

  VK_CHECK(vkCreateImageView(context->device.logical_device, &view_create_info,
                             context->allocator, &image->view));
}

void vulkan_image_destroy(vulkan_context *context, vulkan_image *image) {
  if (image->view) {
    vkDestroyImageView(context->device.logical_device, image->view,
                       context->allocator);
    image->view = 0;
  }
  if (image->memory) {
    vkFreeMemory(context->device.logical_device, image->memory,
                 context->allocator);
    image->memory = 0;
  }
  if (image->handle) {
    vkDestroyImage(context->device.logical_device, image->handle,
                   context->allocator);
    image->handle = 0;
  }
}

void vulkan_image_transition_layout(vulkan_context *context, VkFormat format,
                                    VkImageLayout prev_layout,
                                    VkImageLayout new_layout,
                                    vulkan_image *image) {

  vulkan_command_buffer *trans_buffer = oallocate(sizeof(vulkan_command_buffer), MEMORY_TAG_RENDERER);
  vulkan_command_buffer_allocate_and_begin_single_use(
      context, context->device.graphics_command_pool, trans_buffer);

  VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.oldLayout = prev_layout;
  barrier.newLayout = new_layout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image->handle;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags source_stage;
  VkPipelineStageFlags destination_stage;

  if (prev_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
      new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (prev_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    OERROR("unsupported layout transition!");
    return;
  }

  vkCmdPipelineBarrier(trans_buffer->handle, source_stage, destination_stage, 0,
                       0, 0, 0, 0, 1, &barrier);

  vulkan_command_buffer_end_single_use(
      context, context->device.graphics_command_pool, trans_buffer,
      context->device.graphics_queue);
}
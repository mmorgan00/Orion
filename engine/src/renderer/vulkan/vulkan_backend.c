#include "vulkan_backend.h"
#include "vulkan_buffer.h"
#include "vulkan_command_buffer.h"
#include "vulkan_device.h"
#include "vulkan_fence.h"
#include "vulkan_framebuffer.h"
#include "vulkan_image.h"
#include "vulkan_platform.h"
#include "vulkan_renderpass.h"
#include "vulkan_swapchain.h"
#include "vulkan_texture.h"

#include "vulkan_types.inl"
#include "vulkan_utils.h"

#include "core/application.h"
#include "core/logger.h"
#include "core/omemory.h"
#include "core/ostring.h"

#include "containers/darray.h"

#include "platform/platform.h"

#include "math/math_types.h"
#include "math/omath.h"
#include "shaders/vulkan_object_shader.h"

// static context for Vulkan
static vulkan_context context;
static u32 cached_framebuffer_width = 0;
static u32 cached_framebuffer_height = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

i32 find_memory_index(u32 type_filter, u32 property_flags);

void create_command_buffers(renderer_backend *backend);
void regenerate_framebuffers(renderer_backend *backend,
                             vulkan_swapchain *swapchain,
                             vulkan_renderpass *renderpass);
b8 recreate_swapchain(renderer_backend *backend);

b8 create_buffers(vulkan_context *context);

u8 *create_sample_texture(u32 height, u32 width);

void upload_data_range(vulkan_context *context, VkCommandPool pool,
                       VkFence fence, VkQueue queue, vulkan_buffer *buffer,
                       u64 offset, u64 size, void *data) {
  // Create a host-visible staging buffer to upload to. Mark it as the source of
  // the transfer.
  VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  vulkan_buffer staging;
  vulkan_buffer_create(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags,
                       true, &staging);

  // Load the data into the staging buffer.
  vulkan_buffer_load_data(context, &staging, 0, size, 0, data);

  // Perform the copy from staging to the device local buffer.
  vulkan_buffer_copy_to(context, pool, fence, queue, staging.handle, 0,
                        buffer->handle, offset, size);

  // Clean up the staging buffer.
  vulkan_buffer_destroy(context, &staging);
}

b8 vulkan_renderer_backend_initialize(renderer_backend *backend,
                                      const char *application_name,
                                      struct platform_state *plat_state) {

  // Function pointers
  context.find_memory_index = find_memory_index;

  context.allocator = 0;

  application_get_framebuffer_size(&cached_framebuffer_width,
                                   &cached_framebuffer_height);
  context.framebuffer_width =
      (cached_framebuffer_width != 0) ? cached_framebuffer_width : 800;
  context.framebuffer_height =
      (cached_framebuffer_height != 0) ? cached_framebuffer_height : 600;
  cached_framebuffer_width = 0;
  cached_framebuffer_height = 0;

  // Setup Vulkan instance.
  VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  app_info.apiVersion = VK_API_VERSION_1_2;
  app_info.pApplicationName = application_name;
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "Orion Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

  VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  create_info.pApplicationInfo = &app_info;

  // Obtain a list of required extensions
  const char **required_extensions = darray_create(const char *);
  darray_push(required_extensions,
              &VK_KHR_SURFACE_EXTENSION_NAME); // Generic surface extension
  platform_get_required_extension_names(
      &required_extensions); // Platform-specific extension(s)
#if defined(_DEBUG)
  darray_push(required_extensions,
              &VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // debug utilities

  ODEBUG("Required extensions:");
  u32 length = darray_length(required_extensions);
  for (u32 i = 0; i < length; ++i) {
    ODEBUG(required_extensions[i]);
  }
#endif

  create_info.enabledExtensionCount = darray_length(required_extensions);
  create_info.ppEnabledExtensionNames = required_extensions;

  // Validation layers.
  const char **required_validation_layer_names = 0;
  u32 required_validation_layer_count = 0;

// If validation should be done, get a list of the required validation layert
// names and make sure they exist. Validation layers should only be enabled on
// non-release builds.
#if defined(_DEBUG)
  OINFO("Validation layers enabled. Enumerating...");

  // The list of validation layers required.
  required_validation_layer_names = darray_create(const char *);
  darray_push(required_validation_layer_names, &"VK_LAYER_KHRONOS_validation");
  required_validation_layer_count =
      darray_length(required_validation_layer_names);

  // Obtain a list of available validation layers
  u32 available_layer_count = 0;
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
  VkLayerProperties *available_layers =
      darray_reserve(VkLayerProperties, available_layer_count);
  VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count,
                                              available_layers));

  // Verify all required layers are available.
  for (u32 i = 0; i < required_validation_layer_count; ++i) {
    OINFO("Searching for layer: %s...", required_validation_layer_names[i]);
    b8 found = false;
    for (u32 j = 0; j < available_layer_count; ++j) {
      if (strings_equal(required_validation_layer_names[i],
                        available_layers[j].layerName)) {
        found = true;
        OINFO("Found.");
        break;
      }
    }

    if (!found) {
      OFATAL("Required validation layer is missing: %s",
             required_validation_layer_names[i]);
      return false;
    }
  }
  OINFO("All required validation layers are present.");
#endif

  create_info.enabledLayerCount = required_validation_layer_count;
  create_info.ppEnabledLayerNames = required_validation_layer_names;

  VK_CHECK(
      vkCreateInstance(&create_info, context.allocator, &context.instance));
  OINFO("Vulkan Instance created.");

  // Debugger
#if defined(_DEBUG)
  ODEBUG("Creating Vulkan debugger...");
  u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT; //|
  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
  debug_create_info.messageSeverity = log_severity;
  debug_create_info.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debug_create_info.pfnUserCallback = vk_debug_callback;

  PFN_vkCreateDebugUtilsMessengerEXT func =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          context.instance, "vkCreateDebugUtilsMessengerEXT");
  OASSERT_MSG(func, "Failed to create debug messenger!");
  VK_CHECK(func(context.instance, &debug_create_info, context.allocator,
                &context.debug_messenger));
  ODEBUG("Vulkan debugger created.");
#endif

  // Create surface
  ODEBUG("Creating Vulkan surface...");
  if (!platform_create_vulkan_surface(plat_state, &context)) {
    OERROR("Failed to create platform surface!");
    return false;
  }

  ODEBUG("Vulkan surface created");

  // Create the device
  if (!vulkan_device_create(&context)) {
    OERROR("Failed to create device!");
    return false;
  }

  // Swapchain
  vulkan_swapchain_create(&context, context.framebuffer_width,
                          context.framebuffer_height, &context.swapchain);

  // Sets background color
  vulkan_renderpass_create(
      &context, &context.main_renderpass, 0, 0, context.framebuffer_width,
      context.framebuffer_height, 0.0f, 0.0f, 0.2f, 1.0f, 1.0f, 0);

  // Swapchain framebuffers
  context.swapchain.framebuffers =
      darray_reserve(vulkan_framebuffer, context.swapchain.image_count);
  regenerate_framebuffers(backend, &context.swapchain,
                          &context.main_renderpass);

  create_command_buffers(backend);

  // Create sync objects.
  context.image_available_semaphores =
      darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
  context.queue_complete_semaphores =
      darray_reserve(VkSemaphore, context.swapchain.max_frames_in_flight);
  context.in_flight_fences =
      darray_reserve(vulkan_fence, context.swapchain.max_frames_in_flight);

  for (u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
    VkSemaphoreCreateInfo semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(context.device.logical_device, &semaphore_create_info,
                      context.allocator,
                      &context.image_available_semaphores[i]);
    vkCreateSemaphore(context.device.logical_device, &semaphore_create_info,
                      context.allocator, &context.queue_complete_semaphores[i]);

    // Create the fence in a signaled state, indicating that the first frame has
    // already been "rendered". This will prevent the application from waiting
    // indefinitely for the first frame to render since it cannot be rendered
    // until a frame is "rendered" before it.
    vulkan_fence_create(&context, true, &context.in_flight_fences[i]);
  }

  // In flight fences should not yet exist at this point, so clear the list.
  // These are stored in pointers because the initial state should be 0, and
  // will be 0 when not in use. Acutal fences are not owned by this list.
  context.images_in_flight =
      darray_reserve(vulkan_fence, context.swapchain.image_count);
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    context.images_in_flight[i] = 0;
  }

  // Create builtin shaders
  if (!vulkan_object_shader_create(&context, &context.object_shader)) {
    OERROR("Error loading built-in basic_lighting shader.");
    return false;
  }

  create_buffers(&context);

  vulkan_buffer staging_buffer;
  u8 *texture_data = create_sample_texture(512, 512);

  vulkan_buffer_create(&context, sizeof(u8) * 512 * 512 * 4,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       true, &staging_buffer);

  vulkan_buffer_load_data(&context, &staging_buffer, 0,
                          sizeof(u8) * 512 * 512 * 4, 0, texture_data);

  vulkan_image_create(
      &context, VK_IMAGE_TYPE_2D, 512, 512, VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true, VK_IMAGE_ASPECT_COLOR_BIT,
      &context.object_shader.texture.image);

  vulkan_image_transition_layout(&context, VK_FORMAT_R8G8B8A8_SRGB,
                                 VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 &context.object_shader.texture.image);

  vulkan_buffer_copy_to_image(&context, 512, 512, staging_buffer,
                              &context.object_shader.texture.image);

  vulkan_image_transition_layout(&context, VK_FORMAT_R8G8B8A8_SRGB,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                 &context.object_shader.texture.image);

  vulkan_buffer_destroy(&context, &staging_buffer);

  vulkan_texture_create_sampler(&context,
                                &context.object_shader.texture.sampler);

  OINFO("Vulkan renderer initialized successfully.");
  return true;
}

void vulkan_renderer_backend_shutdown(renderer_backend *backend) {

  vkDeviceWaitIdle(context.device.logical_device);

  vkDestroySampler(context.device.logical_device,
                   context.object_shader.texture.sampler, context.allocator);
  vulkan_image_destroy(&context, &context.object_shader.texture.image);

  vulkan_buffer_destroy(&context, &context.object_vertex_buffer);
  vulkan_buffer_destroy(&context, &context.object_index_buffer);

  // destroy shader modules
  vulkan_object_shader_destroy(&context, &context.object_shader);

  // Sync objects
  for (u8 i = 0; i < context.swapchain.max_frames_in_flight; ++i) {
    if (context.image_available_semaphores[i]) {
      vkDestroySemaphore(context.device.logical_device,
                         context.image_available_semaphores[i],
                         context.allocator);
      context.image_available_semaphores[i] = 0;
    }
    if (context.queue_complete_semaphores[i]) {
      vkDestroySemaphore(context.device.logical_device,
                         context.queue_complete_semaphores[i],
                         context.allocator);
      context.queue_complete_semaphores[i] = 0;
    }
    vulkan_fence_destroy(&context, &context.in_flight_fences[i]);
  }
  darray_destroy(context.image_available_semaphores);
  context.image_available_semaphores = 0;

  darray_destroy(context.queue_complete_semaphores);
  context.queue_complete_semaphores = 0;

  darray_destroy(context.in_flight_fences);
  context.in_flight_fences = 0;

  darray_destroy(context.images_in_flight);
  context.images_in_flight = 0;

  // Command buffers
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    if (context.graphics_command_buffers[i].handle) {
      vulkan_command_buffer_free(&context, context.device.graphics_command_pool,
                                 &context.graphics_command_buffers[i]);
      context.graphics_command_buffers[i].handle = 0;
    }
  }
  darray_destroy(context.graphics_command_buffers);
  context.graphics_command_buffers = 0;

  // Destroy framebuffers
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    vulkan_framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
  }
  // renderpass
  vulkan_renderpass_destroy(&context, &context.main_renderpass);

  // Swapchain
  ODEBUG("Destroying Vulkan swapchain...");
  vulkan_swapchain_destroy(&context, &context.swapchain);

  ODEBUG("Destroying Vulkan device...");
  vulkan_device_destroy(&context);

  ODEBUG("Destroying Vulkan surface...");
  vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);

  ODEBUG("Destroying Vulkan debugger...");
  if (context.debug_messenger) {
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context.instance, "vkDestroyDebugUtilsMessengerEXT");
    func(context.instance, context.debug_messenger, context.allocator);
  }

  ODEBUG("Destroying Vulkan instance...");
  vkDestroyInstance(context.instance, context.allocator);
}

void vulkan_renderer_backend_on_resized(renderer_backend *backend, u16 width,
                                        u16 height) {
  cached_framebuffer_width = width;
  cached_framebuffer_height = height;
  context.framebuffer_size_generation++;

  OINFO("VUlkan renderer backend->resized: w/h/gen: %i/%i/%llu", width, height,
        context.framebuffer_size_generation);
}

b8 vulkan_renderer_backend_begin_frame(renderer_backend *backend,
                                       f32 delta_time) {

  vulkan_device *device = &context.device;

  // check if we are recreating swapchain
  if (context.recreating_swapchain) {
    VkResult result = vkDeviceWaitIdle(device->logical_device);
    if (!vulkan_result_is_success(result)) {
      OERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (1) failed: "
             "'%s'",
             vulkan_result_string(result, true));
      return false;
    }
    OINFO("Recreatint swapchain, booting.");
    return false;
  }

  // Check if the framebuffer has been resized. If so, must make a new one
  if (context.framebuffer_size_generation !=
      context.framebuffer_size_last_generation) {
    VkResult result = vkDeviceWaitIdle(device->logical_device);
    if (!vulkan_result_is_success(result)) {
      OERROR("vulkan_renderer_backend_begin_frame vkDeviceWaitIdle (2) failed: "
             "'%s'",
             vulkan_result_string(result, true));
      return false;
    }
    if (!recreate_swapchain(backend)) {
      return false;
    }
    OINFO("Resized, booting.");
    return false;
  }

  // Wait for the execution of the current frame to complete. fence being free
  // will let this move on.
  if (!vulkan_fence_wait(&context,
                         &context.in_flight_fences[context.current_frame],
                         UINT64_MAX)) {
    OWARN("In-flight fence wait failure!");
    return false;
  }

  // Acquire next image from swapchain, send the semaphore that should be
  // signaled when it's complete Sempahore will later be waited on by the queue
  // to ensure availability
  if (!vulkan_swapchain_acquire_next_image_index(
          &context, &context.swapchain, UINT64_MAX,
          context.image_available_semaphores[context.current_frame], 0,
          &context.image_index)) {
    return false;
  }

  // Begin recording commands
  vulkan_command_buffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];
  vulkan_command_buffer_reset(command_buffer);
  vulkan_command_buffer_begin(command_buffer, false, false, false);

  // State
  VkViewport viewport;
  viewport.x = 0.0f;
  viewport.y = (f32)context.framebuffer_height;
  viewport.width = (f32)context.framebuffer_width;
  viewport.height = -(f32)context.framebuffer_height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // Scissor
  VkRect2D scissor;
  scissor.offset.x = scissor.offset.y = 0;
  scissor.extent.width = context.framebuffer_width;
  scissor.extent.height = context.framebuffer_height;

  vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

  context.main_renderpass.w = context.framebuffer_width;
  context.main_renderpass.h = context.framebuffer_height;

  // Begin renderpass!
  vulkan_renderpass_begin(
      command_buffer, &context.main_renderpass,
      context.swapchain.framebuffers[context.image_index].handle);

  return true;
}

void vulkan_renderer_backend_update_global_state(mat4 projection, mat4 view,
                                                 vec3 view_position,
                                                 vec4 ambient_color, i32 mode) {
  vulkan_command_buffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];

  vulkan_object_shader_use(&context, &context.object_shader);

  // PASS COPIES - DON'T BLOCK REST OF ENGINE UPDATING TO RENDER
  context.object_shader.global_ubo.projection = projection;
  context.object_shader.global_ubo.view = view;

  // TODO: other ubo properties

  vulkan_object_shader_update_global_state(&context, &context.object_shader);
}

void vulkan_renderer_draw_object(renderer_backend *backend,
                                 vertex_data *vert_data) {

  vulkan_command_buffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];

  // These buffers have already been created, we're just going to upload into
  // them again
  upload_data_range(
      &context, context.device.graphics_command_pool, 0,
      context.device.graphics_queue, &context.object_vertex_buffer, 0,
      sizeof(vertex_3d) * vert_data->vertex_count, vert_data->vertices);
  upload_data_range(&context, context.device.graphics_command_pool, 0,
                    context.device.graphics_queue, &context.object_index_buffer,
                    0, sizeof(u32) * vert_data->index_count,
                    vert_data->indices);

  // TODO: temporary test code
  vulkan_object_shader_use(&context, &context.object_shader);

  // Bind vertex buffer at offset.
  VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(command_buffer->handle, 0, 1,
                         &context.object_vertex_buffer.handle,
                         (VkDeviceSize *)offsets);

  // Bind index buffer at offset.
  vkCmdBindIndexBuffer(command_buffer->handle,
                       context.object_index_buffer.handle, 0,
                       VK_INDEX_TYPE_UINT32);

  // Issue the draw.
  vkCmdDrawIndexed(command_buffer->handle, vert_data->index_count, 1, 0, 0, 0);

  // TODO: end temporary test code
}

b8 vulkan_renderer_backend_end_frame(renderer_backend *backend,
                                     f32 delta_time) {
  vulkan_command_buffer *command_buffer =
      &context.graphics_command_buffers[context.image_index];

  // End renderpass
  vulkan_renderpass_end(command_buffer, &context.main_renderpass);

  vulkan_command_buffer_end(command_buffer);

  // Make sure the previous frame is not using this image
  if (context.images_in_flight[context.image_index] != VK_NULL_HANDLE) {
    vulkan_fence_wait(&context, context.images_in_flight[context.image_index],
                      UINT64_MAX);
  }

  // Mark the image fence as in use by this frame
  context.images_in_flight[context.image_index] =
      &context.in_flight_fences[context.current_frame];

  // reset the fence
  vulkan_fence_reset(&context,
                     &context.in_flight_fences[context.current_frame]);

  VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer->handle;

  // Wait for semaphore
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores =
      &context.queue_complete_semaphores[context.current_frame];

  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores =
      &context.image_available_semaphores[context.current_frame];

  VkPipelineStageFlags flags[1] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit_info.pWaitDstStageMask = flags;

  VkResult result =
      vkQueueSubmit(context.device.graphics_queue, 1, &submit_info,
                    context.in_flight_fences[context.current_frame].handle);

  if (result != VK_SUCCESS) {
    OERROR("vkQueueSubmit failed with result: %s",
           vulkan_result_string(result, true));
    return false;
  }

  vulkan_command_buffer_update_submitted(command_buffer);
  // End queue submission

  // Image is served
  vulkan_swapchain_present(
      &context, &context.swapchain, context.device.graphics_queue,
      context.device.present_queue,
      context.queue_complete_semaphores[context.current_frame],
      context.image_index);

  return true;
}
void vulkan_renderer_backend_create_texture() {

  vulkan_buffer staging_buffer;
  u8 *texture_data;
  vulkan_buffer_create(&context, sizeof(u8) * 512 * 512 * 4,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       true, &staging_buffer);

  vulkan_buffer_load_data(&context, &staging_buffer, 0,
                          sizeof(u8) * 512 * 512 * 4, 0, texture_data);

  vulkan_image texture_image;
  vulkan_image_create(
      &context, VK_IMAGE_TYPE_2D, 512, 512, VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, true, 0,
      &texture_image);

  vulkan_image_transition_layout(
      &context, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &texture_image);

  vulkan_buffer_copy_to_image(&context, 512, 512, staging_buffer,
                              &texture_image);

  vulkan_image_transition_layout(
      &context, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &texture_image);

  vulkan_buffer_destroy(&context, &staging_buffer);
}

// ------------- START PRIVATE FUNCTIONS

/**
 * @brief creates texture pixel data for a checkerboard
 * @param height height of texture in pixels
 * @param width width of texture in pixels
 * @param out_texture the texture data in rgba format will be returned here.
 */
u8 *create_sample_texture(u32 height, u32 width) {
  u8 *texture_data = oallocate(sizeof(u8) * 512 * 512 * 4, MEMORY_TAG_RENDERER);
  oset_memory(texture_data, 0xFF, sizeof(u8) * 512 * 512 * 4);
  // row iteration
  for (u64 row = 0; row < height; ++row) {
    for (u64 col = 0; col < height; ++col) {
      u64 index = (row * height) + col;

      u64 index_bpp = index * 4;
      if (row % 2) {
        if (col % 2) {
          texture_data[index_bpp + 0] = 0;
          texture_data[index_bpp + 1] = 0;
        }
      } else {
        if (!(col % 2)) {
          texture_data[index_bpp + 0] = 0;
          texture_data[index_bpp + 1] = 0;
        }
      }
    }
  }
  return texture_data;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                  VkDebugUtilsMessageTypeFlagsEXT message_types,
                  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                  void *user_data) {
  switch (message_severity) {
  default:
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    OERROR(callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    OWARN(callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    OINFO(callback_data->pMessage);
    break;
  case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    OTRACE(callback_data->pMessage);
    break;
  }
  return VK_FALSE;
}

i32 find_memory_index(u32 type_filter, u32 property_flags) {
  VkPhysicalDeviceMemoryProperties memory_properties;
  vkGetPhysicalDeviceMemoryProperties(context.device.physical_device,
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

void create_command_buffers(renderer_backend *backend) {
  if (!context.graphics_command_buffers) {
    context.graphics_command_buffers =
        darray_reserve(vulkan_command_buffer, context.swapchain.image_count);
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
      ozero_memory(&context.graphics_command_buffers[i],
                   sizeof(vulkan_command_buffer));
    }
  }
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    if (context.graphics_command_buffers[i].handle) {
      vulkan_command_buffer_free(&context, context.device.graphics_command_pool,
                                 &context.graphics_command_buffers[i]);
    }
    ozero_memory(&context.graphics_command_buffers[i],
                 sizeof(vulkan_command_buffer));
    vulkan_command_buffer_allocate(&context,
                                   context.device.graphics_command_pool, true,
                                   &context.graphics_command_buffers[i]);
  }

  ODEBUG("Vulkan command buffers created.");
}

void regenerate_framebuffers(renderer_backend *backend,
                             vulkan_swapchain *swapchain,
                             vulkan_renderpass *renderpass) {
  for (u32 i = 0; i < swapchain->image_count; ++i) {
    // TODO: make this dynamic based on the currently configured attachments
    u32 attachment_count = 2;
    VkImageView attachments[] = {swapchain->views[i],
                                 swapchain->depth_attachment.view};

    vulkan_framebuffer_create(&context, renderpass, context.framebuffer_width,
                              context.framebuffer_height, attachment_count,
                              attachments, &context.swapchain.framebuffers[i]);
  }
}

b8 recreate_swapchain(renderer_backend *backend) {
  // If already being recreated, do not try again.
  if (context.recreating_swapchain) {
    ODEBUG("recreate_swapchain called when already recreating. Booting.");
    return false;
  }

  // Detect if the window is too small to be drawn to
  if (context.framebuffer_width == 0 || context.framebuffer_height == 0) {
    ODEBUG("recreate_swapchain called when window is < 1 in a dimension. "
           "Booting.");
    return false;
  }

  // Mark as recreating if the dimensions are valid.
  context.recreating_swapchain = true;

  // Wait for any operations to complete.
  vkDeviceWaitIdle(context.device.logical_device);

  // Clear these out just in case.
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    context.images_in_flight[i] = 0;
  }

  // Requery support
  vulkan_device_query_swapchain_support(context.device.physical_device,
                                        context.surface,
                                        &context.device.swapchain_support);
  vulkan_device_detect_depth_format(&context.device);

  vulkan_swapchain_recreate(&context, cached_framebuffer_width,
                            cached_framebuffer_height, &context.swapchain);

  // Sync the framebuffer size with the cached sizes.
  context.framebuffer_width = cached_framebuffer_width;
  context.framebuffer_height = cached_framebuffer_height;
  context.main_renderpass.w = context.framebuffer_width;
  context.main_renderpass.h = context.framebuffer_height;
  cached_framebuffer_width = 0;
  cached_framebuffer_height = 0;

  // Update framebuffer size generation.
  context.framebuffer_size_last_generation =
      context.framebuffer_size_generation;

  // cleanup swapchain
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    vulkan_command_buffer_free(&context, context.device.graphics_command_pool,
                               &context.graphics_command_buffers[i]);
  }

  // Framebuffers.
  for (u32 i = 0; i < context.swapchain.image_count; ++i) {
    vulkan_framebuffer_destroy(&context, &context.swapchain.framebuffers[i]);
  }

  context.main_renderpass.x = 0;
  context.main_renderpass.y = 0;
  context.main_renderpass.w = context.framebuffer_width;
  context.main_renderpass.h = context.framebuffer_height;

  regenerate_framebuffers(backend, &context.swapchain,
                          &context.main_renderpass);

  create_command_buffers(backend);

  // Clear the recreating flag.
  context.recreating_swapchain = false;

  return true;
}

b8 create_buffers(vulkan_context *context) {
  VkMemoryPropertyFlagBits memory_property_flags =
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  const u64 vertex_buffer_size = sizeof(vertex_3d) * 1024 * 1024;
  if (!vulkan_buffer_create(
          context, vertex_buffer_size,
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          memory_property_flags, true, &context->object_vertex_buffer)) {
    OERROR("Error creating vertex buffer.");
    return false;
  }
  context->geometry_vertex_offset = 0;

  const u64 index_buffer_size = sizeof(u32) * 1024 * 1024;
  if (!vulkan_buffer_create(
          context, index_buffer_size,
          VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
          memory_property_flags, true, &context->object_index_buffer)) {
    OERROR("Error creating vertex buffer.");
    return false;
  }
  context->geometry_index_offset = 0;

  return true;
}

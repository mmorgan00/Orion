#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

b8 load_texture(vulkan_context* context, const char *image_source, u32 *out_width, u32 *out_height,
                u32 **pixels) {

  char file_name[1024];
  // Start by trying to find the file
  string_format(file_name, "assets/materials/%s", image_source);
  if (!filesystem_exists(file_name)) {
    OERROR("Could not find texture source file: %s", image_source);
    return false;
  }
  // TODO: Absolutely not. Shoulders of giants. stb_image.h
  int tex_width, tex_height, tex_channels;

  stbi_uc *pix = stbi_load(file_name, &tex_width, &tex_height, &tex_channels,
                           STBI_rgb_alpha);


  if (!pix) {
    // Error handling
    OERROR("Unable to read texture source file: %s.", file_name);
  }

  VkDeviceSize image_size = tex_width * tex_height * 4; // res * rgba
  vulkan_image out_image;
  vulkan_image_create(context, VK_IMAGE_TYPE_2D, tex_width, tex_height,  VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true,
                      VK_IMAGE_ASPECT_COLOR_BIT, &out_image);

  OINFO("Loaded texture source %s successfully", image_source);
  return true;
}
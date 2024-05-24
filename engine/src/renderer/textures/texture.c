#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

b8 load_texture(const char *image_source, u32 *out_width, u32 *out_height,
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

  OINFO("Loaded texture source %s successfully", image_source);
  return true;
}
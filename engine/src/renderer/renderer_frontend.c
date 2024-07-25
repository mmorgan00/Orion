#include "renderer_frontend.h"

#include "renderer_backend.h"

#include "core/logger.h"
#include "core/omemory.h"
#include "math/omath.h"

// Backend render context
static renderer_backend *backend = 0;

b8 renderer_initialize(const char *application_name,
                       struct platform_state *plat_state) {
  backend = oallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);

  // TODO: Make configurable
  renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, plat_state, backend);
  backend->frame_number = 0;

  if (!backend->initialize(backend, application_name, plat_state)) {
    OFATAL("Renderer backend failed to initialize. Shutting down");
    return false;
  }

  return true;
}

void renderer_shutdown() {
  backend->shutdown(backend);
  ofree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
}

b8 renderer_begin_frame(f32 delta_time) {
  return backend->begin_frame(backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
  b8 result = backend->end_frame(backend, delta_time);
  backend->frame_number++;
  return result;
}

/**
 * @brief Function handler for a window resize. Calls the appropriate backend
 * resize handler with the given parameters
 */
void renderer_on_resized(u16 width, u16 height) {
  if (backend) {
    backend->resized(backend, width, height);
  } else {
    OWARN("renderer backend does not exist to accept resize: %i %i", width,
          height);
  }
}

/**
 * @brief Renders the frame using the given render packet.
 * Presently, this performs some small amount of view transformation
 * @param packet The render packet of data. Right now this only contains delta
 * time
 */
b8 renderer_draw_frame(render_packet *packet) {
  // If the begin frame was successful, continue mid frame ops
  if (renderer_begin_frame(packet->delta_time)) {

    mat4 projection =
        mat4_perspective(deg_to_rad(45.0f), 1280 / 720.0f, 0.1f, 1000.0f);
    static f32 z = 0.0f;
    z += 0.01f;
    mat4 view = mat4_translation((vec3){0, 0, z}); // -30.0f
    view = mat4_inverse(view);

    backend->update_global_state(projection, view, vec3_zero(), vec4_one(), 0);

    b8 result = renderer_end_frame(packet->delta_time);

    // If end frame had issue, likely unrecoverable. shutdown
    if (!result) {
      OFATAL("render_end_frame failed. Application shutting down");
      return false;
    }
  }

  return true;
}

#include "renderer_frontend.h"

#include "renderer_backend.h"
// #include "textures/texture.h"

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

void renderer_on_resized(u16 width, u16 height) {
  if (backend) {
    backend->resized(backend, width, height);
    // backend->create_texture("test", false, 0, 0, 0, 0, 0, 0); // garbage,
    // just want to check wiring backend->destroy_texture(0);
  } else {
    OWARN("renderer backend does not exist to accept resize: %i %i", width,
          height);
  }
}

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

    // mat4 model = mat4_translation((vec3){0, 0, 0});
    static f32 angle = 0.01f;
    angle += 0.001f;
    quat rotation = quat_from_axis_angle(vec3_forward(), angle, false);
    mat4 model = quat_to_rotation_matrix(rotation, vec3_zero());
    geometry_render_data render_data;
    render_data.model = model;
    backend->update_object(render_data);
    // backend->update_global_state(projection, view, vec3_zero(), vec4_one(),
    // 0); //TODO: Doesn't work yet, extraneous calls that are throwing
    // validation errors
    b8 result = renderer_end_frame(packet->delta_time);

    // If end frame had issue, likely unrecoverable. shutdown
    if (!result) {
      OFATAL("render_end_frame failed. Application shutting down");
      return false;
    }
  }

  return true;
}

void renderer_create_texture(const char *name, b8 auto_release, i32 width,
                             i32 height, i32 channel_count, const u8 *pixels,
                             b8 has_transparency, struct texture *out_texture) {
  backend->create_texture(name, auto_release, width, height, channel_count,
                          pixels, has_transparency, out_texture);
}

void renderer_destroy_texture(struct texture *texture) {
  backend->destroy_texture(texture);
}

void renderer_update_object(geometry_render_data render_data) {
  backend->update_object(render_data);
}
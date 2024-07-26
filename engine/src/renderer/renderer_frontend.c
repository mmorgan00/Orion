#include "renderer_frontend.h"

#include "renderer_backend.h"

#include "containers/darray.h"
#include "core/logger.h"
#include "core/omemory.h"
#include "math/omath.h"

// Backend render context
static renderer_backend *backend = 0;

/**
 * @brief Current scene data. Later this will be a proper scene graph that can
 * handle multiple transformation dependency chains. For now, we just want to
 * hold everything in the scene to draw
 */
static render_object *scene_data = 0;

/**
 * @brief Mesh data stored as array of pointers since meshes can vary in range.
 */
static vertex_3d **mesh_data = 0;

b8 renderer_initialize(const char *application_name,
                       struct platform_state *plat_state) {
  backend = oallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);
  // initialize scene data
  scene_data = darray_create(render_object *);
  mesh_data = darray_create(vertex_3d *);

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
    z += 0.001f;
    mat4 view = mat4_translation((vec3){0, 0, z}); // -30.0f
    view = mat4_inverse(view);
    backend->update_global_state(projection, view, vec3_zero(), vec4_one(), 0);

    const f32 f = 0.5f;

    // TODO: REMOVE TEMP CODE
    const u32 vert_count = 4;
    vertex_3d verts[vert_count];
    ozero_memory(verts, sizeof(vertex_3d) * vert_count);

    verts[0].position.x = -0.5 * f;
    verts[0].position.y = -0.5 * f;
    verts[0].tex_coord.u = 0.0;
    verts[0].tex_coord.v = 0.0;

    verts[1].position.x = 0.5 * f;
    verts[1].position.y = 0.5 * f;
    verts[1].tex_coord.u = 1.0;
    verts[1].tex_coord.v = 1.0;

    verts[2].position.x = -0.5 * f;
    verts[2].position.y = 0.5 * f;
    verts[2].tex_coord.u = 0.0;
    verts[2].tex_coord.v = 1.0;

    verts[3].position.x = 0.5 * f;
    verts[3].position.y = -0.5 * f;
    verts[3].tex_coord.u = 1.0;
    verts[3].tex_coord.v = 0.0;

    const u32 index_count = 6;
    u32 indices[6] = {0, 1, 2, 0, 3, 1};
    vertex_data vd;
    vd.index_count = index_count;
    vd.indices = indices;
    vd.vertices = verts;
    vd.vertex_count = vert_count;
    backend->draw_object(backend, &vd);
    // for item in scene{
    //  renderer_draw_object(item)
    //}

    b8 result = renderer_end_frame(packet->delta_time);

    // If end frame had issue, likely unrecoverable. shutdown
    if (!result) {
      OFATAL("render_end_frame failed. Application shutting down");

      return false;
    }
  }

  return true;
}

/**
 * @brief Creates a new object to be rendered
 * @param gemoetry_data_id - geometry data id to reference when drawing
 * @param texture_data_id - texture data id to reference when drawing
 */
u32 renderer_register_object(u32 geometry_data_id, u32 texture_data_id) {
  // Declaring static to increment between calls for multiple objects.
  // This may be bad code. If it is we'll address it later, but it should work
  // for now
  static u32 object_id = 0;

  render_object nro; // new render object
  // TODO: Check for valid mesh geo data at given ID index
  nro.geometry_data_id = geometry_data_id;
  nro.texture_data_id = texture_data_id;
  nro.id = object_id;

  // REGISTER THE OBJECT NOW
  darray_push(scene_data, nro);
  // TODO TEMP AS HELL
  ODEBUG("Vertex 0 X value: %f", mesh_data[0][0].position.x);
  ODEBUG("Vertex 0 Y value: %f", mesh_data[0][0].position.y);
  ODEBUG("Vertex 1 X value: %f", mesh_data[0][1].position.x);
  ODEBUG("Vertex 1 Y value: %f", mesh_data[0][1].position.y);
  ODEBUG("Vertex 2 X value: %f", mesh_data[0][2].position.x);
  ODEBUG("Vertex 2 Y value: %f", mesh_data[0][2].position.y);
  ODEBUG("Vertex 3 X value: %f", mesh_data[0][3].position.x);
  ODEBUG("Vertex 3 Y value: %f", mesh_data[0][3].position.y);
  return object_id++; // Post incrememnt to return 'current' value, then
                      // incrememnt static var for next call
}

/**
 * @brief loads a given mesh into the renderer, returns back an ID to be
 * referenced in registering an object Since ID's are generated via sequential
 * integer counts, we can get away with an array to store them. Our current
 * darray supports pop_at indexes, so this should suffice for a while.
 */
u32 renderer_load_mesh(vertex_3d *new_mesh_data, u32 vertex_count) {
  static u32 mesh_id = 0;
  darray_push(mesh_data, new_mesh_data);
  return mesh_id++;
}
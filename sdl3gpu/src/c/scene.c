#include "scene.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

typedef struct _Scene
{
    int x;
} Scene;

static Scene scene;

void Scene_create(const char* path)
{
    cgltf_options options = {0};
    cgltf_data* gltf = NULL;

    if (cgltf_parse_file(&options, path, &gltf) != cgltf_result_success ||
        cgltf_load_buffers(&options, gltf, path) != cgltf_result_success ||
        cgltf_validate(gltf) != cgltf_result_success
        )
    {
        printf("Could not load gltf from %s\n", path);
    }

    for (size_t n = 0; n < gltf->nodes_count; ++n)
    {
        printf("%s\n", (gltf->nodes + n)->name);
    }
}

void Scene_destroy()
{

}
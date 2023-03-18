#include <raycaster.h>

#include <stdio.h>
#include <thread>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void file_mode(Scene *scene, u32 w, u32 h, u32 cores) {
    u32 *data = raytrace(scene, w, h, cores);

    stbi_flip_vertically_on_write(1);
    stbi_write_png("out.png", w, h, 4, data, 4 * w);
}

f32 random_float() {
    return (f32)rand() / (f32)RAND_MAX;
}

int main(int argc, char *argv[]) {
	u32 num_threads = 20;
	if (argc > 1) {
		num_threads = atoi(argv[1]);
	}

    Scene scene;

    u32 n = 400;
    u32 i = 0;
    
    scene.spheres = (Sphere *) malloc(n * sizeof(Sphere));
    scene.materials = (Material *) malloc(n * sizeof(Material));
   
   /* for (s32 j = -11; j < 10; ++j) {
        for (s32 k = -13; k < 4; ++k) {*/
   	for (s32 j = -2; j < 2; ++j) {
        for (s32 k = -3; k < 1; ++k) {
            v3 center = vec3(j * 2.5 + random_float() * 0.5, k*3.5 + random_float(), 0.6f);

            Material mat;
            f32 r = random_float();
            if (r > 0.6) {
                mat = make_metallic(vec3(random_float(), random_float(), random_float()));
            } else {
                mat = make_matt(vec3(random_float(), random_float(), random_float()));
            }

			scene.materials[i] = mat;
            scene.spheres[i] = make_sphere(center, 0.6f, i);
            i++;
        }
    }

    scene.num_spheres = i;
    scene.num_materials = i;

    scene.materials[i] = make_matt(vec3(0.5));
    Plane floor = make_plane(0, i);
	scene.planes = &floor;
    scene.num_planes = 1;

	u32 w = 1280, h = 720;
    v3 cam_pos = { 0, 12, 5 };
    v3 look_at = { 0, 0, 1 };
    scene.camera = make_camera(25, cam_pos, look_at, length(cam_pos - look_at), 0.15, w, h);
    
    file_mode(&scene, w, h, num_threads);

    return 0;
}

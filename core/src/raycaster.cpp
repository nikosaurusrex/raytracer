#include "raycaster.h"

#include <stdlib.h>
#include <time.h>
#include <thread>
#include <vector>

#define MIN_DIST 0.001f
#define MAX_DIST 200
#define PI 3.1415926535f

#ifdef _WIN64
#pragma intrinsic(__rdtsc)

u64 get_cpu_time() {
	return __rdtsc();
}

#else

u64 get_cpu_time() {
}

#endif

f32 clamp(f32 v, f32 l, f32 h) {
    if (v < l) return l;
    if (v > h) return h;
    return v;
}

// RGB -> BGR
u32 rgb_to_hex(v3 v) {
    u32 hex = 0xFF << 24;
    hex |= (u32)(v.b * 255.9) << 16;
    hex |= (u32)(v.g * 255.9) << 8;
    hex |= (u32)(v.r * 255.9);

    return hex;
}

v3 clamp(v3 v, f32 l, f32 h) {
    f32 b = clamp(v.b, l, h);
    f32 g = clamp(v.g, l, h);
    f32 r = clamp(v.r, l, h);
    
    return vec3(r, g, b);
}

f32 linear_to_srgb(f32 l) {
	if (l < 0.0f) { 
		l = 0.0f;
	}
	if (l > 1.0f) {
		l = 1.0f;
	}

   f32 s = l * 12.92f;
   if (l > 0.0031308f) {
       s = 1.055f * pow(l, 1.0f / 2.4f) - 0.055f;
   }

   return s;
}

v3 linear_to_srgb(v3 v) {
	return vec3(
		linear_to_srgb(v.x),
		linear_to_srgb(v.y),
		linear_to_srgb(v.z)
	);
}

void map_sphere_uv(v3 p, f32 *u, f32 *v) {
	f32 phi = atan2f(p.z, p.x);	
	f32 theta = asinf(p.y);

	*u = 1.0f - (phi + PI) / (2.0f * PI);
	*v = (theta + PI / 2.0f) / PI;
}

Material make_matt(v3 albedo) {
	return make_matt(albedo, 0);
}

Material make_metallic(v3 albedo) {
	return make_metallic(albedo, 0);
}

Material make_matt(v3 albedo, Texture *tex) {
    Material mat;

    mat.kind = MATT;
    mat.albedo = albedo;
    mat.texture = tex;

    return mat;
}

Material make_metallic(v3 albedo, Texture *tex) {
	Material mat;

    mat.kind = METALLIC;
    mat.albedo = albedo;
    mat.texture = tex;

    return mat;
}

Sphere make_sphere(v3 center, f32 radius, u32 material_index) {
	Sphere sphere;

    sphere.center = center;
    sphere.radius = radius;
    sphere.material_index = material_index;

    return sphere;
}

Plane make_plane(f32 z, u32 material_index) {
	Plane plane;

    plane.z = z;
    plane.material_index = material_index;

    return plane;
}

Camera make_camera(f32 fov, v3 pos, v3 lookat, f32 focus_dist, f32 aperture, u32 width, u32 height) {
	Camera camera;

	f32 theta = (fov/180.0f) * PI;
	f32 aspect_ratio = (f32)width / (f32)height;

	f32 h = tan(theta/2);
	f32 viewport_height = 2.0f * h;
	f32 viewport_width = aspect_ratio * viewport_height;

	v3 w = normalize(pos - lookat);
	v3 u = normalize(cross(vec3(0, 0, 1), w));
	v3 v = cross(w, u);

	camera.pos = pos;
	camera.u = u;
	camera.v = v;
	camera.hori = focus_dist * viewport_width * u;
	camera.vert = focus_dist * viewport_height * v;
	camera.llc = camera.pos - camera.hori/2 - camera.vert/2 - focus_dist*w;

	camera.lens_radius = aperture / 2;
	
	return camera;
}

Ray camera_get_ray(Camera *camera, f32 s, f32 t, Random *random) {
	lane_v3 cam_pos = lane_v3_from_v3(camera->pos);
	lane_v3 cam_u = lane_v3_from_v3(camera->u);
	lane_v3 cam_v = lane_v3_from_v3(camera->v);
	lane_v3 cam_llc = lane_v3_from_v3(camera->llc);
	lane_v3 cam_vert = lane_v3_from_v3(camera->vert);
	lane_v3 cam_hori = lane_v3_from_v3(camera->hori);

	lane_v3 rd = lane_vec3(camera->lens_radius) * random_vec3_lane(random);
	lane_v3 offset = cam_u * rd.x + cam_v * rd.y;

	lane_f32 lane_s = lane_f32_create(s);
	lane_f32 lane_t = lane_f32_create(t);

	Ray ray;
	ray.origin = cam_pos + offset;
	ray.dir = cam_llc + lane_s*cam_hori + lane_t*cam_vert - cam_pos - offset;
	return ray;
}

bool scatter(Material material, Ray *ray, lane_v3 p, lane_v3 n, lane_v3 *attenuation, Random *random) {

    switch (material.kind) {
		case MATT: {
			v3 target = p + n + random_vec3(random);

			ray->origin = p;
			ray->dir = normalize(target - p);
			
			*attenuation = material.albedo;

			return true;
		}
		case METALLIC: {
			v3 reflected = reflect(ray->dir, n);
			
			ray->origin = p;
			ray->dir = reflected;

			*attenuation = material.albedo;

			return dot(ray->dir, n) > 0;
		}
        case DIALECTRIC: {
        	return false;
        }
    }
    return false;
}

Hit scan_hit(Scene *scene, Ray *ray) {
    Hit hit{};
    hit.t = MAX_DIST;

    v3 ro = ray->origin;
    v3 rd = ray->dir;

    for (u32 i = 0; i < scene->num_planes; ++i) {
    	Plane plane = scene->planes[i];
        f32 distance = (plane.z - ro.z) / rd.z;

        if (distance > MIN_DIST && distance < hit.t) {
            hit.t = distance;
            hit.material_index = plane.material_index;
            hit.n = { 0, 0, 1 };
        }
    }

    for (u32 i = 0; i < scene->num_spheres; ++i) {
    	Sphere sphere = scene->spheres[i];
        v3 displacement = ro - sphere.center;
        f32 a = dot(rd, rd);
        f32 b = 2.0f * dot(rd, displacement);
        f32 c = dot(displacement, displacement) - sphere.radius * sphere.radius;

        f32 discriminant = b * b - 4.0f * a * c;

        if (discriminant < 0) {
            continue;
        }

        f32 t0 = (-b + sqrt(discriminant)) / (2.0f * a);
        f32 t1 = (-b - sqrt(discriminant)) / (2.0f * a);
        f32 t;

        if (t0 > MIN_DIST) {
            if (t1 > MIN_DIST) {
                t = min(t0, t1);
            } else {
                t = t0;
            }
        } else {
            t = t1;
        }

        if (t > MIN_DIST && t < hit.t) {
            hit.t = t;
            hit.n = normalize((ro + rd * hit.t) - sphere.center);
            hit.material_index = sphere.material_index;
        }
    }

    return hit;
}

void raytrace_tile(WorkQueue *queue, Scene *scene, u32 *data, u32 w, u32 h) {
    if (queue->tile_index >= queue->tile_count) {
        return;
    }

    Camera *camera = &scene->camera;
    Tile *tile = &queue->tiles[queue->tile_index];

    queue->tile_index++;

    u32 rays_per_pixel = 128;
    u32 bounces = 8;

    for (u32 y = 0; y < tile->h; ++y) {
        for (u32 x = 0; x < tile->w; ++x) {
			v3 output = vec3(0.0);
			u32 xx = x + tile->x;
			u32 yy = y + tile->y;

        	for (u32 i = 0; i < rays_per_pixel; ++i) {
				f32 u = (f32)xx / (f32)w;
				f32 v = (f32)yy / (f32)h;

                Ray ray = camera_get_ray(camera, u, v, &tile->random);

                v3 attenuation = vec3(1.0f);
                Ray scattered;

                for (u32 i = 0; i < bounces; ++i) {
					queue->total_bounces++;

                    Hit hit = scan_hit(scene, &ray);
                    v3 p = ray.origin + hit.t * ray.dir;

                    if (hit.t < MAX_DIST) {
                        Material material = scene->materials[hit.material_index];

                        v3 catt;
                        if (!scatter(material, &ray, p, hit.n, &catt, &tile->random)) {
                            attenuation = vec3(0);
                            break;
                        }

                        attenuation = attenuation * catt;
                    } else {
                        break;
                    }
                }

                output = output + attenuation * vec3(0.5f, 0.7f, 1.0f);
			}

            output = output / rays_per_pixel;

			output = clamp(output, 0.0f, 1.0f);
			output = linear_to_srgb(output);
			
            data[yy * w + xx] = rgb_to_hex(output);
        }
    }
}

void raytrace_data(Scene *scene, u32 *data, u32 w, u32 h, u32 cores) {
    WorkQueue queue;
    
    u32 ts = w / cores;

    u32 tiles_x = (w + ts - 1) / ts;
    u32 tiles_y = (h + ts - 1) / ts;
    u32 tiles_count = tiles_x * tiles_y;

    queue.tiles = (Tile *)malloc(tiles_count * sizeof(Tile));
    queue.tile_count = tiles_count;
    queue.tile_index = 0;

    for (u32 y = 0; y < tiles_y; ++y) {
        for (u32 x = 0; x < tiles_x; ++x) {
            u32 tx = x * ts;
            u32 ty = y * ts;

            u32 tw = ts;
            u32 th = ts;

            if (tx + tw > w) {
                tw = w - tx;
            }

            if (ty + th > h) {
                th = h - ty;
            }

            queue.tiles[y * tiles_x + x] = {{(u32) rand()}, tx, ty, tw, th};
        }
    }

	/* TODO: apparently clock does measure CPU time on non-Windows? */
    clock_t before = clock();
	u64 before_cpu_time = get_cpu_time();
    
    std::vector<std::thread> threads;

    auto loop = [&]() {
        while (queue.tile_index < queue.tile_count) {
            raytrace_tile(&queue, scene, data, w, h);

            u32 percentage = (u32)((f32)(queue.tile_index + 0) / (f32)queue.tile_count * 100);
            printf("\rRaytrace %3d%%", percentage);
            fflush(stdout);
        }
    };

    for (u32 i = 0; i < cores; ++i) {
        std::thread t(loop);

        threads.push_back(move(t));
    }

    for (auto &t : threads) {
        t.join();
    }

    clock_t after = clock();
	u64 after_cpu_time = get_cpu_time();
	clock_t diff = after - before;
	u64 diff_cpu_time = after_cpu_time - before_cpu_time;

	u64 bounces = queue.total_bounces;
	putc('\n', stdout);
	printf("Raycasting took %ld ms\n", diff);
    printf("Total bounces %llu\n", bounces);
    printf("Performance %fms/bounce\n", (f64)diff / (f64)bounces);
    printf("Performance %fcycles/bounce\n", (f64)diff_cpu_time / (f64)bounces);
}

u32 *raytrace(Scene *scene, u32 w, u32 h, u32 cores) {
    u32 *data = (u32 *)malloc(w * h * sizeof(u32));

    raytrace_data(scene, data, w, h, cores);

    return data;
}

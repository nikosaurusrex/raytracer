#include "raycaster.h"

#include <stdlib.h>
#include <thread>
#include <vector>

#define MIN_DIST 0.001f
#define MAX_DIST 200
#define PI 3.1415926535f

f32 clamp(f32 v, f32 l, f32 h) {
    if (v < l) return l;
    if (v > h) return h;
    return v;
}

// RGB -> BGR
u32 rgb_to_hex(v3 v) {
    f32 b = clamp(v.b, 0, 1);
    f32 g = clamp(v.g, 0, 1);
    f32 r = clamp(v.r, 0, 1);

    u32 hex = 0xFF << 24;
    hex |= (u32)(b * 255.9) << 16;
    hex |= (u32)(g * 255.9) << 8;
    hex |= (u32)(r * 255.9);

    return hex;
}

f32 randomf() {
	return (f32)rand() / (f32)RAND_MAX;
}

f32 randomf2() {
	return 2.0f * randomf() - 1.0f;
}

v3 random_vec3() {
    v3 v;

    do {
        v = 2.0f * vec3(randomf(), randomf(), randomf()) - vec3(1.0);
    } while (length2(v) >= 1.0f);

    return v;
}

f32 reflectance(f32 cosine, f32 ref_idx) {
    auto r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

v3 refract(v3 uv, v3 n, double etai_over_etat) {
    auto cos_theta = fmin(dot(-uv, n), 1.0);
    v3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    v3 r_out_parallel = -sqrt(fabs(1.0 - length2(r_out_perp))) * n;
    return r_out_perp + r_out_parallel;
}

Material *make_matt(v3 albedo) {
    Material *mat = new Material();

    mat->kind = MATT;
    mat->albedo = albedo;

    return mat;
}

Material *make_metallic(v3 albedo) {
    Material *mat = new Material();

    mat->kind = METALLIC;
    mat->albedo = albedo;

    return mat;
}

Sphere *make_sphere(v3 center, f32 radius, Material *mat) {
    Sphere *sphere = new Sphere();

    sphere->center = center;
    sphere->radius = radius;
    sphere->material = mat;

    return sphere;
}

Plane *make_plane(f32 z, Material *mat) {
    Plane *plane = new Plane();

    plane->z = z;
    plane->material = mat;

    return plane;
}

Camera make_camera(f32 fov, v3 pos, v3 lookat, f32 focus_dist, f32 aperture, s32 width, s32 height) {
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

Ray camera_get_ray(Camera *camera, f32 s, f32 t) {
	v3 rd = camera->lens_radius * random_vec3();
	v3 offset = camera->u * rd.x + camera->v * rd.y;

	Ray ray;
	ray.origin = camera->pos + offset;
	ray.dir = camera->llc + s*camera->hori + t*camera->vert - camera->pos - offset;
	return ray;
}

bool scatter(Material *material, Ray *ray, v3 p, v3 n, v3 *attenuation) {
    switch (material->kind) {
		case MATT: {
			v3 target = p + n + random_vec3();

			ray->origin = p;
			ray->dir = normalize(target - p);

			*attenuation = material->albedo;

			return true;
		}
		case METALLIC: {
			v3 reflected = reflect(ray->dir, n);
			
			ray->origin = p;
			ray->dir = reflected;

			*attenuation = material->albedo;

			return dot(ray->dir, n) > 0;
		}
        case DIALECTRIC: {
            *attenuation = vec3(1.0f);
            f32 ir = 1.5f;
            bool front_face = dot(ray->dir, n) < 0;
            f32 refraction_ratio = front_face ? (1.0 / ir) : ir;

            f32 cos_theta = fmin(dot(-ray->dir, n), 1.0);
            f32 sin_theta = sqrtf(1.0 - cos_theta * cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            v3 direction;

            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > randomf())
                direction = reflect(ray->dir, n);
            else
                direction = refract(ray->dir, n, refraction_ratio);

            ray->origin = p;
            ray->dir = direction;
            return true;
        }
    }
    return false;
}

Hit scan_hit(Scene *scene, Ray *ray) {
    Hit hit{};
    hit.t = MAX_DIST;

    v3 ro = ray->origin;
    v3 rd = ray->dir;

    for (s32 i = 0; i < scene->num_planes; ++i) {
    	Plane *plane = scene->planes[i];
        f32 distance = (plane->z - ro.z) / rd.z;

        if (distance > MIN_DIST && distance < hit.t) {
            hit.t = distance;
            hit.material = plane->material;
            hit.n = { 0, 0, 1 };
        }
    }

    for (s32 i = 0; i < scene->num_spheres; ++i) {
    	Sphere *sphere = scene->spheres[i];
        v3 displacement = ro - sphere->center;
        f32 a = dot(rd, rd);
        f32 b = 2.0f * dot(rd, displacement);
        f32 c = dot(displacement, displacement) - sphere->radius * sphere->radius;

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
            hit.n = normalize((ro + rd * hit.t) - sphere->center);
            hit.material = sphere->material;
        }
    }

    return hit;
}

void raytrace_tile(WorkQueue *queue, Scene *scene, u32 *data, s32 w, s32 h) {
    if (queue->tile_index >= queue->tile_count) {
        return;
    }

    Camera *camera = &scene->camera;
    Tile *tile = &queue->tiles[queue->tile_index];

    queue->tile_index++;

    s32 rays_per_pixel = 128;
    s32 bounces = 8;

    for (s32 y = tile->h - 1; y >= 0; --y) {
        for (s32 x = 0; x < tile->w; ++x) {
			v3 output = vec3(0.0);
			s32 xx = x + tile->x;
			s32 yy = y + tile->y;

        	for (s32 i = 0; i < rays_per_pixel; ++i) {
				f32 u = (f32)xx / (f32)w;
				f32 v = (f32)yy / (f32)h;

                Ray ray = camera_get_ray(camera, u, v);

                v3 attenuation = vec3(1.0f);
                Ray scattered;

                for (s32 i = 0; i < bounces; ++i) {
                    Hit hit = scan_hit(scene, &ray);
                    v3 p = ray.origin + hit.t * ray.dir;

                    if (hit.t < MAX_DIST) {
                        Material *material = hit.material;

                        v3 catt;
                        if (!scatter(material, &ray, p, hit.n, &catt)) {
                            attenuation = vec3(0);
                            break;
                        }

                        attenuation = attenuation * catt;
                    } else {
                        break;
                    }
                }

                /*
                f32 t = 0.5f * (ray.dir.y + 1.0f);
                v3 color = (1.0f - t) * vec3(1, 1, 1) + t * vec3(0.5f, 0.7f, 1.0f);*/

                output = output + attenuation * vec3(0.5f, 0.7f, 1.0f);
			}

            output = output / rays_per_pixel;
            output = pow(output, 0.5f);

            data[yy * w + xx] = rgb_to_hex(output);
        }
    }
}

void raytrace_data(Scene *scene, u32 *data, s32 w, s32 h, s32 cores) {
    WorkQueue queue;
    
    s32 ts = w / cores;

    s32 tiles_x = (w + ts - 1) / ts;
    s32 tiles_y = (h + ts - 1) / ts;
    s32 tiles_count = tiles_x * tiles_y;

    queue.tiles = (Tile *)malloc(tiles_count * sizeof(Tile));
    queue.tile_count = tiles_count;
    queue.tile_index = 0;

    for (s32 y = 0; y < tiles_y; ++y) {
        for (s32 x = 0; x < tiles_x; ++x) {
            s32 tx = x * ts;
            s32 ty = y * ts;

            s32 tw = ts;
            s32 th = ts;

            if (tx + tw > w) {
                tw = w - tx;
            }

            if (ty + th > h) {
                th = h - ty;
            }

            queue.tiles[y * tiles_x + x] = {tx, ty, tw, th};
        }
    }
    
    std::vector<std::thread> threads;

    auto loop = [&]() {
        while (queue.tile_index < queue.tile_count) {
            raytrace_tile(&queue, scene, data, w, h);

            s32 percentage = (s32)((f32)(queue.tile_index + 0) / (f32)queue.tile_count * 100);
            printf("\rRaytrace %3d%%", percentage);
            fflush(stdout);
        }
    };

    for (s32 i = 0; i < cores; ++i) {
        std::thread t(loop);

        threads.push_back(move(t));
    }

    for (auto &t : threads) {
        t.join();
    }
}

u32 *raytrace(Scene *scene, s32 w, s32 h, s32 cores) {
    u32 *data = (u32 *)malloc(w * h * sizeof(u32));

    raytrace_data(scene, data, w, h, cores);

    return data;
}
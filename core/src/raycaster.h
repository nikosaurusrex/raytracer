#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <atomic>

#include "ray_math.h"

#define ARR_LEN(x) (sizeof(x)/sizeof(*x))

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

struct Camera {
	v3 pos;
	v3 hori;
	v3 vert;
	v3 llc;
	v3 u;
	v3 v;
	f32 lens_radius;
};

enum material_kind {
	MATT,
	METALLIC,
	DIALECTRIC
};

struct Material {
	u32 kind;
	v3 albedo;
};

struct Ray {
	lane_v3 origin;
	lane_v3 dir;
};

struct Hit {
	lane_f32 t;
	lane_v3 n;
	lane_u32 material_index;
};

struct Sphere {
	v3 center;
	f32 radius;
	u32 material_index;
};

struct Plane {
	u32 material_index;
	f32 z;
};

struct Scene {
	Plane *planes;
	u32 num_planes;

	Sphere *spheres;
	u32 num_spheres;

	Material *materials;
	u32 num_materials;

	Camera camera;
};

struct Tile {
	Random random;
	u32 x;
	u32 y;
	u32 w;
	u32 h;
};

struct WorkQueue {
	Tile *tiles;
	u32 tile_count;

	std::atomic<u32> tile_index;
	std::atomic<u64> total_bounces;
};

Material make_matt(v3 albedo);
Material make_metallic(v3 albedo);

Sphere make_sphere(v3 center, f32 radius, u32 material_index);
Plane make_plane(f32 z, u32 material_index);

Camera make_camera(f32 fov, v3 pos, v3 lookat, f32 focus_dist, f32 aperture, u32 width, u32 height);

f32 clamp(f32 v, f32 l, f32 h);
u32 rgb_to_hex(v3 v);

void raytrace_tile(WorkQueue *queue, Scene *scene, u32 *data, u32 w, u32 h);

void raytrace_data(Scene *scene, u32 *data, u32 w, u32 h, u32 cores);
u32 *raytrace(Scene *scene, u32 w, u32 h, u32 cores);

#endif

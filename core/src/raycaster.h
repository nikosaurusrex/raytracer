#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <atomic>

#include "vec.h"

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
	s32 kind;
	v3 albedo;
};

struct Ray {
	v3 origin;
	v3 dir;
};

struct Hit {
	f32 t;
	v3 n;
	Material *material;
};

struct Sphere {
	Material *material;
	f32 radius;
	v3 center;
};

struct Plane {
	Material *material;
	f32 z;
};

struct Scene {
	Plane **planes;
	Sphere **spheres;

	s32 num_planes;
	s32 num_spheres;

	Camera camera;
};

struct Tile {
	s32 x;
	s32 y;
	s32 w;
	s32 h;
};

struct WorkQueue {
	Tile *tiles;
	s32 tile_count;

	std::atomic<s32> tile_index;
};

Material *make_matt(v3 albedo);
Material *make_metallic(v3 albedo);

Sphere *make_sphere(v3 center, f32 radius, Material *mat);
Plane *make_plane(f32 z, Material *mat);

Camera make_camera(f32 fov, v3 pos, v3 lookat, f32 focus_dist, f32 aperture, s32 width, s32 height);

f32 clamp(f32 v, f32 l, f32 h);
u32 rgb_to_hex(v3 v);

void raytrace_tile(WorkQueue *queue, Scene *scene, u32 *data, s32 w, s32 h);

void raytrace_data(Scene *scene, u32 *data, s32 w, s32 h, s32 cores);
u32 *raytrace(Scene *scene, s32 w, s32 h, s32 cores);

#endif

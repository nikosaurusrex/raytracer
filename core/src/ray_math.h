#ifndef RAYCASTER_MATH_H
#define RAYCASTER_MATH_H

#include <stdint.h>
#include <math.h>

typedef uint32_t u32;
typedef float f32;

const u32 u32_max = (u32)-1;

struct Random {
	u32 state;
};

inline u32 random_u32(Random *random) {
	u32 x = random->state;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	random->state = x;

	return x;
}

inline f32 randomf(Random *random) {
	return (f32)random_u32(random) / (f32) u32_max;
}

inline f32 randomf2(Random *random) {
	return 2.0f * randomf(random) - 1.0f;
}

#define LANE_WIDTH 1
#if (LANE_WIDTH == 1)

typedef u32 lane_u32;
typedef f32 lane_f32;

inline lane_u32 lane_u32_create(u32 v) {
	return v;
}

inline lane_f32 lane_f32_create(f32 v) {
	return v;
}

inline lane_f32 lane_f32_from_u32(lane_u32 a) {
	return (lane_f32) a;
}

#elif (LANE_WIDTH == 8)

#include <immintrin.h>

struct lane_u32 {
	__m256i v;
};

struct lane_f32 {
	__m256 v;
};

inline lane_u32 lane_u32_create(u32 v) {
	return { _mm256_set1_epi32(v) };
}

inline lane_u32 operator<<(lane_u32 a, u32 b) {
	return { _mm256_slli_epi32(a.v, b) };
}

inline lane_u32 operator>>(lane_u32 a, u32 b) {
	return { _mm256_srli_epi32(a.v, b) };
}

inline lane_u32 operator^(lane_u32 a, lane_u32 b) {
	return { _mm256_xor_si256(a.v, b.v) };
}

inline lane_u32 &operator^=(lane_u32 &a, lane_u32 b) {
	a.v = _mm256_xor_si256(a.v, b.v);
	return a;
}

inline lane_f32 lane_f32_create(f32 v) {
	return { _mm256_set1_ps(v) };
}

inline lane_f32 lane_f32_from_u32(lane_u32 a) {
	return { _mm256_cvtepi32_ps(a.v) };
}

inline lane_f32 operator+(lane_f32 a, lane_f32 b) {
	return { _mm256_add_ps(a.v, b.v) };
}

inline lane_f32 &operator+=(lane_f32 &a, lane_f32 b) {
	a.v = _mm256_add_ps(a.v, b.v);
	return a;
}

inline lane_f32 operator-(lane_f32 a, lane_f32 b) {
	return { _mm256_sub_ps(a.v, b.v) };
}

inline lane_f32 operator*(lane_f32 a, lane_f32 b) {
	return { _mm256_mul_ps(a.v, b.v) };
}

inline lane_f32 operator/(lane_f32 a, lane_f32 b) {
	return { _mm256_div_ps(a.v, b.v) };
}

inline lane_f32 operator-(lane_f32 a) {
	return { _mm256_sub_ps(_mm256_set1_ps(0.0f), a.v) };
}

inline lane_f32 sqrtf(lane_f32 a) {
	/* note: maybe use _mm256_rsqrt_ps, which is 1/approx(sqrt) but much faster */
	return { _mm256_sqrt_ps(a.v) };
}

#else
#error "Lane width has to be 1, or 8"
#endif

union v3 {
	struct {
		f32 x;
		f32 y;
		f32 z;
	};
	struct {
		f32 r;
		f32 g;
		f32 b;
	};
};

inline v3 vec3(f32 v) {
	return { v, v, v };
}

inline v3 vec3(f32 x, f32 y, f32 z) {
	return { x, y, z };
}

inline v3 operator+(v3 a, v3 b) {
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline v3 operator-(v3 a, v3 b) {
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline v3 operator*(v3 a, v3 b) {
	return { a.x *b.x, a.y *b.y, a.z *b.z };
}

inline v3 operator*(v3 a, f32 v) {
	return { a.x *v, a.y *v, a.z *v };
}

inline v3 operator*(f32 v, v3 a) {
	return { a.x *v, a.y *v, a.z *v };
}

inline v3 operator/(v3 a, v3 b) {
	return { a.x / b.x, a.y / b.y, a.z / b.z };
}

inline v3 operator/(v3 a, f32 v) {
	return { a.x / v, a.y / v, a.z / v };
}

inline v3 operator/(f32 v, v3 a) {
	return { a.x / v, a.y / v, a.z / v };
}

inline v3 operator-(v3 a) {
	return {-a.x, -a.y, -a.z};
}

inline f32 dot(v3 a, v3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline f32 length(v3 a) {
	return sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline f32 length2(v3 a) {
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

inline v3 normalize(v3 a) {
	return (1.0f / length(a)) * a;
}

inline v3 cross(v3 a, v3 b) {
	f32 xx = a.y * b.z - a.z * b.y;
	f32 yy = a.z * b.x - a.x * b.z;
	f32 zz = a.x * b.y - a.y * b.x;
	return { xx, yy, zz };
}

inline v3 pow(v3 a, f32 p) {
	return { powf(a.x, p), powf(a.y, p), powf(a.z, p) };
}

inline v3 pow(v3 a, v3 b) {
	return { powf(a.x, b.x), powf(a.y, b.y), powf(a.z, b.z) };
}

inline v3 reflect(v3 a, v3 b) {
	return a - 2.0f * dot(a, b) * b;
}

inline v3 random_vec3(Random *random) {
    v3 v;

    do {
        v = lane_f32_create(2.0f) * vec3(randomf(random), randomf(random), randomf(random)) - vec3(1.0);
    } while (length2(v) >= lane_f32_create(1.0f));

    return v;
}

#if (LANE_WIDTH == 1)

typedef v3 lane_v3;

inline lane_v3 lane_vec3(lane_f32 v) {
	return { v, v, v };
}

inline lane_v3 lane_v3_from_v3(v3 v) {
	return v;
}

inline lane_v3 random_vec3_lane(Random *random) {
	v3 v = random_vec3(random);
	return v;
}

#else

union lane_v3 {
	struct {
		lane_f32 x;
		lane_f32 y;
		lane_f32 z;
	};
	struct {
		lane_f32 r;
		lane_f32 g;
		lane_f32 b;
	};
};

inline lane_v3 lane_vec3(lane_f32 v) {
	return { v, v, v };
}

inline lane_v3 lane_vec3(lane_f32 x, lane_f32 y, lane_f32 z) {
	return { x, y, z };
}

inline lane_v3 operator+(lane_v3 a, lane_v3 b) {
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline lane_v3 operator-(lane_v3 a, lane_v3 b) {
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline lane_v3 operator*(lane_v3 a, lane_v3 b) {
	return { a.x * b.x, a.y * b.y, a.z * b.z };
}

inline lane_v3 operator*(lane_v3 a, lane_f32 v) {
	return { a.x * v, a.y * v, a.z * v };
}

inline lane_v3 operator*(lane_f32 v, lane_v3 a) {
	return { a.x * v, a.y * v, a.z * v };
}

inline lane_v3 operator/(lane_v3 a, lane_v3 b) {
	return { a.x / b.x, a.y / b.y, a.z / b.z };
}

inline lane_v3 operator/(lane_v3 a, lane_f32 v) {
	return { a.x / v, a.y / v, a.z / v };
}

inline lane_v3 operator/(lane_f32 v, lane_v3 a) {
	return { a.x / v, a.y / v, a.z / v };
}

inline lane_v3 operator-(lane_v3 a) {
	return {-a.x, -a.y, -a.z};
}

inline lane_f32 dot(lane_v3 a, lane_v3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline lane_f32 length(lane_v3 a) {
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline lane_f32 length2(lane_v3 a) {
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

inline lane_v3 normalize(lane_v3 a) {
	return (lane_f32_create(1.0f) / length(a)) * a;
}

inline lane_v3 cross(lane_v3 a, lane_v3 b) {
	lane_f32 xx = a.y * b.z - a.z * b.y;
	lane_f32 yy = a.z * b.x - a.x * b.z;
	lane_f32 zz = a.x * b.y - a.y * b.x;
	return { xx, yy, zz };
}

inline lane_v3 reflect(lane_v3 a, lane_v3 b) {
	return a - lane_f32_create(2.0f) * dot(a, b) * b;
}

inline lane_v3 lane_v3_from_v3(v3 v) {
	lane_v3 v;

	v.x = { _mm256_set1_ps(v.x) };
	v.y = { _mm256_set1_ps(v.y) };
	v.z = { _mm256_set1_ps(v.z) };
	
	return v;
}

inline lane_v3 random_vec3_lane(Random *random) {
	v3 a = random_vec3();
	v3 b = random_vec3();
	v3 c = random_vec3();
	v3 d = random_vec3();
	v3 e = random_vec3();
	v3 f = random_vec3();
	v3 g = random_vec3();
	v3 h = random_vec3();

	lane_v3 v;
	v.x = { _mm256_set_ps(a.x, b.x, c.x, d.x, e.x, f.x, g.x, h.x) };
	v.y = { _mm256_set_ps(a.y, b.y, c.y, d.y, e.y, f.y, g.y, h.y) };
	v.z = { _mm256_set_ps(a.z, b.z, c.z, d.z, e.z, f.z, g.z, h.z) };
	return v;
}

#endif

#endif

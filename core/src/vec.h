#ifndef RAYCASTER_MATH_H
#define RAYCASTER_MATH_H

#include <math.h>

typedef float f32;

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

#ifdef RAYTRACER_SIMD
#include <immintrin.h>

union v8 {
	__m256 v;
	struct {
		f32 a;
		f32 b;
		f32 c;
		f32 d;
		f32 e;
		f32 f;
		f32 g;
		f32 h;
	};
};

inline v8 v8_create(f32 a) {
	v8 v;
	v.v = _mm256_set1_ps(a);
	return v;
}

inline v8 v8_create(__m256 a) {
	v8 v;
	v.v = a;
	return v;
}

inline v8 operator-(v8 a, v8 b) {
	v8 r;
	r.v = _mm256_sub_ps(a.v, b.v);
	return r;
}

inline v8 operator*(v8 a, v8 b) {
	v8 r;
	r.v = _mm256_mul_ps(a.v, b.v);
	return r;
}

inline v8 operator*(f32 b, v8 a) {
	v8 v;

	__m256 val = _mm256_set1_ps(b);
	v.v = _mm256_mul_ps(val, a.v);

	return v;
}

inline v8 operator/(v8 a, v8 b) {
	v8 r;
	r.v = _mm256_div_ps(a.v, b.v);
	return r;
}

inline v8 operator/(f32 b, v8 a) {
	v8 v;

	__m256 val = _mm256_set1_ps(b);
	v.v = _mm256_div_ps(val, a.v);

	return v;
}

struct v3x8 {
	__m256 x;
	__m256 y;
	__m256 z;
};

inline v3x8 v3x8_create(f32 a) {
	v3x8 v;

	v.x = _mm256_set1_ps(a);
	v.y = _mm256_set1_ps(a);
	v.z = _mm256_set1_ps(a);

	return v;
}

inline v3x8 v3x8_create(v3 a) {
	v3x8 v;

	v.x = _mm256_setr_ps(a.x, a.x, a.x, a.x, a.x, a.x, a.x, a.x);
	v.y = _mm256_setr_ps(a.y, a.y, a.y, a.y, a.y, a.y, a.y, a.y);
	v.z = _mm256_setr_ps(a.z, a.z, a.z, a.z, a.z, a.z, a.z, a.z);

	return v;
}

inline v3x8 v3x8_create(v3 a, v3 b, v3 c, v3 d, v3 e, v3 f, v3 g, v3 h) {
	v3x8 v;

	v.x = _mm256_setr_ps(a.x, b.x, c.x, d.x, e.x, f.x, g.x, h.x);
	v.y = _mm256_setr_ps(a.y, b.y, c.y, d.y, e.y, f.y, g.y, h.y);
	v.z = _mm256_setr_ps(a.z, b.z, c.z, d.z, e.z, f.z, g.z, h.z);

	return v;
}

inline v3x8 v3x8_create(v8 x, v8 y, v8 z) {
	v3x8 v;

	v.x = _mm256_setr_ps(x.a, x.b, x.c, x.d, x.e, x.f, x.g, x.h);
	v.y = _mm256_setr_ps(y.a, y.b, y.c, y.d, y.e, y.f, y.g, y.h);
	v.z = _mm256_setr_ps(z.a, z.b, z.c, z.d, z.e, z.f, z.g, z.h);

	return v;
}

inline v3x8 operator+(v3x8 a, v3x8 b) {
	v3x8 v;

	v.x = _mm256_add_ps(a.x, b.x);
	v.y = _mm256_add_ps(a.y, b.y);
	v.z = _mm256_add_ps(a.z, b.z);

	return v;
}

inline v3x8 operator-(v3x8 a, v3x8 b) {
	v3x8 v;

	v.x = _mm256_sub_ps(a.x, b.x);
	v.y = _mm256_sub_ps(a.y, b.y);
	v.z = _mm256_sub_ps(a.z, b.z);

	return v;
}

inline v3x8 operator*(v3x8 a, v3x8 b) {
	v3x8 v;

	v.x = _mm256_mul_ps(a.x, b.x);
	v.y = _mm256_mul_ps(a.y, b.y);
	v.z = _mm256_mul_ps(a.z, b.z);

	return v;
}

inline v3x8 operator*(v3x8 a, v8 b) {
	v3x8 v;

	v.x = _mm256_mul_ps(a.x, b.v);
	v.y = _mm256_mul_ps(a.y, b.v);
	v.z = _mm256_mul_ps(a.z, b.v);

	return v;
}

inline v3x8 operator*(v8 b, v3x8 a) {
	v3x8 v;

	v.x = _mm256_mul_ps(a.x, b.v);
	v.y = _mm256_mul_ps(a.y, b.v);
	v.z = _mm256_mul_ps(a.z, b.v);

	return v;
}

inline v3x8 operator/(v3x8 a, v3x8 b) {
	v3x8 v;

	v.x = _mm256_div_ps(a.x, b.x);
	v.y = _mm256_div_ps(a.y, b.y);
	v.z = _mm256_div_ps(a.z, b.z);

	return v;
}

inline v3x8 operator/(v3x8 a, v8 b) {
	v3x8 v;

	v.x = _mm256_div_ps(a.x, b.v);
	v.y = _mm256_div_ps(a.y, b.v);
	v.z = _mm256_div_ps(a.z, b.v);

	return v;
}

inline v3x8 operator/(v8 b, v3x8 a) {
	v3x8 v;

	v.x = _mm256_div_ps(a.x, b.v);
	v.y = _mm256_div_ps(a.y, b.v);
	v.z = _mm256_div_ps(a.z, b.v);

	return v;
}

inline v3x8 operator-(v3x8 a) {
	v3x8 v;
	
	v.x = _mm256_xor_ps(a.x, _mm256_set1_ps(-0.0));
	v.y = _mm256_xor_ps(a.y, _mm256_set1_ps(-0.0));
	v.z = _mm256_xor_ps(a.z, _mm256_set1_ps(-0.0));

	return v;
}

inline v8 dot(v3x8 a, v3x8 b) {
	__m256 xx = _mm256_mul_ps(a.x, b.x);
	__m256 yy = _mm256_mul_ps(a.y, b.y);
	__m256 zz = _mm256_mul_ps(a.z, b.z);

	__m256 r = _mm256_add_ps(xx, _mm256_add_ps(yy, zz));

	v8 v;
	v.v = r;
	return v;
}

inline v8 length(v3x8 a) {
	__m256 xx = _mm256_mul_ps(a.x, a.x);
	__m256 yy = _mm256_mul_ps(a.y, a.y);
	__m256 zz = _mm256_mul_ps(a.z, a.z);

	__m256 r = _mm256_add_ps(xx, _mm256_add_ps(yy, zz));
	r = _mm256_sqrt_ps(r);

	v8 v;
	v.v = r;
	return v;
}

inline v8 length2(v3x8 a) {
	__m256 xx = _mm256_mul_ps(a.x, a.x);
	__m256 yy = _mm256_mul_ps(a.y, a.y);
	__m256 zz = _mm256_mul_ps(a.z, a.z);

	__m256 r = _mm256_add_ps(xx, _mm256_add_ps(yy, zz));

	v8 v;
	v.v = r;
	return v;
}

inline v3x8 normalize(v3x8 a) {
	return (1.0f / length(a)) * a;
}

inline v3x8 reflect(v3x8 a, v3x8 b) {
	return a - 2.0f * dot(a, b) * b;
}

#endif

#endif

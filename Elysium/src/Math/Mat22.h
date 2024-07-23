#pragma once
#include "Vec2.h"

#include <assert.h>

// 2-by-2 matrix, stored in column major order
struct Mat22
{
	Mat22() {}
	Mat22(float angle)
	{
		float cosA = std::cosf(angle), sinA = std::sinf(angle);
		col1.x = cosA; col2.x = -sinA;
		col1.y = sinA; col2.y =  cosA;
	}

	Mat22(const Vec2& c1, const Vec2& c2) : col1(c1), col2(c2) {}

	Mat22(float a11, float a12, float a21, float a22)
	{
		col1.x = a11; col2.x = a12;
		col1.y = a21; col2.y = a22;
	}

	void Set(const Vec2& c1, const Vec2& c2)
	{
		col1 = c1;
		col2 = c2;
	}

	void SetIdentity()
	{
		col1.x = 1.0f; col2.x = 0.0f;
		col1.y = 0.0f; col2.y = 1.0f;
	}

	void SetZero()
	{
		col1.x = 0.0f; col2.x = 0.0f;
		col1.y = 0.0f; col2.y = 0.0f;
	}

	Mat22 Transpose() const
	{
		return Mat22(col1.x, col1.y, col2.x, col2.y);
	}

	Mat22 Invert() const
	{
		float a = col1.x, b = col2.x, c = col1.y, d = col2.y;
		Mat22 B;
		float det = a * d - b * c;
		assert(det != 0.0f);
		det = 1.0f / det;
		B.col1.x =  det * d; B.col2.x = -det * b;
		B.col1.y = -det * c; B.col2.y =  det * a;
		return B;
	}

	Vec2 col1, col2;
};

inline Vec2 operator * (const Mat22& A, const Vec2& v)
{
	return Vec2(A.col1.x * v.x + A.col2.x * v.y, A.col1.y * v.x + A.col2.y * v.y);
}

inline Mat22 operator + (const Mat22& A, const Mat22& B)
{
	return Mat22(A.col1 + B.col1, A.col2 + B.col2);
}

inline Mat22 operator * (const Mat22& A, const Mat22& B)
{
	return Mat22(A * B.col1, A * B.col2);
}
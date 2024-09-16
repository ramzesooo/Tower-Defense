#pragma once
#include <iostream>

#include <cmath>

class Vector2D
{
public:
	float x;
	float y;

	Vector2D() 
	{ 
		x = 0.0f;
		y = 0.0f;
	}
	Vector2D(float x, float y) 
	{ 
		this->x = x;
		this->y = y; 
	}

	inline Vector2D& Add(const Vector2D& vec)
	{ 
		this->x += vec.x;
		this->y += vec.y;
		return *this;
	}
	inline Vector2D& Subtract(const Vector2D& vec)
	{
		this->x -= vec.x;
		this->y -= vec.y;
		return *this;
	}
	inline Vector2D& Multiply(const Vector2D& vec)
	{
		this->x *= vec.x;
		this->y *= vec.y;
		return *this;
	}
	inline Vector2D& Divide(const Vector2D& vec)
	{
		this->x /= vec.x;
		this->y /= vec.y;
		return *this;
	}

	friend inline Vector2D& operator+(Vector2D& v1, const Vector2D& v2)
	{
		return v1.Add(v2);
	}
	friend inline Vector2D& operator-(Vector2D& v1, const Vector2D& v2)
	{
		return v1.Subtract(v2);
	}
	friend inline Vector2D& operator*(Vector2D& v1, const Vector2D& v2)
	{
		return v1.Multiply(v2);
	}
	friend inline Vector2D& operator/(Vector2D& v1, const Vector2D& v2)
	{
		return v1.Divide(v2);
	}

	inline Vector2D& operator+=(const Vector2D& vec)
	{
		return this->Add(vec);
	}
	inline Vector2D& operator-=(const Vector2D& vec)
	{
		return this->Subtract(vec);
	}
	inline Vector2D& operator*=(const Vector2D& vec)
	{
		return this->Multiply(vec);
	}
	inline Vector2D& operator/=(const Vector2D& vec)
	{
		return this->Divide(vec);
	}

	template<typename T>
	inline Vector2D& operator*(T& i)
	//Vector2D& operator*(const int& i)
	{
		this->x *= i;
		this->y *= i;

		return *this;
	}
	inline Vector2D& Zero()
	{
		this->x = 0.0f;
		this->y = 0.0f;
		return *this;
	}

	inline Vector2D& Roundf()
	{
		this->x = std::roundf(this->x);
		this->y = std::roundf(this->y);
		return *this;
	}

	friend inline std::ostream& operator<<(std::ostream& stream, const Vector2D& vec)
	{
		stream << "(" << vec.x << ", " << vec.y << ")";
		return stream;
	}
};
#pragma once
#include <iostream>
#include <cmath>

class Vector2D
{
public:
	float x = 0.0f, y = 0.0f;

	Vector2D() = default;
	Vector2D(float setX, float setY) : x(setX), y(setY) {}
	Vector2D(const Vector2D &r) : x(r.x), y(r.y) {}
	~Vector2D() = default;

	auto operator<=>(const Vector2D&) const = default;

	Vector2D &operator=(const Vector2D &r)
	{
		if (this == &r)
			return *this;

		this->x = r.x;
		this->y = r.y;

		return *this;
	}

	inline Vector2D &Add(const Vector2D& vec)
	{ 
		this->x += vec.x;
		this->y += vec.y;
		return *this;
	}
	inline Vector2D &Subtract(const Vector2D& vec)
	{
		this->x -= vec.x;
		this->y -= vec.y;
		return *this;
	}
	inline Vector2D &Multiply(const Vector2D& vec)
	{
		this->x *= vec.x;
		this->y *= vec.y;
		return *this;
	}
	inline Vector2D &Divide(const Vector2D& vec)
	{
		this->x /= vec.x;
		this->y /= vec.y;
		return *this;
	}

	friend inline Vector2D &operator+(Vector2D& v1, const Vector2D& v2)
	{
		return v1.Add(v2);
	}
	friend inline Vector2D &operator-(Vector2D& v1, const Vector2D& v2)
	{
		return v1.Subtract(v2);
	}
	friend inline Vector2D &operator*(Vector2D& v1, const Vector2D& v2)
	{
		return v1.Multiply(v2);
	}
	friend inline Vector2D &operator/(Vector2D& v1, const Vector2D& v2)
	{
		return v1.Divide(v2);
	}

	inline Vector2D &operator+=(const Vector2D& vec)
	{
		return this->Add(vec);
	}
	inline Vector2D &operator-=(const Vector2D& vec)
	{
		return this->Subtract(vec);
	}
	inline Vector2D &operator*=(const Vector2D& vec)
	{
		return this->Multiply(vec);
	}
	inline Vector2D &operator/=(const Vector2D& vec)
	{
		return this->Divide(vec);
	}

	template<typename T>
	inline Vector2D operator+(const T& i)
	{
		return Vector2D(this->x + i, this->y + i);
	}

	template<typename T>
	inline Vector2D operator-(const T& i)
	{
		return Vector2D(this->x - i, this->y - i);
	}

	template<typename T>
	inline Vector2D operator*(const T& i)
	{
		return Vector2D(this->x * i, this->y * i);
	}
	
	template<typename T>
	inline Vector2D operator/(const T& i)
	{
		return Vector2D(this->x / i, this->y / i);
	}

	template<typename T>
	inline Vector2D &operator+=(const T& i)
	{
		this->x += i;
		this->y += i;

		return *this;
	}
	template<typename T>
	inline Vector2D &operator-=(const T& i)
	{
		this->x -= i;
		this->y -= i;

		return *this;
	}
	template<typename T>
	inline Vector2D &operator*=(const T& i)
	{
		this->x *= i;
		this->y *= i;

		return *this;
	}

	template <typename T>
	inline Vector2D &operator/=(const T& i)
	{
		this->x /= i;
		this->y /= i;

		return *this;
	}

	inline Vector2D &Zero()
	{
		this->x = 0.0f;
		this->y = 0.0f;
		return *this;
	}

	inline Vector2D Roundf() const
	{
		return Vector2D(std::roundf(x), std::roundf(y));
	}

	inline Vector2D Trunc() const
	{
		return Vector2D(std::trunc(x), std::trunc(y));
	}

	inline bool IsEqualZero() const
	{
		return x == 0.0f && y == 0.0f;
	}

	friend inline std::ostream& operator<<(std::ostream& stream, const Vector2D& vec)
	{
		stream << "(" << vec.x << ", " << vec.y << ")";
		return stream;
	}
};

namespace std {
	template <>
	struct hash<Vector2D>
	{
		std::size_t operator()(Vector2D x) const
		{
			std::string_view bytes{ reinterpret_cast<const char*>(&x), sizeof(x) };
			return std::hash<std::string_view>{}(bytes);
		}
	};
}
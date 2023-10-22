#pragma once
#include <math.h>

class Vector
{
public:
	Vector(const float* clr)
    {
        x = clr[0];
        y = clr[1];
        z = clr[2];
    }

    Vector( ) { this->x = 0; this->y = 0; this->z = 0; };
    Vector(float fx, float fy, float fz)
    {
        this->x = fx;
        this->y = fy;
        this->z = fz;
    }

    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f)
    {
        x = ix; y = iy; z = iz;
    }

    Vector operator*(float fl)
    {
        return Vector(x * fl, y * fl, z * fl);
    }

    Vector operator*(const Vector& v)
    {
        return Vector(x * v.x, y * v.y, z * v.z);
    }

	__inline void Mul(float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
	}

    float &operator[](int i)
    {
        return ((float*)this)[i];
    }
	
    float operator[](int i) const
    {
        return ((float*)this)[i];
    }

	bool __inline IsZero()
	{
		return (!x && !y && !z);
	}

    void __inline Zero()
    {
        x = y = z = 0.0f;
    }

    bool operator==(const Vector &src) const
    {
        return (src.x == x) && (src.y == y) && (src.z == z);
    }
	
    bool operator!=(const Vector &src) const
    {
        return (src.x != x) || (src.y != y) || (src.z != z);
    }

    Vector &operator+=(const Vector &v)
    {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }
	
    Vector &operator-=(const Vector &v)
    {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }
	
    Vector &operator*=(float fl)
    {
        x *= fl;
        y *= fl;
        z *= fl;
        return *this;
    }
	
    Vector &operator*=(const Vector &v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
	
    Vector &operator/=(const Vector &v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }
	
    Vector &operator+=(float fl)
    {
        x += fl;
        y += fl;
        z += fl;
        return *this;
    }
	
    Vector &operator/=(float fl)
    {
        x /= fl;
        y /= fl;
        z /= fl;
        return *this;
    }
	
    Vector &operator-=(float fl)
    {
        x -= fl;
        y -= fl;
        z -= fl;
        return *this;
    }

    void NormalizeInPlace()
    {
        *this = Normalized();
    }
	
    Vector Normalized() const
    {
        Vector res = *this;
        float l = res.Length();
        if(l != 0.0f) {
            res /= l;
        } else {
            res.x = res.y = res.z = 0.0f;
        }
        return res;
    }

	float Normalize() const
	{
		Vector res = *this;
		float l = res.Length();
		if (l != 0.0f)
		{
			res /= l;
		}
		else
		{
			res.x = res.y = res.z = 0.0f;
		}
		return l;
	}

    Vector ToEulerAngles(Vector* pseudoup = nullptr)
    {
        auto pitch = 0.0f;
        auto yaw = 0.0f;
        auto roll = 0.0f;

        auto length = Length2D();

        auto deg2rad = [](int flRadians) -> float
        {
            return flRadians * (180.0f / 3.14159265359);
        };

        if (pseudoup)
        {
            auto left = pseudoup->Cross(*this);

            left.Normalize();

            pitch = deg2rad(atan2(-this->z, length));

            if (pitch < 0.0f)
                pitch += 360.0f;

            if (length > 0.001f) {
                yaw = deg2rad(atan2(this->y, this->x));

                if (yaw < 0.0f)
                    yaw += 360.0f;

                auto up_z = (this->x * left.y) - (this->y * left.x);

                roll = deg2rad(atan2(left.z, up_z));

                if (roll < 0.0f)
                    roll += 360.0f;
            }
            else {
                yaw = deg2rad(atan2(-left.x, left.y));

                if (yaw < 0.0f)
                    yaw += 360.0f;
            }
        }
        else {
            if (this->x == 0.0f && this->y == 0.0f) {
                if (this->z > 0.0f)
                    pitch = 270.0f;
                else
                    pitch = 90.0f;
            }
            else {
                pitch = deg2rad(atan2(-this->z, length));

                if (pitch < 0.0f)
                    pitch += 360.0f;

                yaw = deg2rad(atan2(this->y, this->x));

                if (yaw < 0.0f)
                    yaw += 360.0f;
            }
        }

        return Vector( pitch, yaw, roll );
    }

    float DistTo(const Vector &vOther) const
    {
        Vector delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.Length();
    }
	
    float DistToSqr(const Vector &vOther) const
    {
        Vector delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;
        delta.z = z - vOther.z;

        return delta.LengthSqr();
    }
	
    // shit pasted from other projects.
    __forceinline void to_vectors(Vector& right, Vector& up) {
        Vector tmp;
        if (x == 0.f && y == 0.f)
        {
            // pitch 90 degrees up/down from identity.
            right[0] = 0.f;
            right[1] = -1.f;
            right[2] = 0.f;
            up[0] = -z;
            up[1] = 0.f;
            up[2] = 0.f;
        }
        else
        {
            tmp[0] = 0; tmp[1] = 0; tmp[2] = 1;

            // get directions vector using cross product.
            right = Cross(tmp).Normalized();
            up = right.Cross(*this).Normalized();
        }
    }

    float Dot(const Vector &vOther) const
    {
        return (x*vOther.x + y*vOther.y + z*vOther.z);
    }

	void VectorCrossProduct(const Vector &a, const Vector &b, Vector &result)
	{
		result.x = a.y*b.z - a.z*b.y;
		result.y = a.z*b.x - a.x*b.z;
		result.z = a.x*b.y - a.y*b.x;
	}

	Vector Cross(const Vector &vOther)
	{
		Vector res;
		VectorCrossProduct(*this, vOther, res);
		return res;
	}
	
    float Length() const
    {
        return sqrt(x*x + y*y + z*z);
    }
	
    float LengthSqr(void) const
    {
        return (x*x + y*y + z*z);
    }
	
    float Length2DSqr(void) const
    {
        return (x * x + y * y );
    }

    float Length2D() const
    {
        return sqrt(x*x + y*y);
    }

    Vector &operator=(const Vector &vOther)
    {
        x = vOther.x; y = vOther.y; z = vOther.z;
        return *this;
    }

    Vector operator-(void) const
    {
        return Vector(-x, -y, -z);
    }
	
    Vector operator+(const Vector &v) const
    {
        return Vector(x + v.x, y + v.y, z + v.z);
    }

	Vector operator+(float fl) const
	{
		return Vector(x + fl, y + fl, z + fl);
	}
	
    Vector operator-(const Vector &v) const
    {
        return Vector(x - v.x, y - v.y, z - v.z);
    }
	
	Vector operator-(float fl) const
	{
		return Vector(x - fl, y - fl, z - fl);
	}

    Vector operator*(float fl) const
    {
        return Vector(x * fl, y * fl, z * fl);
    }
	
    Vector operator*(const Vector &v) const
    {
        return Vector(x * v.x, y * v.y, z * v.z);
    }
	
    Vector operator/(float fl) const
    {
        return Vector(x / fl, y / fl, z / fl);
    }
	
    Vector operator/(const Vector &v) const
    {
        return Vector(x / v.x, y / v.y, z / v.z);
    }

    bool Vector::operator < (const Vector& v) {
        return { this->x < v.x&&
                 this->y < v.y&&
                 this->z < v.z };
    }

    bool Vector::operator > (const Vector& v) {
        return { this->x > v.x &&
                 this->y > v.y &&
                 this->z > v.z };
    }

    bool Vector::operator<=(const Vector& v) {
        return { this->x <= v.x &&
                 this->y <= v.y &&
                 this->z <= v.z };
    }

    bool Vector::operator>=(const Vector& v) {
        return { this->x >= v.x &&
                 this->y >= v.y &&
                 this->z >= v.z };
    }

	float x;
	float y;
	float z;
};
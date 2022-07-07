#pragma once
#include <DirectXMath.h>
using namespace DirectX;

namespace GxEngine
{
#define RAD2DEG 57.295780f;
#define DEG2RAD 0.0174533f
	constexpr float DegToRad(float deg)
	{
		return deg * DEG2RAD;
	}
	constexpr float RadToDeg(float rad)
	{
		return rad * RAD2DEG;
	}
	struct Vector2
	{
	public:
		union
		{
			struct
			{
				float x;
				float y;
			};
			XMFLOAT2 XM;
		};

		static Vector2 zero()
		{
			Vector2 res;
			XMStoreFloat2(&res.XM, XMVectorZero());
			return res;
		}
		static Vector2 one()
		{
			Vector2 res;
			res.x = res.y = 1;
			return res;
		}

		Vector2() = default;
		Vector2(float num)
		{
			x = y = num;
		}
		Vector2(float X, float Y)
		{
			x = X;
			y = Y;
		}
		Vector2(const Vector2& v)
		{
			XMStoreFloat2(&XM, XMLoadFloat2(&v.XM));
		}
		inline float SqurMagnitude()
		{
			float f;
			XMStoreFloat(&f, XMVector2LengthSq(XMLoadFloat2(&XM)));
			return f;
		}
		inline float Magnitude()
		{
			float f;
			XMStoreFloat(&f, XMVector2Length(XMLoadFloat2(&XM)));
			return f;
		}
		inline void Normalize()
		{
			XMStoreFloat2(&XM, XMVector2Normalize(XMLoadFloat2(&XM)));
		}
		inline Vector2 Normalized()
		{
			Vector2 res = *this;
			res.Normalize();
			return res;
		}

		inline Vector2 operator +(Vector2& a)
		{
			Vector2 res;
			XMStoreFloat2(&res.XM, XMVectorAdd(XMLoadFloat2(&this->XM), XMLoadFloat2(&a.XM)));
			return res;
		}
		inline Vector2 operator -(Vector2& a)
		{
			Vector2 res;
			XMStoreFloat2(&res.XM, XMVectorSubtract(XMLoadFloat2(&this->XM), XMLoadFloat2(&a.XM)));
			return res;
		}
		static inline float Dot(Vector2 a, Vector2 b)
		{
			float f;
			XMStoreFloat(&f, XMVector2Dot(XMLoadFloat2(&a.XM), XMLoadFloat2(&b.XM)));
			return f;
		}
		static inline Vector2 Cross(Vector2 a, Vector2 b)
		{
			Vector2 res;
			XMStoreFloat2(&res.XM, XMVector2Cross(XMLoadFloat2(&a.XM), XMLoadFloat2(&b.XM)));
			return res;
		}
	};
	struct Vector3
	{
	public:
		union
		{
			struct
			{
				float x;
				float y;
				float z;
			};
			XMFLOAT3 XM;
		};
		static Vector3 zero()
		{
			Vector3 res;
			XMStoreFloat3(&res.XM, XMVectorZero());
			return res;
		}
		static Vector3 one()
		{
			Vector3 res;
			res.x = res.y = res.z = 1;
			return res;
		}
		Vector3() = default;
		Vector3(float num)
		{
			x = y = z = num;
		}
		Vector3(float X, float Y, float Z)
		{
			x = X;
			y = Y;
			z = Z;
		}
		Vector3(const Vector3& v)
		{
			XMStoreFloat3(&XM, XMLoadFloat3(&v.XM));
		}
		inline float SqurMagnitude()
		{
			float f;
			XMStoreFloat(&f, XMVector3LengthSq(XMLoadFloat3(&XM)));
			return f;
		}
		inline float Magnitude()
		{
			float f;
			XMStoreFloat(&f, XMVector3Length(XMLoadFloat3(&XM)));
			return f;
		}
		inline Vector3 Normalize()
		{
			XMStoreFloat3(&XM, XMVector3Normalize(XMLoadFloat3(&XM)));
			return *this;
		}
		inline Vector3 Normalized()
		{
			Vector3 res = *this;
			res.Normalize();
			return res;
		}

		inline Vector3 operator +(Vector3& a)
		{
			Vector3 res;
			XMStoreFloat3(&res.XM, XMVectorAdd(XMLoadFloat3(&this->XM), XMLoadFloat3(&a.XM)));
			return res;
		}
		inline Vector3 operator -(Vector3& a)
		{
			Vector3 res;
			XMStoreFloat3(&res.XM, XMVectorSubtract(XMLoadFloat3(&this->XM), XMLoadFloat3(&a.XM)));
			return res;
		}
		inline Vector3 operator *(float s)
		{
			Vector3 res;
			XMStoreFloat3(&res.XM, XMLoadFloat3(&this->XM) * s);
			return res;
		}
		inline Vector3 operator /(float s)
		{
			Vector3 res;
			XMStoreFloat3(&res.XM, XMLoadFloat3(&this->XM) / s);
			return res;
		}
		inline void operator +=(Vector3& a)
		{
			XMStoreFloat3(&XM, XMVectorAdd(XMLoadFloat3(&this->XM), XMLoadFloat3(&a.XM)));
		}
		inline void operator -=(Vector3& a)
		{
			XMStoreFloat3(&XM, XMVectorSubtract(XMLoadFloat3(&this->XM), XMLoadFloat3(&a.XM)));
		}
		static inline float Dot(Vector3 a, Vector3 b)
		{
			float f;
			XMStoreFloat(&f, XMVector3Dot(XMLoadFloat3(&a.XM), XMLoadFloat3(&b.XM)));
			return f;
		}
		static inline Vector3 Cross(Vector3 a, Vector3 b)
		{
			Vector3 res;
			XMStoreFloat3(&res.XM, XMVector3Cross(XMLoadFloat3(&a.XM), XMLoadFloat3(&b.XM)));
			return res;
		}

	};
	struct Quaternion
	{
	public:
		union
		{
			struct
			{
				float x;
				float y;
				float z;
				float w;
			};
			XMFLOAT4 XM;
		};
		static Quaternion zero()
		{
			Quaternion res;
			XMStoreFloat4(&res.XM, XMVectorZero());
			return res;
		}
		static Quaternion one()
		{
			Quaternion res;
			res.x = res.y = res.z = 1;
			return res;
		}
		static Quaternion identity()
		{
			Quaternion res;
			XMStoreFloat4(&res.XM, XMQuaternionIdentity());
			return res;
		}
		Quaternion() = default;
		Quaternion(float num)
		{
			x = y = z = num;
		}
		Quaternion(float X, float Y, float Z, float W)
		{
			x = X;
			y = Y;
			z = Z;
			w = W;
		}
		Quaternion(const Quaternion& v)
		{
			XMStoreFloat4(&XM, XMLoadFloat4(&v.XM));
		}
		Quaternion(const XMVECTORF32& v)
		{
			XMStoreFloat4(&XM, v);
		}
		inline float SqurMagnitude()
		{
			float f;
			XMStoreFloat(&f, XMVector4LengthSq(XMLoadFloat4(&XM)));
			return f;
		}
		inline float Magnitude()
		{
			float f;
			XMStoreFloat(&f, XMVector4Length(XMLoadFloat4(&XM)));
			return f;
		}
		inline void Normalize()
		{
			XMStoreFloat4(&XM, XMVector4Normalize(XMLoadFloat4(&XM)));
		}
		inline Quaternion Normalized()
		{
			Quaternion res = *this;
			res.Normalize();
			return res;
		}
		inline Quaternion operator +(Quaternion& a)
		{
			Quaternion res;
			XMStoreFloat4(&res.XM, XMVectorAdd(XMLoadFloat4(&this->XM), XMLoadFloat4(&a.XM)));
			return res;
		}
		inline Quaternion operator -(Quaternion& a)
		{
			Quaternion res;
			XMStoreFloat4(&res.XM, XMVectorSubtract(XMLoadFloat4(&this->XM), XMLoadFloat4(&a.XM)));
			return res;
		}
		inline Quaternion operator *(Quaternion& a)
		{
			Quaternion res;
			XMStoreFloat4(&res.XM, XMQuaternionMultiply(XMLoadFloat4(&this->XM), XMLoadFloat4(&a.XM)));
			return res;
		}
		inline Vector3 operator *(Vector3& a)
		{
			Vector3 res;
			XMStoreFloat3(&res.XM, XMVector3Rotate(XMLoadFloat3(&a.XM), XMLoadFloat4(&XM)));
			//XMMatrixRotaionQu
			//XMVector3Transform
			//XMStoreFloat4(&res.XM, XMQuaternion(XMLoadFloat4(&this->XM), XMLoadFloat4(&a.XM)));
			return res;
		}
		static inline float Dot(Quaternion a, Quaternion b)
		{
			float f;
			XMStoreFloat(&f, XMVector4Dot(XMLoadFloat4(&a.XM), XMLoadFloat4(&b.XM)));
			return f;
		}
		static inline Quaternion Cross(Quaternion a, Quaternion b)
		{
			Quaternion res;
			XMStoreFloat4(&res.XM, XMVector3Cross(XMLoadFloat4(&a.XM), XMLoadFloat4(&b.XM)));
			return res;
		}
		static inline Quaternion RollPitchYaw(float pitch, float yaw, float roll)
		{
			Quaternion res;
			XMStoreFloat4(&res.XM, XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
			return res;
		}
	};
	struct Matrix4x4
	{
	public:
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			float m[4][4];
			XMFLOAT4X4 XM;
		};

		static inline Matrix4x4 identity()
		{
			Matrix4x4 res;
			XMStoreFloat4x4(&res.XM, XMMatrixIdentity());
			return res;
		}
		static inline Matrix4x4 Translation(Vector3 v)
		{
			Matrix4x4 m;
			XMMATRIX xmm = XMMatrixTranslation(v.x, v.y, v.z);
			XMStoreFloat4x4(&m.XM, xmm);
		}
		static inline Matrix4x4 TRS(Vector3 t, Quaternion r, Vector3 s)
		{
			Matrix4x4 m;
			XMMATRIX xmm = XMMatrixScalingFromVector(XMLoadFloat3(&s.XM))
				* XMMatrixRotationQuaternion(XMLoadFloat4(&r.XM))
				* XMMatrixTranslation(t.x, t.y, t.z);
			XMStoreFloat4x4(&m.XM, xmm);
			return m;
		}
		Matrix4x4() = default;
		Matrix4x4(const Matrix4x4& v)
		{
			XMStoreFloat4x4(&XM, XMLoadFloat4x4(&v.XM));
		}

		inline Matrix4x4 Invers()
		{
			XMMatrixInverse(&XMMatrixDeterminant(XMLoadFloat4x4(&XM)), XMLoadFloat4x4(&XM));
			return *this;
		}

		inline Matrix4x4 Inversed()
		{
			Matrix4x4 m = *this;
			return m.Invers();
		}

		inline Matrix4x4 Transpose()
		{
			XMStoreFloat4x4(&XM, XMMatrixTranspose(XMLoadFloat4x4(&XM)));
			return *this;
		}

		inline Matrix4x4 Transposed()
		{
			Matrix4x4 m = *this;
			return m.Transpose();

		}
		inline Matrix4x4 operator * (Matrix4x4& m)
		{
			Matrix4x4 res;
			XMStoreFloat4x4(&res.XM, XMMatrixMultiply(XMLoadFloat4x4(&this->XM), XMLoadFloat4x4(&m.XM)));
			return res;
		}
		static inline Matrix4x4 LookAt(Vector3 eyePoint, Vector3 target)
		{
			Matrix4x4 res;
			XMStoreFloat4x4(&res.XM, XMMatrixLookAtLH(XMLoadFloat3(&eyePoint.XM), XMLoadFloat3(&target.XM), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
			return res;
		}
		static inline Matrix4x4 LookTo(Vector3 eyePoint, Vector3 direction)
		{
			Matrix4x4 res;
			XMStoreFloat4x4(&res.XM, XMMatrixLookToLH(XMLoadFloat3(&eyePoint.XM), XMLoadFloat3(&direction.XM), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
			return res;
		}

		static inline Matrix4x4 Perspective(float yFov/*degree*/, float aspectRatio, float nearZ, float farZ)
		{
			Matrix4x4 res;
			XMStoreFloat4x4(&res.XM, XMMatrixPerspectiveFovLH(yFov * 3.1415926f / 180.f, aspectRatio, nearZ, farZ));
			return res;
		}
	};

}


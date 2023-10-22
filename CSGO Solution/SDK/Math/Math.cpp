#include "Math.hpp"
#include "../Globals.hpp"

void Math::FixMovement( void* pCmd )
{
	C_UserCmd* pUserCmd = reinterpret_cast < C_UserCmd* > ( pCmd );
	if ( !pUserCmd || !pUserCmd->m_nCommand )
		return;

	QAngle wish;
	g_Globals.m_Interfaces.m_EngineClient->GetViewAngles(&wish);

	float y; // xmm3_4
	float v9; // xmm4_4
	float x; // xmm0_4
	float v11; // xmm3_4
	float v12; // xmm4_4
	float v13; // xmm0_4
	float v14; // xmm5_4
	float v15; // xmm5_4
	float v16; // xmm5_4
	float v17; // xmm7_4
	float v18; // xmm6_4
	float v19; // xmm3_4
	float v21; // [esp+Ch] [ebp-74h]
	float v22; // [esp+Ch] [ebp-74h]
	float z; // [esp+10h] [ebp-70h]
	float v24; // [esp+10h] [ebp-70h]
	float v25; // [esp+14h] [ebp-6Ch]
	float v26; // [esp+14h] [ebp-6Ch]
	float v27; // [esp+18h] [ebp-68h]
	float v28; // [esp+1Ch] [ebp-64h]
	float v29; // [esp+20h] [ebp-60h]
	float v30; // [esp+24h] [ebp-5Ch]
	float v31; // [esp+28h] [ebp-58h]
	float v32; // [esp+28h] [ebp-58h]
	float v33; // [esp+2Ch] [ebp-54h]
	float v34; // [esp+30h] [ebp-50h]
	float v35; // [esp+34h] [ebp-4Ch]
	float v36; // [esp+38h] [ebp-48h]
	float v37; // [esp+40h] [ebp-40h]
	QAngle  input; // [esp+44h] [ebp-3Ch] BYREF
	Vector forward; // [esp+50h] [ebp-30h] BYREF
	Vector right; // [esp+5Ch] [ebp-24h] BYREF
	QAngle v41; // [esp+68h] [ebp-18h] BYREF
	Vector up; // [esp+74h] [ebp-Ch] BYREF

	input = wish;
	v41 = pUserCmd->m_angViewAngles;

	if ( wish.yaw == v41.yaw && wish.pitch == v41.pitch && wish.roll == v41.roll )
		return;

	const float move_magnitude = std::sqrt((pUserCmd->m_flForwardMove * pUserCmd->m_flForwardMove) + (pUserCmd->m_flSideMove * pUserCmd->m_flSideMove));

	if ( move_magnitude == 0.f )
		return;

	AngleVectors(input, forward, right, up);

	y = forward.y;
	z = forward.z;

	v37 = 1.f;

	if ( forward.z == 0.f )
		x = forward.x;
	else {

		z = 0.f;
		v9 = forward.Length2D();

		if ( v9 >= 0.00000011920929f ) {
			y = forward.y * (1.f / v9);
			v35 = forward.x * (1.f / v9);
			goto LABEL_13;
		}

		y = 0.f;
		x = 0.f;
	}

	v35 = x;

LABEL_13:
	v36 = y;
	v11 = right.y;
	v31 = right.z;

	if ( right.z == 0.0 ) {
		v13 = right.x;
		goto LABEL_18;
	}

	v31 = 0.f;
	v12 = right.Length2D();

	if ( v12 < 0.00000011920929f ) {
		v11 = 0.f;
		v13 = 0.f;
	LABEL_18:
		v21 = v13;
		goto LABEL_19;
	}

	v11 = right.y * (1.f / v12);
	v21 = right.x * (1.f / v12);

LABEL_19:
	if ( up.z < 0.00000011920929f )
		v25 = 0.f;
	else
		v25 = 1.f;

	if ( v41.roll == 180.0 /*a7 parameter*/ )
		pUserCmd->m_flForwardMove = std::abs(pUserCmd->m_flForwardMove);

	AngleVectors(v41, right, forward, up);

	v33 = right.z;
	if ( right.z == 0.f ) {
		v27 = right.y;
		v28 = right.x;
	}
	else {

		v33 = 0.f;
		v14 = right.Length2D();

		if ( v14 < 0.00000011920929f ) {
			v27 = 0.f;
			v28 = 0.f;
		}
		else
		{
			v28 = right.x * (1.f / v14);
			v27 = right.y * (1.f / v14);
		}
	}

	v34 = forward.z;
	if ( forward.z == 0.0 ) {
		v29 = forward.y;
		v30 = forward.x;
	}
	else {

		v34 = 0.f;
		v15 = forward.Length2D();

		if ( v15 < 0.00000011920929f ) {
			v29 = 0.f;
			v30 = 0.f;
		}
		else {
			v30 = forward.x * (1.f / v15);
			v29 = forward.y * (1.f / v15);
		}
	}

	if ( up.z < 0.00000011920929f )
		v37 = 0.f;

	v16 = v11 * pUserCmd->m_flSideMove;
	v24 = z * pUserCmd->m_flForwardMove;
	v22 = v21 * pUserCmd->m_flSideMove;
	v17 = v35 * pUserCmd->m_flForwardMove;
	v18 = v36 * pUserCmd->m_flForwardMove;
	v26 = v25 * pUserCmd->m_flUpMove;
	v32 = v31 * pUserCmd->m_flSideMove;
	v19 = pUserCmd->m_flUpMove * 0.f;

	pUserCmd->m_flForwardMove = ((((v16 * v27) + (v22 * v28)) + (v32 * v33)) + (((v18 * v27) + (v17 * v28)) + (v24 * v33))) + (((v19 * v27) + (v19 * v28)) + (v26 * v33));
	pUserCmd->m_flSideMove = ((((v16 * v29) + (v22 * v30)) + (v32 * v34)) + (((v18 * v29) + (v17 * v30)) + (v24 * v34))) + (((v19 * v29) + (v19 * v30)) + (v26 * v34));
	pUserCmd->m_flUpMove = ((((v16 * 0.f) + (v22 * 0.f)) + (v32 * v37)) + (((v18 * 0.f) + (v17 * 0.f)) + (v24 * v37))) + (((v19 * 0.f) + (v19 * 0.f)) + (v26 * v37));
}

static bool screen_transform(const Vector& in, Vector& out)
{
	static auto& w2sMatrix = g_Globals.m_Interfaces.m_EngineClient->WorldToScreenMatrix();

	out.x = w2sMatrix.m[0][0] * in.x + w2sMatrix.m[0][1] * in.y + w2sMatrix.m[0][2] * in.z + w2sMatrix.m[0][3];
	out.y = w2sMatrix.m[1][0] * in.x + w2sMatrix.m[1][1] * in.y + w2sMatrix.m[1][2] * in.z + w2sMatrix.m[1][3];
	out.z = 0.0f;

	float w = w2sMatrix.m[3][0] * in.x + w2sMatrix.m[3][1] * in.y + w2sMatrix.m[3][2] * in.z + w2sMatrix.m[3][3];

	if (w < 0.001f) {
		out.x *= 100000;
		out.y *= 100000;
		return false;
	}

	out.x /= w;
	out.y /= w;

	return true;
}

bool Math::WorldToScreen(const Vector& in, Vector& out)
{
	if (screen_transform(in, out)) {
		int w, h;
		g_Globals.m_Interfaces.m_EngineClient->GetScreenSize(w, h);

		out.x = (w / 2.0f) + (out.x * w) / 2.0f;
		out.y = (h / 2.0f) - (out.y * h) / 2.0f;

		return true;
	}
	return false;
}

void Math::VectorVectors(Vector& forward, Vector& right, Vector& up) {
	if (fabs(forward.x) < 1e-6 && fabs(forward.y) < 1e-6)
	{
		// pitch 90 degrees up/down from identity.
		right = Vector(0, -1, 0);
		up = Vector(-forward.z, 0, 0);
	}
	else
	{
		// get directions vector using cross product.
		right = forward.Cross(Vector(0, 0, 1)).Normalized();
		up = right.Cross(forward).Normalized();
	}
}

void Math::MatrixCopy(const matrix3x4_t& in, matrix3x4_t& out)
{
	//Assert(s_bMathlibInitialized);
	memcpy(out.Base(), in.Base(), sizeof(float) * 3 * 4);
}

void Math::ConcatTransforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out)
{
	//Assert(s_bMathlibInitialized);
	if (&in1 == &out)
	{
		matrix3x4_t in1b;
		MatrixCopy(in1, in1b);
		ConcatTransforms(in1b, in2, out);
		return;
	}
	if (&in2 == &out)
	{
		matrix3x4_t in2b;
		MatrixCopy(in2, in2b);
		ConcatTransforms(in1, in2b, out);
		return;
	}

	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
		in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
		in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
		in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
		in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
		in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
		in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
		in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
		in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
		in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
		in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
		in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
		in1[2][2] * in2[2][3] + in1[2][3];
}

void Math::PointSystemAngleMatrix(const Vector& angles, matrix3x4_t& matrix)
{
	float sr, sp, sy, cr, cp, cy;

	DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles.y));
	DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles.x));
	DirectX::XMScalarSinCos(&sr, &cr, DEG2RAD(angles.z));

	// matrix = (YAW * PITCH) * ROLL
	matrix[0][0] = cp * cy;
	matrix[1][0] = cp * sy;
	matrix[2][0] = -sp;

	// NOTE: Do not optimize this to reduce multiplies! optimizer bug will screw this up.
	matrix[0][1] = sr * sp * cy + cr * -sy;
	matrix[1][1] = sr * sp * sy + cr * cy;
	matrix[2][1] = sr * cp;
	matrix[0][2] = (cr * sp * cy + -sr * -sy);
	matrix[1][2] = (cr * sp * sy + -sr * cy);
	matrix[2][2] = cr * cp;

	matrix[0][3] = 0.0f;
	matrix[1][3] = 0.0f;
	matrix[2][3] = 0.0f;
}
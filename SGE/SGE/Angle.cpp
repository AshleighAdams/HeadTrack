#include "Angle.h"

CAngle::CAngle(double P, double Y, double R)
{
	Pitch = P;
	Yaw = Y;
	Roll = R;
}

CAngle::CAngle()
{
	Pitch = Yaw = Roll = 0;
}

CVector CAngle::ToVector() const
{
	return CVector(Pitch, Yaw);
}

CVector CAngle::Forward() const
{
	return CVector(Pitch, Yaw);
}

CVector CAngle::Right() const
{
	const CVector crossright(0, 0, 1);
	return CVector(0.0, Yaw).Cross(crossright);
}

CVector CAngle::Up() const
{
	return CVector(0.0, 0.0);
}


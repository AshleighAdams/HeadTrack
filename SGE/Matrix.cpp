#ifdef NOTHING_NOT_DEF

#include "Matrix.h"

CMatrix::CMatrix(void)
{
	for(int i = 0; i < 9; i++)
		m_Values[i] = 0;
}

double& CMatrix::operator[](int i)
{
	return m_Values[i];
}

CMatrix CMatrix::FromAngle(const CAngle& A)
{
	CMatrix xM;
	CMatrix yM;
	CMatrix zM;

	double sinX = sin(A.Pitch / 180.0 * 3.1415926);
	double cosX = sin(A.Pitch / 180.0 * 3.1415926);

	double sinY = sin(A.Roll / 180.0 * 3.1415926);
	double cosY = sin(A.Roll / 180.0 * 3.1415926);

	double sinZ = sin(A.Yaw / 180.0 * 3.1415926);
	double cosZ = sin(A.Yaw / 180.0 * 3.1415926);

	// rotation about x-axis (pitch)
	xM[0] = 1.0f; xM[1] = 0.0f; xM[2] = 0.0f;
	xM[3] = 0.0f; xM[4] = cosX; xM[5] = sinX;
	xM[6] = 0.0f; xM[7] = -sinX; xM[8] = cosX;
	 
	// rotation about y-axis (roll)
	yM[0] = cosY; yM[1] = 0.0f; yM[2] = sinY;
	yM[3] = 0.0f; yM[4] = 1.0f; yM[5] = 0.0f;
	yM[6] = -sinY; yM[7] = 0.0f; yM[8] = cosY;
	 
	// rotation about z-axis (azimuth)
	zM[0] = cosZ; zM[1] = sinZ; zM[2] = 0.0f;
	zM[3] = -sinZ; zM[4] = cosZ; zM[5] = 0.0f;
	zM[6] = 0.0f; zM[7] = 0.0f; zM[8] = 1.0f;

	xM.Multiply(yM);
	zM.Multiply(xM);

	return zM;
}

CMatrix CMatrix::Multiply(CMatrix& A)
{
	CMatrix ret;
	ret[0] = m_Values[0] * A[0] + m_Values[1] * A[3] + m_Values[2] * A[6];
	ret[1] = m_Values[0] * A[1] + m_Values[1] * A[4] + m_Values[2] * A[7];
	ret[2] = m_Values[0] * A[2] + m_Values[1] * A[5] + m_Values[2] * A[8];
	 
	ret[3] = m_Values[3] * A[0] + m_Values[4] * A[3] + m_Values[5] * A[6];
	ret[4] = m_Values[3] * A[1] + m_Values[4] * A[4] + m_Values[5] * A[7];
	ret[5] = m_Values[3] * A[2] + m_Values[4] * A[5] + m_Values[5] * A[8];
	 
	ret[6] = m_Values[6] * A[0] + m_Values[7] * A[3] + m_Values[8] * A[6];
	ret[7] = m_Values[6] * A[1] + m_Values[7] * A[4] + m_Values[8] * A[7];
	ret[8] = m_Values[6] * A[2] + m_Values[7] * A[5] + m_Values[8] * A[8];
	return ret;
}

CAngle CMatrix::GetAngle()
{
	CAngle ret;
	ret.Yaw		= atan2(m_Values[1], m_Values[4]);
	ret.Pitch	= asin(-m_Values[7]);
	ret.Roll	= atan2(-m_Values[6], m_Values[8]);
	
	return ret;
}

CMatrix::~CMatrix(void)
{
}

#endif
#ifndef MATRIX_H
#define MATRIX_H

#include "SGE/Engine.h"

class CMatrix
{
public:
	CMatrix(void);
	~CMatrix(void);
	CMatrix Multiply(CMatrix& A);
	CAngle GetAngle();
	static CMatrix FromAngle(const CAngle& A);
	double& operator[](int i);
private:
	double m_Values[9];
};

#endif
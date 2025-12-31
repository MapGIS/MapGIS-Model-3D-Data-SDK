#include "stdafx.h"
#include "g3dtilegaussian.h"



MapGIS::Tile::CoordinateSystemConverter  MapGIS::Tile::CGGaussianDataTrans::GetCoordinateSystemConverter(MapGIS::Tile::CoordinateSystem from, MapGIS::Tile::CoordinateSystem to)
{
	MapGIS::Tile::CoordinateSystemConverter rtn;
	auto aNum = static_cast<int>(from) - 1;
	auto bNum = static_cast<int>(to) - 1;
	if (aNum < 0 || bNum < 0 || from == to)
	{
		return rtn;
	}

	int x1 = (int)((int)from & 0B111100000000) >> 8;
	int y1 = (int)((int)from & 0B000011110000) >> 4;
	int z1 = (int)((int)from & 0B000000001111);

	int x2 = (int)((int)to & 0B111100000000) >> 8;
	int y2 = (int)((int)to & 0B000011110000) >> 4;
	int z2 = (int)((int)to & 0B000000001111);

	rtn.match[0] = ((x2 & 0B1110) == (x1 & 0B1110) ? 1 : 0)* (x2 == x1 ? 1 : -1);
	rtn.match[1] = ((x2 & 0B1110) == (y1 & 0B1110) ? 1 : 0)* (x2 == y1 ? 1 : -1);
	rtn.match[2] = ((x2 & 0B1110) == (z1 & 0B1110) ? 1 : 0)* (x2 == z1 ? 1 : -1);
	rtn.match[3] = ((y2 & 0B1110) == (x1 & 0B1110) ? 1 : 0)* (y2 == x1 ? 1 : -1);
	rtn.match[4] = ((y2 & 0B1110) == (y1 & 0B1110) ? 1 : 0)* (y2 == y1 ? 1 : -1);
	rtn.match[5] = ((y2 & 0B1110) == (z1 & 0B1110) ? 1 : 0)* (y2 == z1 ? 1 : -1);
	rtn.match[6] = ((z2 & 0B1110) == (x1 & 0B1110) ? 1 : 0)* (z2 == x1 ? 1 : -1);
	rtn.match[7] = ((z2 & 0B1110) == (y1 & 0B1110) ? 1 : 0)* (z2 == y1 ? 1 : -1);
	rtn.match[8] = ((z2 & 0B1110) == (z1 & 0B1110) ? 1 : 0)* (z2 == z1 ? 1 : -1);
	
	return rtn;
};


std::array<float, 3> MapGIS::Tile::CGGaussianDataTrans::TransPoints(std::array<float, 3> orPoints, std::array<float, 9> matrix)
{
	float	rtnx = matrix[0] * orPoints[0] + matrix[1] * orPoints[1] + matrix[2] * orPoints[2];
	float	rtny = matrix[3] * orPoints[0] + matrix[4] * orPoints[1] + matrix[5] * orPoints[2];
	float	rtnz = matrix[6] * orPoints[0] + matrix[7] * orPoints[1] + matrix[8] * orPoints[2];
	return{ rtnx ,rtny ,rtnz };
}

std::array<float, 4>  MapGIS::Tile::CGGaussianDataTrans::quatMultiply(const std::array<float, 4> & a, const std::array<float, 4>& b)
{
	// x component: w1*x2 + x1*w2 + y1*z2 - z1*y2
	float x = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];

	// y component: w1*y2 - x1*z2 + y1*w2 + z1*x2
	float y = a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0];

	// z component: w1*z2 + x1*y2 - y1*x2 + z1*w2
	float z = a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3];

	// w component: w1*w2 - x1*x2 - y1*y2 - z1*z2
	float w = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];

	return{ x,y,z,w };
}


// 单位化四元数
std::array<float, 4> MapGIS::Tile::CGGaussianDataTrans::normalize(std::array<float, 4> q)
{
	float norm = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];

	if (norm < 1e-8f) {
		// 防止除以0，返回默认四元数（无旋转）
		return{ 0.0f, 0.0f, 0.0f, 1.0f };
	}

	norm = sqrt(norm);
	float invNorm = 1.0f / norm;

	return{
		q[0] * invNorm,
		q[1] * invNorm,
		q[2] * invNorm,
		q[3] * invNorm
	};
}


std::array<float, 4> MapGIS::Tile::CGGaussianDataTrans::TransRotations(std::array<float, 4> orRotations, std::array<float, 9> matrix) {

	//使用四元数共轭  
	//源四元数 orQ = [w,x,y,z]
	//目标desQ =  alignQ * orQ * alignQ（负1次方）


	//matrix = axesMatch(CoordinateSystem::LDB, CoordinateSystem::RDB);

	//float value1 = matrix[0] * (matrix[4] * matrix[8] - matrix[5] * matrix[7]);
	//float value2 = matrix[1] * (matrix[3] * matrix[8] - matrix[5] * matrix[6]);
	//float value3 = matrix[2] * (matrix[3] * matrix[7] - matrix[4] * matrix[6]);
	//if ((value1 - value2 + value3)<0)
	//{//存在左右手转换
	//	//将x取反向，
	//	orRotations[0] *= -1;
	//	matrix[0] *= -1;
	//	matrix[1] *= -1;
	//	matrix[2] *= -1;
	//}

	//1、从旋转矩阵提取四元数alignQ
	float alignW = 0;
	float alignX = 0;
	float alignY = 0;
	float alignZ = 0;

	float alignInvW = 0;
	float alignInvX = 0;
	float alignInvY = 0;
	float alignInvZ = 0;

	float trace = matrix[0] + matrix[4] + matrix[8];
	if (trace > 0)
	{
		float temp = sqrt(trace + 1) * 2;
		alignW = 0.25 * temp;
		alignX = (matrix[7] - matrix[5]) / temp;
		alignY = (matrix[2] - matrix[6]) / temp;
		alignZ = (matrix[3] - matrix[1]) / temp;
	}
	else if (matrix[0] > matrix[4] && matrix[0] > matrix[8])
	{
		float temp = sqrt(1.0 + matrix[0] - matrix[4] - matrix[8]) * 2;
		alignW = (matrix[7] - matrix[5]) / temp;
		alignX = 0.25 * temp;
		alignY = (matrix[1] + matrix[3]) / temp;
		alignZ = (matrix[2] + matrix[6]) / temp;

	}
	else if (matrix[4] > matrix[8])
	{
		float temp = sqrt(1.0 + matrix[4] - matrix[0] - matrix[8]) * 2;
		alignW = (matrix[2] - matrix[6]) / temp;
		alignX = (matrix[1] + matrix[3]) / temp;
		alignY = 0.25 * temp;
		alignZ = (matrix[5] + matrix[7]) / temp;
	}
	else
	{
		float temp = sqrt(1.0 + matrix[8] - matrix[0] - matrix[4]) * 2;
		alignW = (matrix[3] - matrix[1]) / temp;
		alignX = (matrix[2] + matrix[6]) / temp;
		alignY = (matrix[5] + matrix[7]) / temp;
		alignZ = 0.25 * temp;
	}

	alignInvW = alignW;
	alignInvX = -alignX;
	alignInvY = -alignY;
	alignInvZ = -alignZ;

	std::array<float, 4> q_m = quatMultiply(orRotations, { alignInvX ,alignInvY ,alignInvZ ,alignInvW });
	std::array<float, 4> q_t = quatMultiply({ alignX ,alignY ,alignZ ,alignW }, q_m);

	/*std::array<float, 4> q_m = quatMultiply(orRotations, { alignX ,alignY ,alignZ ,alignW  });
	std::array<float, 4> q_t = quatMultiply({ alignInvX ,alignInvY ,alignInvZ ,alignInvW }, q_m);*/

	return q_t;
}


//or x y z
std::array<float, 3> MapGIS::Tile::CGGaussianDataTrans::TransScale(std::array<float, 3> orScale, std::array<float, 9> matrix)
{
	float	rtnx = fabs(matrix[0]) * orScale[0] + fabs(matrix[1]) * orScale[1] + fabs(matrix[2]) * orScale[2];
	float	rtny = fabs(matrix[3]) * orScale[0] + fabs(matrix[4]) * orScale[1] + fabs(matrix[5]) * orScale[2];
	float	rtnz = fabs(matrix[6]) * orScale[0] + fabs(matrix[7]) * orScale[1] + fabs(matrix[8]) * orScale[2];
	return{ rtnx ,rtny ,rtnz };
}


std::vector<float> MapGIS::Tile::CGGaussianDataTrans::TransSh(std::vector<float> &sh, std::array<float, 9> matrix)
{
	std::vector<float> rtn;

	if (sh.size() >= 9)
	{//一阶参数变换
		std::array<float, 3> value1 = rotate_sh1({ sh[0] ,sh[3] ,sh[6] }, matrix);
		std::array<float, 3> value2 = rotate_sh1({ sh[1] ,sh[4] ,sh[7] }, matrix);
		std::array<float, 3> value3 = rotate_sh1({ sh[2] ,sh[5] ,sh[8] }, matrix);

		for (int i = 0; i < 3; i++)
		{
			rtn.emplace_back(value1[i]);
			rtn.emplace_back(value2[i]);
			rtn.emplace_back(value3[i]);
		}

	}

	if (sh.size() >= 24)
	{//一阶参数变换

		std::array<float, 5> value1 = rotate_sh2({ sh[9]  ,sh[12] ,sh[15],sh[18] ,sh[21] }, matrix);
		std::array<float, 5> value2 = rotate_sh2({ sh[10] ,sh[13] ,sh[16],sh[19] ,sh[22] }, matrix);
		std::array<float, 5> value3 = rotate_sh2({ sh[11] ,sh[14] ,sh[17],sh[20] ,sh[23] }, matrix);

		for (int i = 0; i < 5; i++)
		{
			rtn.emplace_back(value1[i]);
			rtn.emplace_back(value2[i]);
			rtn.emplace_back(value3[i]);
		}
	}

	if (sh.size() >= 45)
	{//一阶参数变换
		std::array<float, 7> value1 = rotate_sh3({ sh[24] ,sh[27] ,sh[30],sh[33] ,sh[36],sh[39] ,sh[42] }, matrix);
		std::array<float, 7> value2 = rotate_sh3({ sh[25] ,sh[28] ,sh[31],sh[34] ,sh[37],sh[40] ,sh[43] }, matrix);
		std::array<float, 7> value3 = rotate_sh3({ sh[26] ,sh[29] ,sh[32],sh[35] ,sh[38],sh[41] ,sh[44] }, matrix);
		for (int i = 0; i < 7; i++)
		{
			rtn.emplace_back(value1[i]);
			rtn.emplace_back(value2[i]);
			rtn.emplace_back(value3[i]);
		}
	}
	return rtn;

}



std::array<double, 3> MapGIS::Tile::CGGaussianDataTrans::rotation_to_euler_zyz(const  std::array<float, 9> & R) {
	double beta = acos(R[8]);
	double alpha, gamma;

	const double eps = 1e-10;

	if (beta < eps) {
		alpha = atan2(R[3], R[0]);
		gamma = 0.0;
	}
	else if (fabs(beta - PI) < eps) {
		alpha = atan2(R[3], -R[0]);
		gamma = 0.0;
	}
	else {
		alpha = atan2(R[5], R[2]);
		gamma = atan2(R[7], -R[6]);
	}
	return{ alpha, beta, gamma };
}

// 构造 l=1 实值 Wigner D-矩阵
std::array<std::array<double, 3>, 3> MapGIS::Tile::CGGaussianDataTrans::wigner_d_matrix_l1(double alpha, double beta, double gamma) {
	std::array<std::array<double, 3>, 3> D = {};

	double ca = cos(alpha); double sa = sin(alpha);
	double cb = cos(beta); double sb = sin(beta);
	double cg = cos(gamma); double sg = sin(gamma);

	// d^{(1)}_{m'm}(beta)
	double d[3][3] = {
		{ 0.5*(1 + cb),   -sb / sqrt(2), 0.5*(1 - cb) },
		{ sb / sqrt(2),      cb,      -sb / sqrt(2) },
		{ 0.5*(1 - cb),   sb / sqrt(2),  0.5*(1 + cb) }
	};

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			int m = i - 1;
			int mp = j - 1;

			if (m == 0 && mp == 0) {
				D[i][j] = d[i][j];
			}
			else if (m > 0 && mp > 0) {
				D[i][j] = 0.5 * (d[i][j] * (ca*cg - sa*sg) + d[2 - i][j] * (ca*sg + sa*cg));
			}
			else if (m > 0 && mp < 0) {
				D[i][j] = 0.5 * (d[i][2 - j] * (ca*sg - sa*cg) + d[2 - i][2 - j] * (ca*cg + sa*sg));
			}
			else if (m < 0 && mp > 0) {
				D[i][j] = 0.5 * (d[2 - i][j] * (ca*sg + sa*cg) - d[i][j] * (ca*cg - sa*sg));
			}
			else if (m < 0 && mp < 0) {
				D[i][j] = 0.5 * (d[2 - i][2 - j] * (ca*cg + sa*sg) - d[i][2 - j] * (ca*sg - sa*cg));
			}
			else if (m == 0 && mp > 0) {
				D[i][j] = d[i][j] * (ca*cg - sa*sg);
			}
			else if (m == 0 && mp < 0) {
				D[i][j] = d[i][2 - j] * (ca*sg + sa*cg);
			}
			else if (m > 0 && mp == 0) {
				D[i][j] = d[i][1] * (ca*cg - sa*sg);
			}
			else if (m < 0 && mp == 0) {
				D[i][j] = d[2 - i][1] * (ca*sg + sa*cg);
			}
		}
	}

	return D;
}

// 使用 Wigner D-矩阵旋转一阶 SH 系数
std::array<float, 3> MapGIS::Tile::CGGaussianDataTrans::rotate_sh1(const std::array<float, 3>& coeffs,const std::array<float, 9>& R) {
	// Step 1: 提取欧拉角
	std::array<double, 3> euler = rotation_to_euler_zyz(R);
	double alpha = euler[0];
	double beta = euler[1];
	double gamma = euler[2];

	// Step 2: 构造 D^(1)(R)
	auto D = wigner_d_matrix_l1(alpha, beta, gamma);

	// Step 3: 被动变换：c' = D^T @ c
	std::array<float, 3> result = {};
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			result[i] += D[j][i] * coeffs[j];  // D^T[i][j] = D[j][i]
		}
	}
	return result;
}

std::array<float, 5> MapGIS::Tile::CGGaussianDataTrans::rotate_sh2(const std::array<float, 5>& coeffs, const std::array<float, 9>& R) {
	// Step 1: 提取欧拉角
	std::array<double, 3> euler = rotation_to_euler_zyz(R);
	double alpha = euler[0];
	double beta = euler[1];
	double gamma = euler[2];


	// Step 2: Precompute trig values
	double ca = cos(alpha); double sa =sin(alpha);
	double cb = cos(beta); double sb = sin(beta);
	double cg = cos(gamma); double sg = sin(gamma);
	double c2a = cos(2 * alpha); double  s2a = sin(2 * alpha);
	double c2g = cos(2 * gamma); double s2g = sin(2 * gamma);

	// Precompute small-d elements for l=2
	double d[5][5] = { { 0 } };

	auto set_d = [&](int i, int j, double val) {
		d[i][j] = val;
	};

	// m = -2
	set_d(0, 0, 0.25*(1 + cb)*(1 + cb));
	set_d(0, 1, -0.5*sb*(1 + cb));
	set_d(0, 2, 0.5*sb*sb);
	set_d(0, 3, -0.5*sb*(1 - cb));
	set_d(0, 4, 0.25*(1 - cb)*(1 - cb));

	// m = -1
	set_d(1, 0, 0.5*sb*(1 + cb));
	set_d(1, 1, 0.25*(1 + cb)*(1 - 3 * cb));
	set_d(1, 2, -sb*cb);
	set_d(1, 3, 0.25*(1 - cb)*(1 + 3 * cb));
	set_d(1, 4, -0.5*sb*(1 - cb));

	// m = 0
	set_d(2, 0, 0.5*sb*sb);
	set_d(2, 1, sb*cb);
	set_d(2, 2, cb*cb - 0.5*sb*sb);
	set_d(2, 3, -sb*cb);
	set_d(2, 4, 0.5*sb*sb);

	// m = +1
	set_d(3, 0, 0.5*sb*(1 - cb));
	set_d(3, 1, 0.25*(1 - cb)*(1 + 3 * cb));
	set_d(3, 2, sb*cb);
	set_d(3, 3, 0.25*(1 + cb)*(1 - 3 * cb));
	set_d(3, 4, -0.5*sb*(1 + cb));

	// m = +2
	set_d(4, 0, 0.25*(1 - cb)*(1 - cb));
	set_d(4, 1, -0.5*sb*(1 - cb));
	set_d(4, 2, 0.5*sb*sb);
	set_d(4, 3, -0.5*sb*(1 + cb));
	set_d(4, 4, 0.25*(1 + cb)*(1 + cb));

	// Step 3: Build real D-matrix (5x5), index: 0->m=-2, 1->-1, ..., 4->+2
	std::array<std::array<double, 5>, 5> D_real = {};

	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			int m = i - 2;
			int mp = j - 2;

			if (m == 0 && mp == 0) {
				D_real[i][j] = d[i][j];
			}
			else if (m > 0 && mp > 0) {
				D_real[i][j] = 0.5 * (d[i][j] * (ca*cg - sa*sg) + d[4 - i][j] * (ca*sg + sa*cg));
			}
			else if (m > 0 && mp < 0) {
				D_real[i][j] = 0.5 * (d[i][4 - j] * (ca*sg - sa*cg) + d[4 - i][4 - j] * (ca*cg + sa*sg));
			}
			else if (m < 0 && mp > 0) {
				D_real[i][j] = 0.5 * (d[4 - i][j] * (ca*sg + sa*cg) - d[i][j] * (ca*cg - sa*sg));
			}
			else if (m < 0 && mp < 0) {
				D_real[i][j] = 0.5 * (d[4 - i][4 - j] * (ca*cg + sa*sg) - d[i][4 - j] * (ca*sg - sa*cg));
			}
			else if (m == 0 && mp > 0) {
				D_real[i][j] = d[i][j] * (ca*cg - sa*sg);
			}
			else if (m == 0 && mp < 0) {
				D_real[i][j] = d[i][4 - j] * (ca*sg + sa*cg);
			}
			else if (m > 0 && mp == 0) {
				D_real[i][j] = d[i][2] * (ca*cg - sa*sg);
			}
			else if (m < 0 && mp == 0) {
				D_real[i][j] = d[4 - i][2] * (ca*sg + sa*cg);
			}
		}
	}

	// Step 4: Apply transformation: c' = D^T @ c
	std::array<float, 5> result = {};
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 5; ++j) {
			result[i] += D_real[j][i] * coeffs[j];  // D^T[i][j] = D[j][i]
		}
	}

	return result;
}

// 构造 l=3 的实值 Wigner D-矩阵 (7x7)
std::array<std::array<double, 7>, 7> MapGIS::Tile::CGGaussianDataTrans::wigner_d_matrix_l3(double alpha, double beta, double gamma) {
	std::array<std::array<double, 7>, 7> D = {};

	double ca = cos(alpha); double sa = sin(alpha);
	double cb = cos(beta); double sb = sin(beta);
	double cg = cos(gamma); double sg = sin(gamma);
	double c2a = cos(2 * alpha); double s2a = sin(2 * alpha);
	double c3a = cos(3 * alpha); double s3a = sin(3 * alpha);
	double c2g = cos(2 * gamma); double s2g = sin(2 * gamma);
	double c3g = cos(3 * gamma); double s3g = sin(3 * gamma);

	// 预计算小 d 矩阵 d^{(3)}_{m'm}(beta)
	double d[7][7] = { { 0 } };

	auto set_d = [&](int i, int j, double val) {
		d[i][j] = val;
	};

	// m = -3
	set_d(0, 0, 0.125 * (1 + cb)*(1 + cb)*(1 + cb));
	set_d(0, 1, -0.25*sb*(1 + cb)*(1 + cb)*sqrt(3));
	set_d(0, 2, 0.25*sb*sb*(1 + cb)*sqrt(3));
	set_d(0, 3, -0.25*sb*sb*sb);
	set_d(0, 4, 0.25*sb*sb*(1 - cb)*sqrt(3));
	set_d(0, 5, -0.25*sb*(1 - cb)*(1 - cb)*sqrt(3));
	set_d(0, 6, 0.125*(1 - cb)*(1 - cb)*(1 - cb));

	// m = -2
	set_d(1, 0, 0.25*sb*(1 + cb)*(1 + cb)*sqrt(3));
	set_d(1, 1, 0.25*(1 + cb)*(1 + cb)*(1 - 2 * cb)*sqrt(3) / sqrt(3)); // simplify
	d[1][1] = 0.25*(1 + cb)*(1 + cb)*(1 - 2 * cb);
	set_d(1, 2, -0.5*sb*(1 + cb)*cb*sqrt(3));
	set_d(1, 3, 0.5*sb*sb*cb*sqrt(3));
	set_d(1, 4, -0.5*sb*(1 - cb)*cb*sqrt(3));
	set_d(1, 5, 0.25*(1 - cb)*(1 - cb)*(1 + 2 * cb));
	set_d(1, 6, -0.25*sb*(1 - cb)*(1 - cb)*sqrt(3));

	// m = -1
	set_d(2, 0, 0.25*sb*sb*(1 + cb)*sqrt(3));
	set_d(2, 1, 0.5*sb*(1 + cb)*cb*sqrt(3));
	set_d(2, 2, 0.5*(1 + cb)*(1 - 3 * cb*cb));
	set_d(2, 3, -sb*cb*cb);
	set_d(2, 4, 0.5*(1 - cb)*(1 - 3 * cb*cb));
	set_d(2, 5, -0.5*sb*(1 - cb)*cb*sqrt(3));
	set_d(2, 6, 0.25*sb*sb*(1 - cb)*sqrt(3));

	// m = 0
	set_d(3, 0, 0.25*sb*sb*sb);
	set_d(3, 1, 0.5*sb*sb*cb*sqrt(3));
	set_d(3, 2, sb*cb*cb);
	set_d(3, 3, cb*cb*cb - 1.5*cb*sb*sb);
	set_d(3, 4, -sb*cb*cb);
	set_d(3, 5, -0.5*sb*sb*cb*sqrt(3));
	set_d(3, 6, -0.25*sb*sb*sb);

	// m = +1
	set_d(4, 0, 0.25*sb*sb*(1 - cb)*sqrt(3));
	set_d(4, 1, 0.5*sb*(1 - cb)*cb*sqrt(3));
	set_d(4, 2, 0.5*(1 - cb)*(1 - 3 * cb*cb));
	set_d(4, 3, -sb*cb*cb);
	set_d(4, 4, 0.5*(1 + cb)*(1 - 3 * cb*cb));
	set_d(4, 5, -0.5*sb*(1 + cb)*cb*sqrt(3));
	set_d(4, 6, 0.25*sb*sb*(1 + cb)*sqrt(3));

	// m = +2
	set_d(5, 0, 0.25*sb*(1 - cb)*(1 - cb)*sqrt(3));
	set_d(5, 1, 0.25*(1 - cb)*(1 - cb)*(1 + 2 * cb));
	set_d(5, 2, 0.5*sb*(1 - cb)*cb*sqrt(3));
	set_d(5, 3, -0.5*sb*sb*cb*sqrt(3));
	set_d(5, 4, 0.5*sb*(1 + cb)*cb*sqrt(3));
	set_d(5, 5, 0.25*(1 + cb)*(1 + cb)*(1 - 2 * cb));
	set_d(5, 6, -0.25*sb*(1 + cb)*(1 + cb)*sqrt(3));

	// m = +3
	set_d(6, 0, 0.125*(1 - cb)*(1 - cb)*(1 - cb));
	set_d(6, 1, 0.25*sb*(1 - cb)*(1 - cb)*sqrt(3));
	set_d(6, 2, 0.25*sb*sb*(1 - cb)*sqrt(3));
	set_d(6, 3, 0.25*sb*sb*sb);
	set_d(6, 4, 0.25*sb*sb*(1 + cb)*sqrt(3));
	set_d(6, 5, 0.25*sb*(1 + cb)*(1 + cb)*sqrt(3));
	set_d(6, 6, 0.125*(1 + cb)*(1 + cb)*(1 + cb));

	// 构造实值 D 矩阵 (7x7)，索引: 0->m=-3, 1->-2, ..., 6->+3
	for (int i = 0; i < 7; ++i) {
		for (int j = 0; j < 7; ++j) {
			int m = i - 3;
			int mp = j - 3;

			if (m == 0 && mp == 0) {
				D[i][j] = d[i][j];
			}
			else if (m > 0 && mp > 0) {
				D[i][j] = 0.5 * (d[i][j] * (ca*cg - sa*sg) + d[6 - i][j] * (ca*sg + sa*cg));
			}
			else if (m > 0 && mp < 0) {
				D[i][j] = 0.5 * (d[i][6 - j] * (ca*sg - sa*cg) + d[6 - i][6 - j] * (ca*cg + sa*sg));
			}
			else if (m < 0 && mp > 0) {
				D[i][j] = 0.5 * (d[6 - i][j] * (ca*sg + sa*cg) - d[i][j] * (ca*cg - sa*sg));
			}
			else if (m < 0 && mp < 0) {
				D[i][j] = 0.5 * (d[6 - i][6 - j] * (ca*cg + sa*sg) - d[i][6 - j] * (ca*sg - sa*cg));
			}
			else if (m == 0 && mp > 0) {
				D[i][j] = d[i][j] * (ca*cg - sa*sg);
			}
			else if (m == 0 && mp < 0) {
				D[i][j] = d[i][6 - j] * (ca*sg + sa*cg);
			}
			else if (m > 0 && mp == 0) {
				D[i][j] = d[i][3] * (ca*cg - sa*sg);
			}
			else if (m < 0 && mp == 0) {
				D[i][j] = d[6 - i][3] * (ca*sg + sa*cg);
			}
		}
	}

	return D;
}

// 三阶 SH 系数旋转（被动变换）
std::array<float, 7> MapGIS::Tile::CGGaussianDataTrans::rotate_sh3(const std::array<float, 7>& coeffs,const std::array<float, 9>& R) {
	// Step 1: 提取欧拉角
	std::array<double, 3> euler = rotation_to_euler_zyz(R);
	double alpha = euler[0];
	double beta = euler[1];
	double gamma = euler[2];

	// Step 2: 构造 D^(3)(R)
	auto D = wigner_d_matrix_l3(alpha, beta, gamma);

	// Step 3: 被动变换：c' = D^T @ c
	std::array<float, 7> result = {};
	for (int i = 0; i < 7; ++i) {
		for (int j = 0; j < 7; ++j) {
			result[i] += D[j][i] * coeffs[j];  // D^T[i][j] = D[j][i]
		}
	}

	return result;
}
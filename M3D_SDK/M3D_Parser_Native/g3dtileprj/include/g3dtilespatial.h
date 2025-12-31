#pragma once
#ifndef _G3D_TILE_SPATIAL_H_
#define _G3D_TILE_SPATIAL_H_

#include "g3dtiledefine.h"

namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 三维向量类 (double精度)
		/// 用于表示三维空间中的点、向量或方向
		//================================================================================
		class MAPGISG3DTILEEXPORT Vector3D
		{
		public:
			double x, y, z;  // 三维坐标分量

		public:
			// 默认构造函数，初始化为(0,0,0)
			Vector3D();

			// 构造函数，使用指定的x,y,z值初始化
			Vector3D(const double fX, const double fY, const double fZ);

			// 构造函数，从double数组初始化
			explicit Vector3D(const double afCoordinate[3]);

			// 构造函数，从int数组初始化
			explicit Vector3D(const int afCoordinate[3]);

			// 构造函数，从指针初始化
			explicit Vector3D(double* fX, double* fY, double* fZ);
			
			// 构造函数，使用标量初始化所有分量
			explicit Vector3D(const double scaler);

			// 下标访问运算符，只读版本
			double operator [] (const size_t i) const;
			
			// 下标访问运算符，可写版本
			double& operator [] (const size_t i);
			
			// 赋值运算符
			Vector3D& operator = (const Vector3D& rkVector);

			// 标量赋值运算符
			Vector3D& operator = (const double fScaler);

			// 等于比较运算符
			bool operator == (const Vector3D& rkVector) const;

			// 不等于比较运算符
			bool operator != (const Vector3D& rkVector) const;
			
			// 向量相加运算符
			Vector3D operator + (const Vector3D& rkVector) const;
			
			// 向量相减运算符
			Vector3D operator - (const Vector3D& rkVector) const;
			
			// 标量乘法运算符
			Vector3D operator * (const double fScalar) const;

			// 分量相乘运算符（逐分量乘法）
			Vector3D operator * (const Vector3D& rhs) const;

			// 标量除法运算符
			Vector3D operator / (const double fScalar) const;

			// 分量相除运算符（逐分量除法）
			Vector3D operator / (const Vector3D& rhs) const;

			// 正号运算符（返回自身）
			const Vector3D& operator + () const;

			// 负号运算符（返回反向向量）
			Vector3D operator - () const;

			// 向量加法赋值运算符
			Vector3D& operator += (const Vector3D& rkVector);

			// 标量加法赋值运算符
			Vector3D& operator += (const double fScalar);

			// 向量减法赋值运算符
			Vector3D& operator -= (const Vector3D& rkVector);

			// 标量减法赋值运算符
			Vector3D& operator -= (const double fScalar);

			// 标量乘法赋值运算符
			Vector3D& operator *= (const double fScalar);

			// 分量乘法赋值运算符
			Vector3D& operator *= (const Vector3D& rkVector);

			// 标量除法赋值运算符
			Vector3D& operator /= (const double fScalar);

			// 分量除法赋值运算符
			Vector3D& operator /= (const Vector3D& rkVector);

			// 小于比较运算符（字典序比较）
			bool operator < (const Vector3D& rhs) const;

			// 大于比较运算符（字典序比较）
			bool operator > (const Vector3D& rhs) const;

			// 交换两个向量的内容
			void Swap(Vector3D& other);
			
			// 计算向量点积（数量积）
			double DotProduct(const Vector3D& vec) const;

			// 计算向量叉积（向量积）
			Vector3D CrossProduct(const Vector3D& rkVector) const;

			// 将向量单位化（归一化），返回原向量长度
			double Normalise();

			// 返回单位化后的向量副本，不改变原向量
			Vector3D NormalisedCopy(void) const;

			// 计算向量的模长（大小）
			double Length() const;

			// 计算到另一个点的距离
			double Distance(const Vector3D& rhs) const;
		};
		
		//================================================================================
		/// 四维向量类 (double精度)
		/// 通常用于齐次坐标表示或四元数
		//================================================================================
		class MAPGISG3DTILEEXPORT Vector4D
		{
		public:
			double x, y, z, w;  // 四维坐标分量

		public:
			// 默认构造函数，初始化为(0,0,0,0)
			Vector4D();

			// 构造函数，使用指定的x,y,z,w值初始化
			Vector4D(const double fX, const double fY, const double fZ, const double fW);

			// 构造函数，从double数组初始化
			explicit Vector4D(const double afCoordinate[4]);

			// 构造函数，从int数组初始化
			explicit Vector4D(const int afCoordinate[4]);

			// 构造函数，从指针初始化
			explicit Vector4D(double* fX, double *fY, double *fZ, double *fW);

			// 构造函数，使用标量初始化所有分量
			explicit Vector4D(const double scaler);

			// 构造函数，从三维向量构造（w设为0）
			explicit Vector4D(const Vector3D& rhs);

			// 下标访问运算符，只读版本
			double operator [] (const size_t i) const;

			// 下标访问运算符，可写版本
			double& operator [] (const size_t i);

			// 赋值运算符
			Vector4D& operator = (const Vector4D& rkVector);

			// 标量赋值运算符
			Vector4D& operator = (const double fScalar);

			// 等于比较运算符
			bool operator == (const Vector4D& rkVector) const;

			// 不等于比较运算符
			bool operator != (const Vector4D& rkVector) const;

			// 从三维向量赋值（w分量设为0）
			Vector4D& operator = (const Vector3D& rhs);

			// 向量相加运算符
			Vector4D operator + (const Vector4D& rkVector) const;

			// 向量相减运算符
			Vector4D operator - (const Vector4D& rkVector) const;

			// 标量乘法运算符
			Vector4D operator * (const double fScalar) const;

			// 分量相乘运算符（逐分量乘法）
			Vector4D operator * (const Vector4D& rhs) const;

			// 标量除法运算符
			Vector4D operator / (const double fScalar) const;

			// 分量相除运算符（逐分量除法）
			Vector4D operator / (const Vector4D& rhs) const;

			// 正号运算符（返回自身）
			const Vector4D& operator + () const;

			// 负号运算符（返回反向向量）
			Vector4D operator - () const;

			// 向量加法赋值运算符
			Vector4D& operator += (const Vector4D& rkVector);

			// 向量减法赋值运算符
			Vector4D& operator -= (const Vector4D& rkVector);

			// 标量乘法赋值运算符
			Vector4D& operator *= (const double fScalar);

			// 标量加法赋值运算符
			Vector4D& operator += (const double fScalar);

			// 标量减法赋值运算符
			Vector4D& operator -= (const double fScalar);

			// 分量乘法赋值运算符
			Vector4D& operator *= (const Vector4D& rkVector);

			// 标量除法赋值运算符
			Vector4D& operator /= (const double fScalar);

			// 分量除法赋值运算符
			Vector4D& operator /= (const Vector4D& rkVector);
			
			// 交换两个向量的内容
			void Swap(Vector4D& other);
			
			// 计算向量点积（数量积）
			double DotProduct(const Vector4D& vec) const;

			// 零向量常量
			static const Vector4D ZERO;
		};

		//================================================================================
		/// 4x4矩阵类 (double精度)
		/// 用于三维空间变换，包括平移、旋转、缩放等操作
		//================================================================================
		class MAPGISG3DTILEEXPORT Matrix4D
		{
		protected:
			double m[4][4];  // 4x4矩阵元素，行优先存储

		public:
			// 默认构造函数
			Matrix4D();

			// 构造函数，使用16个元素初始化矩阵
			Matrix4D(
				double m00, double m01, double m02, double m03,
				double m10, double m11, double m12, double m13,
				double m20, double m21, double m22, double m23,
				double m30, double m31, double m32, double m33);

			// 下标访问运算符，返回指定行的指针（可写版本）
			double* operator [] (size_t iRow);

			// 下标访问运算符，返回指定行的指针（只读版本）
			const double *operator [] (size_t iRow) const;

			// 矩阵与标量相乘运算符
			Matrix4D operator*(double scalar) const;

            // 矩阵与矩阵相乘运算符
			Matrix4D operator * (const Matrix4D &m2) const;

			// 矩阵与三维向量相乘运算符（用于坐标变换）
			Vector3D operator * (const Vector3D &v) const;

            // 矩阵与D_3DOT向量相乘运算符
			D_3DOT operator * (const D_3DOT &v) const;

            // 矩阵与四维向量相乘运算符
			Vector4D operator * (const Vector4D& v) const;

			// 矩阵加法运算符
			Matrix4D operator + (const Matrix4D &m2) const;
			
			// 矩阵减法运算符
			Matrix4D operator - (const Matrix4D &m2) const;

			// 矩阵赋值运算符
			Matrix4D& operator =(const Matrix4D& matrix);

			// 矩阵相等比较运算符
			bool operator == (const Matrix4D& m2) const;
			
			// 矩阵不等比较运算符
			bool operator != (const Matrix4D& m2) const;

			// 返回矩阵的转置
			Matrix4D Transpose(void) const;

			// 返回矩阵的逆矩阵
			Matrix4D Inverse(void) const;

			// 判断矩阵是否为单位矩阵
			bool IsUnit() const;

			// 将矩阵设置为单位矩阵
			void SetUnit();

			// 交换两个矩阵的内容
			void Swap(Matrix4D& other);

			// 计算矩阵的行列式
			double  Determinant() const;

			// 零矩阵常量
			static const Matrix4D ZERO;
			// 零仿射矩阵常量
			static const Matrix4D ZEROAFFINE;
			// 单位矩阵常量
			static const Matrix4D IDENTITY;
			// 裁剪空间到图像空间转换矩阵常量
			static const Matrix4D CLIPSPACE2DTOIMAGESPACE;
		};

		//================================================================================
		/// WGS84椭球体工具类
		/// 提供地理坐标与笛卡尔坐标的相互转换功能
		//================================================================================
		class MAPGISG3DTILEEXPORT  WGS84Ellipsoid
		{
		public:
			// 将角度值转换为弧度值
			static double GetRadFromDegree(double degree);

			// 根据经纬度获取椭球体表面某点的法向量
			static Vector3D GetEllipsoidNormalByDegree(double Lon, double Lat);

			// 将地理坐标（经纬度+高程）转换为WGS84椭球体的笛卡尔坐标
			static Vector3D CartesianFromGeographicaDegree(double Lon, double Lat, double height);

			// 将地理坐标（经纬度+高程）转换为WGS84椭球体的笛卡尔坐标（另一种实现）
			static Vector3D CartesianFromGeographicaDegree2(double Lon, double Lat, double height);

			// 将WGS84椭球体的笛卡尔坐标转换为地理坐标（角度值表示）
			static Vector3D  GeographicaDegreeFromCartesian(Vector3D dot);

			// 将WGS84椭球体的笛卡尔坐标转换为地理坐标（弧度值表示）
			static Vector3D  GeographicaRadianFromCartesian(Vector3D dot);

            // 将WGS84椭球体的笛卡尔坐标转换为地理坐标（弧度值表示，D_3DOT版本）
			static D_3DOT  GeographicaRadianFromCartesian(D_3DOT dot);

			// 获取指定经纬度和高程位置的坐标变换矩阵
			static Matrix4D GetLocationPointTransform(double longitudeRadians, double latitudeRadians, double heightMeters);
		};
	}
}
#endif
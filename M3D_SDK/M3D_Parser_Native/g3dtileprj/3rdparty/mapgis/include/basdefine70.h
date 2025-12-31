//检查 2007.8.4.new

// basDefine70.h
//
// 说明信息：MAPGIS70基础宏定义和结构定义
//////////////////////////////////////////////////////////////////////
///@file
#ifndef __MAPGIS_BAS_DEFINE70_H__
#define __MAPGIS_BAS_DEFINE70_H__

#define Points_Type			1				//CPoints
#define PolyLine_Type		2				//CCPolyLine
#define Polygon_Type		3				//CPolygon、CPolygonEx
#define MultiPoint_Type		Points_Type		//CPoints
#define AnyLine_Type		4				//CAnyLine
#define MultiLine_Type		5				//CMultiLine
#define Point_Type			6
#define Line_Type			7				//CVarLine
#define AnyPolygon_Type		8				//CAnyPolygon、CAnySurface、CAnyEntity
#define MultiPolygon_Type	9				//CMultiPolygon、CMultiSurface、CMultiEntity
#define PolyGeometry_Type	10				//CPolyGeometry
#define MultiGeometry_Type	11				//CMultiGeometry
#define AnnoGeometry_Type	17				//CAnnoGeometry

#define AnySurface_Type			21		// CAnySurface
#define MultiSurface_Type		22		// CMultiSurface
#define AnyEntity_Type			23		// CAnyEntity
#define MultiEntity_Type		24		// CMultiEntity
#define EmptyGeometry_Type      25 //空几何类型

#define  PI		3.14159265358979323846
#define  MIN_FLOAT	(-3.402823E+38)
#define  ZERO_FLOAT (+1.401298E-45)
#define  MAX_FLOAT	(+3.402823E+38)
#define  MIN_DOUBLE		(-1.79769313486232E+307)
#define  MAX_DOUBLE		(+1.79769313486232E+307)
#define  ZERO_MIN_DBL   (-4.94065645841247E-324)
#define  ZERO_MAX_DBL   (+4.94065645841247E-324)
#define  MIN_VALUE		(+1.000000E-20)		    //绝对值小于此值即认为=0
#define  MIN_INT	-32768
#define  MAX_INT	+32767
#define  MIN_LONG	-2147483648
#define  MAX_LONG   +2147483647

typedef struct
{
	double x;
	double y;
}D_DOT;

typedef struct
{
	double x;
	double y;
	double z;
}D_3DOT;


typedef struct
{
	double xmin;
	double ymin;
	double xmax;
	double ymax;
}D_RECT;


typedef struct
{
	double xmin;
	double ymin;
	double zmin;
	double xmax;
	double ymax;
	double zmax;
}D_3RECT;

#endif //__MAPGIS_BAS_DEFINE70_H__


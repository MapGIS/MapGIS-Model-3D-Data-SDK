
#ifndef __MAPGIS_BAS_TYPES_H__
#define __MAPGIS_BAS_TYPES_H__
#include <stdint.h>

//{{定义基本C语言类型

//long类型在windows下不管32位64位都是4字节，而在linux会随着32位或64位改变。
//所以之前将现有MapGIS代码中所有的long都统一替换为了gisLONG，这样产生了一
//个问题即原有代码中用到以long*为参数的微软函数时，在64位下传入的参数变为
//了gisLONG（即int),这样就会产生编译错误。由于Linux下不管32\64，long都被定义
//成了int，所以现只以windows或linux来重新定义不同的gisLONG，使得在windows下
//不管32位还是64位，LONG都代表long，这样在windows64下就省了修改代码的工作。
//代码中用到以LONG*为参数的微软API函数的
#if (defined MAPGIS_LINUX || defined MAPGIS_ANDROID || defined MAPGIS_IOS)
typedef int					gisLONG;		//gisLONG定位为32位长整型
typedef unsigned int		gisULONG;
typedef long long			gisINT64;
typedef unsigned long long	gisUINT64;

#else
typedef long				gisLONG;
typedef unsigned long	    gisULONG;
typedef int64_t				gisINT64;
typedef uint64_t 			gisUINT64;
#endif

typedef char				gisCHAR;
typedef short				gisSHORT;
typedef int					gisINT;
typedef unsigned char		gisUCHAR;
typedef unsigned short		gisUSHORT;
typedef unsigned int		gisUINT;		//gisUINT定义为32位无符号整型
typedef float				gisFLOAT;
typedef double				gisDOUBLE;

#if (defined _M_X64 || defined MAPGIS_X64 || defined _WIN64)	
	typedef gisINT64        gisVOIDPTR;		//2014.05.12--改动
#else
	typedef long            gisVOIDPTR;
#endif
//}}

#endif //__MAPGIS_BAS_TYPES_H__

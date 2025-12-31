#pragma once

#include "basdefine70.h"
#include "bastypes.h"
#include <string>


struct AnySurface
{
	gisLONG		pntNum;
	gisLONG		pntBufNum;
	D_3DOT	   *pntBuf;
	gisLONG		triangleNum;
	gisLONG		triangleBufNum;
	gisULONG	   *triangleBuf;

	gisLONG		texturelayerNum;
	gisLONG		tpBufNum;
	D_DOT	   *tpBuf;

	gisLONG		colorBufNum;
	gisULONG	   *colorBuf;

	gisLONG		topoBufNum;
	gisULONG	   *topoBuf;

	gisLONG		normalBufNum;
	D_3DOT	   *normalBuf;

	gisLONG    lTextureIndex;

	short		hasFlag;
};


class  CGeomBase
{
public:
	CGeomBase() {};
	virtual ~CGeomBase() {};

	virtual short type() = 0;		//成功返回type>=1;失败返回0
	virtual gisLONG  length() = 0;
	virtual gisLONG  Save(char *ptBuf, gisLONG bufLen) = 0;    //返回数据长度
	virtual gisLONG  Load(char *ptDat, gisLONG datLen) = 0;	   //返回数据长度
	virtual gisLONG  Empty() = 0;		//清空对象
														
};

class CVarLine;

//1. 几何实体：点（多点）的封装类
//该类支持3维空间坐标
class CPoints :public CGeomBase
{
public:
	CPoints();
	CPoints(const CPoints &pnts);
	virtual ~CPoints();

	D_3DOT  *New(gisLONG bufLen);				//重新分配D_3DOT内存缓冲区,将dotNum设置为bufLen
	gisLONG     Set(D_3DOT *ptxyz, gisLONG dotNum);
	gisLONG     Append(D_3DOT &dot);			//返回插入的序号i>=0，失败返回-1
	gisLONG     Del(gisLONG i);					//删除第i号坐标点,成功返回1，失败返回0
	D_3DOT  &operator[](gisLONG i);			//取第i号坐标点对象指针（i>=0且<GetDotNum返回的个数）

	gisLONG     GetNum();						 //取几何实体的个数
	D_3DOT  *GetBufPtr();

	gisLONG   CalRect(D_RECT *rc);
	gisLONG   IsInRect(D_RECT *rc, D_RECT *rrc = NULL);		//是否在矩形范围内
	gisLONG   IsInterRect(D_RECT *rc, D_RECT *rrc = NULL);	//是否和矩形相交

	gisLONG operator=(CPoints  &pnts);
	gisLONG operator=(CVarLine &lin);  //最终转化为3维数据

	virtual short	type();
	virtual gisLONG	length();
	virtual gisLONG	Save(char *ptBuf, gisLONG bufLen);
	virtual gisLONG	Load(char *ptDat, gisLONG datLen);

	virtual gisLONG	Empty();		//清空对象
																						  

private:
	gisLONG     m_dotNum;		//点数
	gisLONG     m_bufLen;      //缓冲区长度(D_3DOT个数)
	D_3DOT	*m_ptXYZ;       //点缓冲区
	gisLONG     m_res;         //。		new zhou 2005.8.2
};

//2、CVarLine仅限于封装2维、3维的折线，及其相关操作。
class CVarLine :public CGeomBase
{
public:
	CVarLine();
	CVarLine(const CVarLine& varLin);
	virtual ~CVarLine();

	void  *New(gisLONG dotNum, char dim = 2, char hasM = 0);      //重新分配内存缓冲区,将dotNum设置为bufLen
	gisLONG   Set(void *ptXY, gisLONG dotNum, char dim = 2, char hasM = 0);
	gisLONG   Set(void *ptXY, double *ptMVal, gisLONG dotNum, char dim = 2);
	gisLONG   Append(void *ptXY, gisLONG dotNum, char dim = 2, char hasM = 0); //返回插入的序号i>=0，失败返回-1
	gisLONG   Append(void *ptXY, double *ptMVal, gisLONG dotNum, char dim = 2);

	gisLONG	Update(gisLONG i, void *dot, char dim);
	gisLONG	UpdateM(gisLONG i, double mVal);

	gisLONG	Del(gisLONG i);	//删除第i号坐标点,成功返回1，失败返回0

	gisLONG	Get(gisLONG i, void *dot, char dim);
	double	GetX(gisLONG i);
	double	GetY(gisLONG i);
	double	GetZ(gisLONG i);
	double	GetM(gisLONG i);

	gisLONG	CalMinMaxMeasureVal(double &minMVal, double &maxMVal);
	gisLONG	CalRect(D_RECT *rc);
	gisLONG	IsInRect(D_RECT *rc, D_RECT *rrc = NULL);		//是否在矩形范围内
	gisLONG	IsInterRect(D_RECT *rc, D_RECT *rrc = NULL);	//是否和矩形相交

	//=====================================ptXY()=====================================
	/// @brief 获取折线的坐标点集合。
	///
	/// 该函数可能返回的是二维或三维的点坐标，具体要根据dim()返回的维数。
	///
	/// @return 返回坐标集合(void*)。
	///
	/// @code
	///     char    dim   = 2;维度
	///     D_DOT*  ptXY  = NULL;
	///     D_3DOT* ptXYZ = NULL;
	///     //在获取坐标集合前应该先取维度，根据维度在获取坐标集合
	///     dim =  Lin.dim();
	///     if( 2 == dim )
	///     {
	///			ptXY = (D_DOT *)(Lin.ptXY());
	///  	}
	///     else if( 3 == dim )
	///     {
	///		    ptXYZ = (D_3DOT *)(Lin.ptXY());
	///     }
	/// @endcode
	//================================================================================
	void*	ptXY();
	char	isMeasure();
	double*	ptMVal();
	gisLONG	dotNum();
	char	dim();

	gisLONG operator=(CVarLine		&lin);
	gisLONG operator=(CPoints		&pnts);   //pnts的类型必须是MultiPoint_Type

	virtual short	type();
		virtual gisLONG    length();
	virtual gisLONG 	Save(char *ptBuf, gisLONG bufLen);
	virtual gisLONG 	Load(char *ptDat, gisLONG datLen);


	virtual gisLONG	Empty();		//清空对象

private:
	void	*m_ptXY;
	char	 m_isMeasure;
	double	*m_ptMVal;
	gisLONG	 m_dotNum;
	gisLONG     m_bufLen;
	char	 m_dim;			//2，3，4，5
	gisLONG     m_bufByteLen;	//缓冲区字节长度
	gisLONG     m_bufLenM;//M缓冲区可以放m的个数
};


class CGeomBase3D :public CGeomBase
{
public:
	virtual short type3D() = 0;
	virtual gisLONG  CalRect3D(D_3RECT *rc3d) = 0;
};


//三维面必须有三角形，否则只有边界要用CAnyPolygon
class CAnySurface :public CGeomBase3D
{
public:
	CAnySurface();
	CAnySurface(const CAnySurface& anySurface);
	virtual ~CAnySurface();

	gisLONG New(gisLONG pntNum, gisLONG triangleNum, gisLONG texturelayerNum);
	//输入点集、三角形、拓扑、顶点颜色、顶点法向量、纹理坐标自动建立符合规范的三维面
	gisLONG Set(gisLONG pntNum, D_3DOT *dots,
		gisLONG triangleNum, gisULONG *triangles, gisULONG *topo = NULL,
		gisULONG *colors = NULL, D_3DOT *normals = NULL, gisLONG texturelayerNum = 0, D_DOT *texturePos = NULL);

	gisLONG		GetPointNum();
	D_3DOT*		GetPoints();
	gisULONG*		GetColor();
	D_3DOT*		GetNormalVector();
	gisLONG		GetTextureLayerNum();
	D_DOT*		GetTexturePosition(gisLONG texturelayerNum = 0);

	gisLONG		GetTriangleNum();
	gisULONG*		GetTriangles();
	gisULONG*		GetTopo();

	gisLONG		HasColor();
	gisLONG		HasNormalVector();
	gisLONG		HasTexturePosition();
	gisLONG		HasTopo();

	gisLONG		DelColor();
	gisLONG		DelNormalVector();
	gisLONG		DelTexturePosition();
	gisLONG		DelTopo();

	//符号子项索引
	gisLONG     SetTextureIndex(gisLONG lTextureIndex);
	gisLONG     GetTextureIndex();

	gisLONG   operator=(CAnySurface &surface);

	virtual short type3D();
	virtual gisLONG  CalRect3D(D_3RECT *rc3d);

	virtual short type();		//成功返回AnyPolygon_Type;失败返回0
	
	virtual gisLONG  length();
	virtual gisLONG  Save(char *ptBuf, gisLONG bufLen);    //返回数据长度
	virtual gisLONG  Load(char *ptDat, gisLONG datLen);	   //返回数据长度

	virtual gisLONG  Empty();		//清空对象

	virtual gisLONG  CalRect(D_RECT *rc);						//计算矩形范围
	virtual gisLONG  IsInRect(D_RECT *rc, D_RECT *rrc = NULL);		//是否在矩形范围内
	virtual gisLONG  IsInterRect(D_RECT *rc, D_RECT *rrc = NULL);	//是否和矩形相交

	double CalSurfArea();//计算表面积
private:
	AnySurface		*m_ptAnySurface;
};

struct SurfaceSet
{
	typedef CAnySurface ItemType;
	gisLONG			 Num;
	gisLONG			 BufNum;
	CAnySurface	   **Item;
};

class CAnyEntity :public CGeomBase3D
{
public:
	CAnyEntity();
	CAnyEntity(const CAnyEntity& anyEntity);
	virtual ~CAnyEntity();

	gisLONG			New(gisLONG surfaceNum);
	gisLONG			GetSurfaceNum();
	CAnySurface*	GetSurface(gisLONG idx);
	CAnySurface*	AppendSurface();
	gisLONG			DelSurface(gisLONG idx);

	gisLONG   operator=(CAnyEntity &surface);
	double CalSurfArea();//计算表面积
	double CalVolume();//计算体积
	virtual short type3D();
	virtual gisLONG  CalRect3D(D_3RECT *rc3d);

	virtual short type();		//成功返回AnyPolygon_Type;失败返回0
	
	virtual gisLONG  length();
	virtual gisLONG  Save(char *ptBuf, gisLONG bufLen);    //返回数据长度
	virtual gisLONG  Load(char *ptDat, gisLONG datLen);	   //返回数据长度

	virtual gisLONG  Empty();		//清空对象

	virtual gisLONG  CalRect(D_RECT *rc);						//计算矩形范围
	virtual gisLONG  IsInRect(D_RECT *rc, D_RECT *rrc = NULL);		//是否在矩形范围内
	virtual gisLONG  IsInterRect(D_RECT *rc, D_RECT *rrc = NULL);	//是否和矩形相交

	
private:
	SurfaceSet			*m_surfaces;
};
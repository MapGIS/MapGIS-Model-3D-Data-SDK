#include "stdafx.h"
#include "cgfile.h"
#include "g3dtilegaussian.h"
#include "ci_gaussian_ply_read_write.h"
#include "ci_gaussian_splat_read_write.h"
#include "ci_gaussian_spz_read_write.h"
#include "ci_gaussian_ksplat_read_write.h"
#include "ci_gaussian_sog_read_write.h"

MapGIS::Tile::CGGaussianReadWrite * GetGaussianReadWrite(CGString filePath)
{
	if (filePath.GetLength() <= 0)
		return NULL;
	int index = filePath.ReverseFind(".");
	if (index < 0)
		return NULL;
	CGString suffix = filePath.Right(filePath.GetLength() - index - 1);

	MapGIS::Tile::CGGaussianReadWrite *pReadWrite = NULL;
	if (suffix.CompareNoCase("ply") == 0)
	{
		pReadWrite = new Ci_PlyGaussianReadWrite();
	}
	else if (suffix.CompareNoCase("splat") == 0)
	{
		pReadWrite = new Ci_SplatGaussianReadWrite();
	}
	else if (suffix.CompareNoCase("ksplat") == 0)
	{
		pReadWrite = new Ci_KSplatGaussianReadWrite();
	}
	else if (suffix.CompareNoCase("spz") == 0)
	{
		pReadWrite = new Ci_SpzGaussianReadWrite();
	}
	else if (suffix.CompareNoCase("sog") == 0 || suffix.CompareNoCase("json") == 0)
	{
		pReadWrite = new Ci_SogGaussianReadWrite();
	}
	return pReadWrite;
}



gisLONG MapGIS::Tile::CGGaussianReadWrite::Read(CGString filePath,  MapGIS::Tile::GaussianModel & data)
{
	gisLONG rtn = 0;
	MapGIS::Tile::CGGaussianReadWrite * pReadWrite = GetGaussianReadWrite(filePath);
	if (pReadWrite != NULL)
	{
		rtn = pReadWrite->i_Read(filePath, data);
	}
	return rtn;
}
gisLONG MapGIS::Tile::CGGaussianReadWrite::Read(CGString filePath, std::function<void( MapGIS::Tile::GaussianFeature & dot, bool& isStop)>& callback)
{
	gisLONG rtn = 0;
	CGGaussianReadWrite * pReadWrite = GetGaussianReadWrite(filePath);
	if (pReadWrite != NULL)
	{
		rtn = pReadWrite->i_Read(filePath, callback);
	}
	return rtn;
}
gisLONG MapGIS::Tile::CGGaussianReadWrite::Write(CGString filePath, const  MapGIS::Tile::GaussianModel & data)
{
	gisLONG rtn = 0;
	MapGIS::Tile::CGGaussianReadWrite * pReadWrite = GetGaussianReadWrite(filePath);
	if (pReadWrite != NULL)
	{
		rtn = pReadWrite->i_Write(filePath, data);
	}
	return rtn;
}


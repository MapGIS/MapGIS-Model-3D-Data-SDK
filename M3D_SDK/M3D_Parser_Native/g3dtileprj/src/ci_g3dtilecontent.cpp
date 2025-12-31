#include "stdafx.h"
#include "../include/g3dtilecontent.h"

MapGIS::Tile::GeometryContent::GeometryContent()
{
	m_pModel = NULL;
	m_pAttribute = NULL;
	m_pInstances = NULL;

	m_isFreeModel = false;
	m_isFreeAttribute = false;
	m_isFreeInstances = false;
};

MapGIS::Tile::GeometryContent::GeometryContent(GeometryContent&& item)noexcept
{
	this->m_pModel = item.m_pModel;
	this->m_pAttribute = item.m_pAttribute;
	this->m_pInstances = item.m_pInstances;
	this->m_isFreeModel = item.m_isFreeModel;
	this->m_isFreeAttribute = item.m_isFreeAttribute;
	this->m_isFreeInstances = item.m_isFreeInstances;

	item.m_pModel = NULL;
	item.m_pAttribute = NULL;
	item.m_pInstances = NULL;
	item.m_isFreeModel = false;
	item.m_isFreeAttribute = false;
	item.m_isFreeInstances = false;
};

MapGIS::Tile::GeometryContent::~GeometryContent()
{
	i_FreeData();
};

void MapGIS::Tile::GeometryContent::CreateData(bool createModel, GeometryType type, bool createAttribute, bool createInstances)
{
	i_FreeData();
	if(createModel)
	{
		switch (type)
		{
		case MapGIS::Tile::GeometryType::Point:
			m_pModel = new PointsModel();
			break;
		case MapGIS::Tile::GeometryType::Line:
			m_pModel = new LinesModel();
			break;
		case MapGIS::Tile::GeometryType::Surface:
			m_pModel = new SurfacesModel();
			break;
		case MapGIS::Tile::GeometryType::Entity:
			m_pModel = new EntitiesModel();
			break;
		case MapGIS::Tile::GeometryType::None:
		default:
			break;
		}
		m_isFreeModel = true;
	}
	
	if (createAttribute)
	{
		m_pAttribute = new Attribute();
		m_isFreeAttribute = true;
	}
	if (createInstances)
	{
		m_pInstances = new vector<ModelInstance>();
		m_isFreeInstances = true;
	}
}

MapGIS::Tile::GeometryContent& MapGIS::Tile::GeometryContent::operator = (MapGIS::Tile::GeometryContent&& item)noexcept
{
	if (this != &item)
	{
		this->m_pModel = item.m_pModel;
		this->m_pAttribute = item.m_pAttribute;
		this->m_pInstances = item.m_pInstances;
		this->m_isFreeModel = item.m_isFreeModel;
		this->m_isFreeAttribute = item.m_isFreeAttribute;
		this->m_isFreeInstances = item.m_isFreeInstances;

		item.m_pModel = NULL;
		item.m_pAttribute = NULL;
		item.m_pInstances = NULL;
		item.m_isFreeModel = false;
		item.m_isFreeAttribute = false;
		item.m_isFreeInstances = false;
	}
	return *this;
}

MapGIS::Tile::G3DModel* MapGIS::Tile::GeometryContent::Get3DModel()
{
	return m_pModel;
}

void MapGIS::Tile::GeometryContent::Set3DModel(MapGIS::Tile::G3DModel* pModel)
{
	if (m_isFreeModel && m_pModel != NULL)
		delete m_pModel;

	if (dynamic_cast<VoxelModel*>(pModel) != NULL)
	{
		m_pModel = NULL;
	}
	else
		m_pModel = pModel;
	m_isFreeModel = false;
}


MapGIS::Tile::Attribute* MapGIS::Tile::GeometryContent::GetAttribute()
{
	return m_pAttribute;
}

void MapGIS::Tile::GeometryContent::SetAttribute(MapGIS::Tile::Attribute* pAttribute)
{
	if (m_isFreeAttribute && m_pAttribute != NULL)
		delete m_pAttribute;
	m_pAttribute = pAttribute;
	m_isFreeAttribute = false;
}

vector<MapGIS::Tile::ModelInstance>* MapGIS::Tile::GeometryContent::GetModelInstance()
{
	return m_pInstances;
}

void MapGIS::Tile::GeometryContent::SetModelInstance(vector<MapGIS::Tile::ModelInstance>* pInstances)
{
	if (m_isFreeInstances && m_pInstances != NULL)
		delete m_pInstances;
	m_pInstances = pInstances;
	m_isFreeInstances = false;
}

MapGIS::Tile::GeometryType MapGIS::Tile::GeometryContent::GetGeometryType()  const
{
	if (m_pModel != NULL)
		return m_pModel->GetGeometryType();
	return GeometryType::None;
}

void MapGIS::Tile::GeometryContent::i_FreeData()
{
	if (m_isFreeModel && m_pModel != NULL)
		delete m_pModel;

	if (m_isFreeAttribute && m_pAttribute != NULL)
		delete m_pAttribute;

	if (m_isFreeInstances && m_pInstances != NULL)
		delete m_pInstances;
	m_pModel = NULL;
	m_pAttribute = NULL;
	m_pInstances = NULL;
	m_isFreeModel = false;
	m_isFreeAttribute = false;
	m_isFreeInstances = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
MapGIS::Tile::VoxelContent::VoxelContent()
{
	m_pModel = NULL;
	m_pAttribute = NULL;

	m_isFreeModel = false;
	m_isFreeAttribute = false;
};

MapGIS::Tile::VoxelContent::VoxelContent(MapGIS::Tile::VoxelContent&& item) noexcept
{
	this->m_pModel = item.m_pModel;
	this->m_pAttribute = item.m_pAttribute;
	this->m_isFreeModel = item.m_isFreeModel;
	this->m_isFreeAttribute = item.m_isFreeAttribute;

	item.m_pModel = NULL;
	item.m_pAttribute = NULL;
	item.m_isFreeModel = false;
	item.m_isFreeAttribute = false;
};

MapGIS::Tile::VoxelContent::~VoxelContent()
{
	i_FreeData();
};

void MapGIS::Tile::VoxelContent::CreateData(bool  createGeometry, bool createAttribute)
{
	i_FreeData();

	if (createGeometry)
	{
		m_pModel = new VoxelModel();
		m_isFreeModel = true;
	}
	if (createAttribute)
	{
		m_pAttribute = new vector<Attribute>();
		m_isFreeAttribute = true;
	}
}

MapGIS::Tile::VoxelContent& MapGIS::Tile::VoxelContent::operator = (MapGIS::Tile::VoxelContent&& item)noexcept
{
	if (this != &item)
	{
		this->m_pModel = item.m_pModel;
		this->m_pAttribute = item.m_pAttribute;
		this->m_isFreeModel = item.m_isFreeModel;
		this->m_isFreeAttribute = item.m_isFreeAttribute;

		item.m_pModel = NULL;
		item.m_pAttribute = NULL;
		item.m_isFreeModel = false;
		item.m_isFreeAttribute = false;
	}
	return *this;
}

MapGIS::Tile::VoxelModel* MapGIS::Tile::VoxelContent::Get3DModel()
{
	return m_pModel;
}

void MapGIS::Tile::VoxelContent::Set3DModel(MapGIS::Tile::VoxelModel* pModel)
{
	if (m_isFreeModel && m_pModel != NULL)
		delete m_pModel;
	m_pModel = pModel;
	m_isFreeModel = false;
}

vector<MapGIS::Tile::Attribute>* MapGIS::Tile::VoxelContent::GetAttribute()
{
	return m_pAttribute;
}

void MapGIS::Tile::VoxelContent::SetAttribute(vector<MapGIS::Tile::Attribute>* pAttribute)
{
	if (m_isFreeAttribute && m_pAttribute != NULL)
		delete m_pAttribute;
	m_pAttribute = pAttribute;
	m_isFreeAttribute = false;
}

void MapGIS::Tile::VoxelContent::i_FreeData()
{
	if (m_isFreeModel && m_pModel != NULL)
		delete m_pModel;
	if (m_isFreeAttribute && m_pAttribute != NULL)
		delete m_pAttribute;
	m_pModel = NULL;
	m_pAttribute = NULL;
	m_isFreeModel = false;
	m_isFreeAttribute = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
MapGIS::Tile::GaussianContent::GaussianContent()
{
	m_pModel = NULL;

	m_isFreeModel = false;
}

MapGIS::Tile::GaussianContent::GaussianContent(GaussianContent&& item)noexcept
{
	this->m_pModel = item.m_pModel;
	this->m_isFreeModel = item.m_isFreeModel;

	item.m_pModel = NULL;
	item.m_isFreeModel = false;
}

MapGIS::Tile::GaussianContent::~GaussianContent()
{
	i_FreeData();
}


void MapGIS::Tile::GaussianContent::CreateData(bool  createGeometry)
{
	i_FreeData();

	if (createGeometry)
	{
		m_pModel = new GaussianModel();
		m_isFreeModel = true;
	}
}

MapGIS::Tile::GaussianContent& MapGIS::Tile::GaussianContent::operator = (MapGIS::Tile::GaussianContent&& item)noexcept
{
	i_FreeData();
	if (this != &item)
	{
		this->m_pModel = item.m_pModel;
		this->m_isFreeModel = item.m_isFreeModel;

		item.m_pModel = NULL;
		item.m_isFreeModel = false;
	}
	return *this;
}

MapGIS::Tile::GaussianModel* MapGIS::Tile::GaussianContent::Get3DModel()
{
	return m_pModel;
}

void MapGIS::Tile::GaussianContent::Set3DModel(MapGIS::Tile::GaussianModel* pModel)
{
	if (m_isFreeModel && m_pModel != NULL)
		delete m_pModel;
	m_pModel = pModel;
	m_isFreeModel = false;
}

void MapGIS::Tile::GaussianContent::i_FreeData()
{
	if (m_isFreeModel && m_pModel != NULL)
		delete m_pModel;
	m_pModel = NULL;
	m_isFreeModel = false;
}
#include "stdafx.h"
#include "ci_structure_tree.h"
#include "ci_assist.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/rapidjson.h"

using namespace MapGIS::Tile;

CGString Ci_StructureTreeManager::SaveStructureTreeLeaf(rapidjson::Document& doc, rapidjson::Value& itemArray, MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath, string name, bool isSaveSubfolder)
{
	if (name.length() <= 0)
		return "";
	CGString path = relativePath;
	if (isSaveSubfolder)
		path = MakePathByRelativePath(relativePath, "./structuretree/");

	CGString newName = name;
	int num = 0;
	while (pCacheStorage->IsExistContent(MakePathByRelativePath(path, newName + ".json")))
	{
		newName = MakePathByRelativePath(path, name + "_" + std::to_string(num)  + ".json");
		num++;
	}
	if (itemArray.Size() > m_nItemSizeFile)
	{
		rapidjson::Document newDoc;
		newDoc.SetObject();
		rapidjson::Value newArray(rapidjson::kArrayType);
		num = 1;
		CGString currentName = newName;
		CGString nextName = newName + "page" + std::to_string(num);
		while (pCacheStorage->IsExistContent(MakePathByRelativePath(path, nextName + ".json")))
		{
			num++;
			nextName = newName + "page" + std::to_string(num);
		}
		for (int i = 0; i < itemArray.Size(); i++)
		{
			rapidjson::Value item(itemArray[i].GetType());
			item.CopyFrom(itemArray[i], doc.GetAllocator());
			newArray.PushBack(item, newDoc.GetAllocator());
			if (newArray.Size() == m_nItemSizeFile)
			{
				newDoc.AddMember("items", newArray, newDoc.GetAllocator());
				if (i != itemArray.Size() - 1)
				{
					CGString name = (nextName + ".json").Converted(CGString::EncodeType::UTF8);
					newDoc.AddMember("nextItemsUri", ToStringValue(name, newDoc.GetAllocator()), newDoc.GetAllocator());
				}
				rapidjson::StringBuffer buffer;
				rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
				doc.Accept(writer);
				pCacheStorage->SetContent(MakePathByRelativePath(path, currentName + ".json"), buffer.GetString(), buffer.GetLength());

				newDoc = rapidjson::Document();
				newDoc.SetObject();
				newArray = rapidjson::Value(rapidjson::kArrayType);
				currentName = nextName;
				num++;
				nextName = newName + "page" + std::to_string(num);
				while (pCacheStorage->IsExistContent(MakePathByRelativePath(path, nextName + ".json")))
				{
					num++;
					nextName = newName + "page" + std::to_string(num);
				}
			}
		}
		if (newArray.Size() > 0)
		{
			newDoc.AddMember("items", newArray, newDoc.GetAllocator());
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			newDoc.Accept(writer);
			pCacheStorage->SetContent(MakePathByRelativePath(path, currentName + ".json"), buffer.GetString(), buffer.GetLength());
		}
	}
	else
	{
		doc.AddMember("items", itemArray, doc.GetAllocator());
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		pCacheStorage->SetContent(MakePathByRelativePath(path, newName + ".json"), buffer.GetString(), buffer.GetLength());
	}
	if (isSaveSubfolder)
		return "structuretree/" + newName + ".json";
	else
		return newName + ".json";
}

void CalcStructureTreeNum(const StructureTree* pNode, int currentIndex,  vector<int>& nums)
{
	if (pNode == NULL)
		return;
	while (nums.size() <= currentIndex)
		nums.emplace_back(0);
	nums[currentIndex]++;
	if (pNode->GetType() == StructureTreeType::Node)
	{
		const StructureTreeNode* pCurrentNode = dynamic_cast<const StructureTreeNode*>(pNode);
		if (pCurrentNode != NULL)
		{
			gisLONG childNum = pCurrentNode->GetChildNum();

			for (int i = 0; i < childNum; i++)
			{
				const StructureTree* item = pCurrentNode->GetChild(i);
				CalcStructureTreeNum(item, currentIndex + 1, nums);
			}
		}
	}
}

gisLONG Ci_StructureTreeManager::StructureTreeLeafToJsonObject(const StructureTreeLeaf& leaf, rapidjson::Value& nodeObj, rapidjson::Value::AllocatorType& allocator)
{
	nodeObj.AddMember("level", leaf.level, allocator);
	CGString name = leaf.name.Converted(CGString::EncodeType::UTF8);
	nodeObj.AddMember("name", ToStringValue(name, allocator), allocator);
	rapidjson::Value propertyObj(rapidjson::kObjectType);

	propertyObj.AddMember("tid", static_cast<int64_t>(leaf.id), allocator);
	propertyObj.AddMember("OID", static_cast<int64_t>(leaf.oid), allocator);
	propertyObj.AddMember("layerID", static_cast<int64_t>(leaf.layerID), allocator);
	if (leaf.box.size() == 6)
	{
		rapidjson::Value boxArray(rapidjson::kArrayType);
		for (vector<double>::const_iterator itr = leaf.box.begin(); itr != leaf.box.end(); itr++)
		{
			boxArray.PushBack((double)*itr, allocator);
		}
		propertyObj.AddMember("box", boxArray, allocator);
	}

	nodeObj.AddMember("property", propertyObj, allocator);
	return 1;
}

CGString Ci_StructureTreeManager::StructureTreeNodechildToJsonFile(MapGIS::Tile::G3DCacheStorage* pCacheStorage, const StructureTreeNode& node, string treeName, CGString relativePath, bool isRootFile)
{
	rapidjson::Document doc;
	doc.SetObject();
	rapidjson::Value itemArray(rapidjson::kArrayType);
	gisLONG childNum = node.GetChildNum();

	vector<int> nums;
	for (int i = 0; i < childNum; i++)
	{
		const StructureTree* item = node.GetChild(i);
		CalcStructureTreeNum(item, 0, nums);
	}

	int childMaxLevel = 0;
	int childCurrentLevel = 0;
	if (nums.size() > 0)
	{
		int sumNum = 0;
		for (int i = 0; i < nums.size(); i++)
		{
			sumNum += nums[i];
			if (sumNum > m_nItemSizeFile)
			{
				childMaxLevel = i - 1;
				break;
			}
		}
		if (sumNum < m_nItemSizeFile)
			childMaxLevel = nums.size() - 1;
	}

	int num = 0;

	for (int i = 0; i < childNum; i++)
	{
		const StructureTree* item = node.GetChild(i);
		if (item->GetType() == StructureTreeType::Leaf)
		{
			const StructureTreeLeaf* pChildLeaf = dynamic_cast<const StructureTreeLeaf*>(item);
			if (pChildLeaf != NULL)
			{
				rapidjson::Value childObj(rapidjson::kObjectType);
				StructureTreeLeafToJsonObject(*pChildLeaf, childObj, doc.GetAllocator());
				itemArray.PushBack(childObj, doc.GetAllocator());
			}
		}
		else
		{
			const StructureTreeNode* pChildNode = dynamic_cast<const StructureTreeNode*>(item);
			if (pChildNode != NULL)
			{
				rapidjson::Value childObj(rapidjson::kObjectType);
				if (isRootFile)
				{
					CGString path = MakePathByRelativePath(relativePath, "/structuretree/0.json");
					StructureTreeNodeToJsonObject(pCacheStorage, *pChildNode, childObj, doc.GetAllocator(), treeName + "_" + std::to_string(num), childMaxLevel, childCurrentLevel, path, false);
				}
				else
					StructureTreeNodeToJsonObject(pCacheStorage, *pChildNode, childObj, doc.GetAllocator(), treeName + "_" + std::to_string(num), childMaxLevel, childCurrentLevel, relativePath, false);
				itemArray.PushBack(childObj, doc.GetAllocator());
			}
		}
		num++;
	}
	return SaveStructureTreeLeaf(doc,itemArray, pCacheStorage, relativePath, treeName, isRootFile);
}

gisLONG  Ci_StructureTreeManager::StructureTreeNodeToJsonObject(MapGIS::Tile::G3DCacheStorage* pCacheStorage, const StructureTreeNode& node, rapidjson::Value& treeObj, rapidjson::Value::AllocatorType& allocator, string treeName, int maxLevel, int currentLevel, CGString relativePath, bool isRootFile)
{
	gisLONG childNum = node.GetChildNum();
	treeObj.AddMember("childSize", childNum, allocator);
	treeObj.AddMember("level", node.level, allocator);
	CGString name = node.name.Converted(CGString::EncodeType::UTF8);
	treeObj.AddMember("name", ToStringValue(name, allocator), allocator);
	rapidjson::Value propertyObj(rapidjson::kObjectType);
	propertyObj.AddMember("minTid", static_cast<int64_t>(node.minID), allocator);
	propertyObj.AddMember("maxTid", static_cast<int64_t>(node.maxID), allocator);

	if (node.box.size() == 6)
	{
		rapidjson::Value boxArray(rapidjson::kArrayType);
		for (vector<double>::const_iterator itr = node.box.begin(); itr != node.box.end(); itr++)
		{
			boxArray.PushBack((double)*itr, allocator);
		}
		propertyObj.AddMember("box", boxArray, allocator);
	}
	treeObj.AddMember("property", propertyObj, allocator);

	if (maxLevel > currentLevel)
	{
		rapidjson::Value childrenObj(rapidjson::kObjectType);
		rapidjson::Value  itemArray(rapidjson::kArrayType);
		int num = 0;

		for (int i = 0; i < childNum; i++)
		{
			const StructureTree* item = node.GetChild(i);

			if (item->GetType() == StructureTreeType::Leaf)
			{
				const StructureTreeLeaf* pChildLeaf = dynamic_cast<const StructureTreeLeaf*>(item);
				if (pChildLeaf != NULL)
				{
					rapidjson::Value  childObj(rapidjson::kObjectType);
					StructureTreeLeafToJsonObject(*pChildLeaf, childObj, allocator);
					itemArray.PushBack(childObj, allocator);
				}
			}
			else
			{
				const StructureTreeNode* pChildNode = dynamic_cast<const StructureTreeNode*>(item);
				if (pChildNode != NULL)
				{
					rapidjson::Value childObj(rapidjson::kObjectType);
					StructureTreeNodeToJsonObject(pCacheStorage, *pChildNode, childObj, allocator, treeName + "_" + std::to_string(num), maxLevel, currentLevel + 1, relativePath, isRootFile);
					itemArray.PushBack(childObj, allocator);
				}
			}
			num++;
		}
		childrenObj.AddMember("items", itemArray, allocator);
		treeObj.AddMember("children", childrenObj, allocator);
	}
	else
	{
		CGString uri = StructureTreeNodechildToJsonFile(pCacheStorage, node, treeName, relativePath, isRootFile);
		uri.Convert(CGString::EncodeType::UTF8);
		treeObj.AddMember("childrenUri", ToStringValue(uri, allocator), allocator);
	}
	return 1;
}

gisLONG Ci_StructureTreeManager::i_Write(MapGIS::Tile::G3DCacheStorage* pCacheStorage, CGString relativePath)
{
	if (m_pRootNode == NULL ||  pCacheStorage == NULL || relativePath.GetLength() <= 0)
		return 0;
	rapidjson::Document doc;
	doc.SetObject();
	vector<int> nums;
	CalcStructureTreeNum(m_pRootNode, 0, nums);
	int maxLevel = 0;
	int currentLevel = 0;
	if (nums.size() > 0)
	{
		int sumNum = 0;
		for (int i = 0; i < nums.size(); i++)
		{
			sumNum += nums[i];
			if (sumNum > m_nItemSizeFile)
			{
				maxLevel = i - 1;
				break;
			}
		}
		if (sumNum < m_nItemSizeFile)
			maxLevel = nums.size() - 1;
	}
	string name = "0";
	StructureTreeNodeToJsonObject(pCacheStorage,*m_pRootNode, doc, doc.GetAllocator(), name, maxLevel, currentLevel, relativePath, true);

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);
	pCacheStorage->SetContent(relativePath, buffer.GetString(), buffer.GetLength());
	return 1;
}

gisLONG Ci_StructureTreeManager::Write(StructureTreeNode& rootNode, MapGIS::Tile::G3DCacheStorage*	pCacheStorage, CGString relativePath)
{
	Ci_StructureTreeManager treeManager(rootNode);
	return treeManager.i_Write(pCacheStorage, relativePath);
}

gisLONG  ii_Read(MapGIS::Tile::G3DCacheStorage*	pCacheStorage, CGString relativePath, StructureTreeNode *	pParentNode, StructureTreeNode *	pCurrentNode, rapidjson::Value& treeObj)
{
	if (!treeObj.IsObject())
		return 0;

	if (treeObj.HasMember("childSize"))
	{//表示根node节点
		if (pCurrentNode != NULL)
		{
			if (treeObj.HasMember("level") && treeObj["level"].IsNumber())
			{
				pCurrentNode->level = JsonValueToInt(treeObj["level"]);
			}

			if (treeObj.HasMember("name") && treeObj["name"].IsString())
			{
				pCurrentNode->name = CGString(treeObj["name"].GetString(),CGString::EncodeType::UTF8);
			}
			if (treeObj.HasMember("property") && treeObj["property"].IsObject())
			{
				rapidjson::Value& propertyValue = treeObj["property"];
				if (propertyValue.HasMember("maxTid") && propertyValue["maxTid"].IsNumber())
				{
					pCurrentNode->maxID = JsonValueToInt64(propertyValue["maxTid"]);
				}
				if (propertyValue.HasMember("minTid") && propertyValue["minTid"].IsNumber())
				{
					pCurrentNode->minID = JsonValueToInt64(propertyValue["minTid"]);
				}
				if (propertyValue.HasMember("box") && propertyValue["box"].IsArray())
				{
					if (propertyValue["box"].Size() == 6)
					{
						for (int i = 0;i < 6;i++)
						{
							if (propertyValue["box"][i].IsNumber())
							{
								pCurrentNode->box.push_back(propertyValue["box"][i].GetDouble());
							}
						}
					}
				}
			}

			if (treeObj.HasMember("children") && treeObj["children"].IsObject())
			{
				rapidjson::Value& childrenValue =  treeObj["children"];
				ii_Read(pCacheStorage, relativePath, pCurrentNode,NULL, childrenValue);
			}
			if (treeObj.HasMember("childrenUri") && treeObj["childrenUri"].IsString())
			{
				CGString childrenUri = CGString(treeObj["childrenUri"].GetString(),CGString::EncodeType::UTF8) ;
				CGString newChildrenUri =   MakePathByRelativePath(relativePath, childrenUri);

				CGByteArray buffer;
				pCacheStorage->GetContent(newChildrenUri, buffer);
				if (buffer.size() > 0)
				{
					rapidjson::Document doc;
					if (doc.Parse(buffer.data(), buffer.size()).HasParseError())
						return 0;
					if (!doc.IsObject())
						return 0;
					return ii_Read(pCacheStorage, relativePath, pCurrentNode, NULL, doc);
				}
			}
		}
	}

	if(treeObj.HasMember("items") && treeObj["items"].IsArray() )
	{//表示子项
		if (pParentNode != NULL)
		{
			for (int i = 0;i < treeObj["items"].Size();i++)
			{
				rapidjson::Value& itemValue = treeObj["items"][i];

				if (itemValue.HasMember("childSize"))
				{
					StructureTreeNode tree;
					pParentNode->AppendChild(tree);
					StructureTree* itemChild = pParentNode->GetChild(pParentNode->GetChildNum() -1);
					ii_Read(pCacheStorage, relativePath, pParentNode, (StructureTreeNode*)itemChild, itemValue);
				}
				else
				{
					StructureTreeLeaf leaf;
					if (treeObj.HasMember("level") && treeObj["level"].IsNumber())
					{
						leaf.level = JsonValueToInt(treeObj["level"]);
					}
					if (treeObj.HasMember("name") && treeObj["name"].IsString())
					{
						leaf.name = CGString(treeObj["name"].GetString(), CGString::EncodeType::UTF8);
					}
					if (treeObj.HasMember("property") && treeObj["property"].IsObject())
					{
						rapidjson::Value& propertyValue = treeObj["property"];

						if (propertyValue.HasMember("OID") && propertyValue["OID"].IsNumber())
						{
							leaf.oid = JsonValueToInt64(propertyValue["OID"]);
						}
						if (propertyValue.HasMember("tid") && propertyValue["tid"].IsNumber())
						{
							leaf.id = JsonValueToInt64(propertyValue["tid"]);
						}
						if (propertyValue.HasMember("layerID") && propertyValue["layerID"].IsNumber())
						{
							leaf.layerID = JsonValueToInt64(propertyValue["layerID"]);
						}
						if (propertyValue.HasMember("box") && propertyValue["box"].IsArray())
						{
							if (propertyValue["box"].Size() == 6)
							{
								for (int i = 0;i < 6;i++)
								{
									if (propertyValue["box"][i].IsNumber())
									{
										pCurrentNode->box.push_back(propertyValue["box"][i].GetDouble());
									}
								}
							}
						}
					}
					pParentNode->AppendChild(leaf);
				}
			}
		}

		if (treeObj.HasMember("nextItemsUri") && treeObj["nextItemsUri"].IsString())
		{
			CGString nextItemsUri = CGString(treeObj["nextItemsUri"].GetString(), CGString::EncodeType::UTF8);
			CGString newNextItemsUri = MakePathByRelativePath(relativePath, nextItemsUri);

			CGByteArray buffer;
			pCacheStorage->GetContent(newNextItemsUri, buffer);
			if (buffer.size() > 0)
			{
				rapidjson::Document doc;
				if (doc.Parse(buffer.data(), buffer.size()).HasParseError())
					return 0;
				if (!doc.IsObject())
					return 0;
				return ii_Read(pCacheStorage, relativePath, pCurrentNode, NULL, doc);
			}
		}
	}
	return 1;
}

gisLONG Ci_StructureTreeManager::i_Read(MapGIS::Tile::G3DCacheStorage*	pCacheStorage, CGString relativePath)
{
	CGByteArray buffer;
	pCacheStorage->GetContent(relativePath, buffer);
	if (buffer.size() > 0)
	{
		rapidjson::Document doc;
		if (doc.Parse(buffer.data(), buffer.size()).HasParseError())
			return 0;
		if (!doc.IsObject())
			return 0;
		return ii_Read(pCacheStorage, relativePath,NULL,m_pRootNode, doc);
	}
	return 0;
}

gisLONG Ci_StructureTreeManager::Read(MapGIS::Tile::G3DCacheStorage*	pCacheStorage, CGString relativePath, StructureTreeNode& rootNode)
{
	rootNode.Clear();
	Ci_StructureTreeManager treeManager(rootNode);
	return treeManager.i_Read(pCacheStorage, relativePath);
}
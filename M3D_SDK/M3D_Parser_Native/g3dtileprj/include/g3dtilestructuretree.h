#pragma once
#ifndef _G3D_TILE_STRUCTURETREE_H_
#define _G3D_TILE_STRUCTURETREE_H_

#include "g3dtiledefine.h"
#include <vector>

using namespace std;

namespace MapGIS
{
	namespace Tile
	{
		//================================================================================
		/// 结构树节点类型枚举
		/// 定义了结构树中节点的两种基本类型
		//================================================================================
		enum class StructureTreeType :int
		{
			Node = 0,   // 内部节点（非叶子节点）
			Leaf = 1    // 叶子节点
		};

		//================================================================================
		/// 结构树基类
		/// 所有结构树节点的抽象基类，定义了通用接口
		//================================================================================
		class MAPGISG3DTILEEXPORT StructureTree
		{
		public:
			//================================================================================
			/// 获取节点类型
			///
			/// @return 节点类型枚举值
			//================================================================================
			virtual StructureTreeType GetType() const = 0;
			
			// 默认构造函数
			StructureTree() {};
			
			// 虚析构函数，确保派生类能正确析构
			virtual ~StructureTree() {};
		};

		//================================================================================
		/// 结构树叶子节点类
		/// 表示结构树中的叶节点，包含具体的几何对象信息
		//================================================================================
		class MAPGISG3DTILEEXPORT StructureTreeLeaf :public StructureTree
		{
		public:
			CGString		name;      // 节点名称
			gisINT64		id;        // 节点ID
			gisINT64		oid;       // 对象ID（原始对象标识）
			gisLONG			level;     // 节点层级
			gisINT64		layerID;   // 图层ID
			vector<double>	box;       // 包围盒（通常包含6个值：minX, minY, minZ, maxX, maxY, maxZ）

			//================================================================================
			/// 获取节点类型
			///
			/// @return StructureTreeType::leaf（叶子节点类型）
			//================================================================================
			StructureTreeType GetType() const { return StructureTreeType::Leaf; }
			
			//================================================================================
			/// 默认构造函数
			/// 初始化所有成员变量为默认值
			//================================================================================
			StructureTreeLeaf()
			{
				name = "";
				id = 0;
				oid = 0;
				layerID = 0;
				level = 0;
			};
		};

		//================================================================================
		/// 结构树内部节点类
		/// 表示结构树中的内部节点，可以包含子节点（叶子节点或其他内部节点）
		//================================================================================
		class MAPGISG3DTILEEXPORT StructureTreeNode :public StructureTree
		{
		public:
			CGString				name;      // 节点名称
			gisINT64				minID;     // 子节点最小ID
			gisINT64				maxID;     // 子节点最大ID
			vector<double>			box;       // 包围盒（通常包含6个值：minX, minY, minZ, maxX, maxY, maxZ）
			gisLONG					level;     // 节点层级

			//================================================================================
			/// 获取节点类型
			///
			/// @return StructureTreeType::node（内部节点类型）
			//================================================================================
			StructureTreeType GetType() const { return StructureTreeType::Node; }

			//================================================================================
			/// 默认构造函数
			/// 初始化所有成员变量为默认值
			//================================================================================
			StructureTreeNode()
			{
				name = "";
				minID = 0;
				maxID = 0;
				level = 0;
			};

			//================================================================================
			/// 析构函数
			/// 自动释放所有子节点的内存
			//================================================================================
			~StructureTreeNode()
			{
				for (vector<StructureTree*>::iterator itr = m_children.begin(); itr != m_children.end(); itr++)
				{
					if (*itr != NULL)
						delete *itr;
				}
			};

			//================================================================================
			/// 添加子节点（内部节点版本）
			/// 深拷贝指定的内部节点及其所有子节点
			///
			/// @param [in] node 要添加的内部节点
			///
			/// @return 操作结果
			///         - 1：成功
			//================================================================================
			gisLONG AppendChild(const StructureTreeNode& node)
			{
				StructureTreeNode *pNode = new StructureTreeNode();
				pNode->name = node.name;
				pNode->level = level + 1;
				pNode->minID = node.minID;
				pNode->maxID = node.maxID;
				
				// 递归拷贝所有子节点
				for (vector<StructureTree*>::const_iterator itr = node.m_children.begin(); itr != node.m_children.end(); itr++)
				{
					StructureTreeType type = (*itr)->GetType();
					if (type == StructureTreeType::Node)
					{
						const StructureTreeNode* pChild = dynamic_cast<const StructureTreeNode*>(*itr);
						pNode->AppendChild(*pChild);
					}
					else
					{
						const StructureTreeLeaf* pChild = dynamic_cast<const StructureTreeLeaf*>(*itr);
						pNode->AppendChild(*pChild);
					}
				}
				m_children.emplace_back(pNode);
				return 1;
			}

			//================================================================================
			/// 添加子节点（叶子节点版本）
			/// 拷贝指定的叶子节点信息
			///
			/// @param [in] leaf 要添加的叶子节点
			///
			/// @return 操作结果
			///         - 1：成功
			//================================================================================
			gisLONG AppendChild(const StructureTreeLeaf& leaf)
			{
				StructureTreeLeaf *pLeaf = new StructureTreeLeaf();
				pLeaf->id = leaf.id;
				pLeaf->name = leaf.name;
				pLeaf->oid = leaf.oid;
				pLeaf->level = level + 1;
				pLeaf->layerID = leaf.layerID;
				pLeaf->box.insert(pLeaf->box.begin(), leaf.box.begin(), leaf.box.end());
				m_children.emplace_back(pLeaf);
				return 1;
			}

			//================================================================================
			/// 获取子节点数量
			///
			/// @return 子节点数量
			//================================================================================
			gisLONG GetChildNum() const
			{
				return m_children.size();
			}

			//================================================================================
			/// 清空所有子节点
			/// 释放所有子节点内存并清空子节点列表
			//================================================================================
			void Clear()
			{
				for (vector<StructureTree*>::iterator itr = m_children.begin(); itr != m_children.end(); itr++)
				{
					if (*itr != NULL)
						delete *itr;
				}
				m_children.clear();
			}

			//================================================================================
			/// 获取指定索引的子节点（非const版本）
			///
			/// @param [in] index 子节点索引
			///
			/// @return 指向子节点的指针，索引无效时返回nullptr
			//================================================================================
			StructureTree* GetChild(int index)
			{
				if (index < 0 || index >= m_children.size())
					return 0;
				return m_children[index];
			}
			
			//================================================================================
			/// 获取指定索引的子节点（const版本）
			///
			/// @param [in] index 子节点索引
			///
			/// @return 指向子节点的const指针，索引无效时返回nullptr
			//================================================================================
			const StructureTree* GetChild(int index) const
			{
				if (index < 0 || index >= m_children.size())
					return 0;

				return m_children[index];
			}

		private:
			vector<StructureTree*>	m_children;  // 子节点列表
		};
	}
}
#endif
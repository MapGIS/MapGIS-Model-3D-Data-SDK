#pragma once
#ifndef RAPID_H_
#define RAPID_H_

#include "writer.h"
#include "stringbuffer.h"
#include "document.h"

namespace rapidjson
{
	class CRapid
	{
	public:
		CRapid() 
		{
			mDocument = new Document();
			mDocument->Parse("{}");

			allocator = new Document::AllocatorType();
			allocator = &mDocument->GetAllocator();
		};
		CRapid(Type type)
		{
			mValue = new Value(type);
		};
		~CRapid() 
		{	
			if (NULL != mValue)
			{
				delete mValue;
				mValue = NULL;
			}
			if (NULL != allocator)
			{
				delete allocator;
				allocator = NULL;
			}
			if (NULL != mDocument)
			{
				delete mDocument;
				mDocument = NULL;
			}			
		};

	private:
		Document* mDocument;
		Value* mValue;
		Document::AllocatorType* allocator;

	public:
		template<typename T>
		void pushBack(const char* key, T value)
		{
			mDocument->AddMember(key, value, allocator);
		};
		
	};
};

#endif
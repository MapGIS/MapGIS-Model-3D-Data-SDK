#include "stdafx.h"
#include "ci_3dmodel_tid.h"
#include "ci_assist.h"
#include "gbytearray.h"

using namespace MapGIS::Tile;

gisLONG Ci_ModelTidFile::From(const CGByteArray& in, vector<vector<gisINT64>>& out)
{
	out.clear();
	int c = 0;
	string magic = ReadByteArrayToString(in, c, 4); c += 4;
	unsigned int version = ReadByteArrayToUnsignedInt32(in.data(), c); c += 4;
	unsigned int bytesLength = ReadByteArrayToUnsignedInt32(in, c); c += 4;
	unsigned int tilesLength = ReadByteArrayToUnsignedInt32(in, c); c += 4;
	if (StrNICmp("tid", magic.c_str(), 3) != 0 || tilesLength <= 0 || in.size() < bytesLength)
		return 0;
	vector<gisUINT> offsetVtr;
	for (int i = 0; i < tilesLength; i++)
	{
		unsigned int offset = ReadByteArrayToUnsignedInt32(in, c); c += 4;
		offsetVtr.emplace_back(offset);
	}
	for (int i = 0; i < tilesLength; i++)
	{
		vector<gisINT64> item;
		int offset = c + offsetVtr[i];
		unsigned int type = ReadByteArrayToUnsignedInt32(in, offset); offset += 4;
		unsigned int tidsLength = ReadByteArrayToUnsignedInt32(in, offset); offset += 4;

		if (type == 16)
		{
			for (int j = 0; j < tidsLength; j++)
			{
				unsigned short value = ReadByteArrayToUnsignedInt16(in, offset); offset += 2;
				item.emplace_back((gisINT64)value);
			}
		}
		else if (type == 32)
		{
			for (int j = 0; j < tidsLength; j++)
			{
				unsigned int value = ReadByteArrayToUnsignedInt32(in, offset); offset += 4;
				item.emplace_back((gisINT64)value);
			}
		}
		else if (type == 64)
		{
			for (int j = 0; j < tidsLength; j++)
			{
				gisUINT64 value = ReadByteArrayToUnsignedInt64(in, offset); offset += 8;
				item.emplace_back((gisINT64)value);
			}
		}
		else
		{
			out.clear();
			return 0;
		}
		out.emplace_back(item);
	}
	return 1;
}
gisLONG Ci_ModelTidFile::To(const vector<vector<gisINT64>>& in, CGByteArray& out)
{
	out.clear();
	vector<gisUINT> type;
	vector<gisUINT> length;
	vector<CGByteArray> buffers;
	if (in.size() > 0)
	{
		for (vector<vector<gisINT64>>::const_iterator itr = in.begin(); itr != in.end(); itr++)
		{
			gisINT64 maxValue = 0;
			for (vector<gisINT64>::const_iterator valueItr = itr->begin(); valueItr != itr->end(); valueItr++)
			{
				maxValue = MAX(maxValue, *valueItr);
			}
			CGByteArray buffer;
			if (maxValue <= UINT16_MAX)
			{
				type.emplace_back(16);
				for (vector<gisINT64>::const_iterator valueItr = itr->begin(); valueItr != itr->end(); valueItr++)
				{
					gisUSHORT value = *valueItr > 0 ? (gisUSHORT)*valueItr : (gisUSHORT)0;
					buffer.append((char*)&value, 2);
				}
			}
			else if (maxValue <= UINT32_MAX)
			{
				type.emplace_back(32);
				for (vector<gisINT64>::const_iterator valueItr = itr->begin(); valueItr != itr->end(); valueItr++)
				{
					gisUINT value = *valueItr > 0 ? (gisUINT)*valueItr : (gisUINT)0;
					buffer.append((char*)&value, 4);
				}
			}
			else
			{
				type.emplace_back(64);
				for (vector<gisINT64>::const_iterator valueItr = itr->begin(); valueItr != itr->end(); valueItr++)
				{
					gisUINT64 value = *valueItr > 0 ? (gisUINT64)*valueItr : (gisUINT64)0;
					buffer.append((char*)&value, 8);
				}
			}
			int nRemainLen = buffer.length() % 8;
			if (nRemainLen > 0)
				buffer.append(8 - nRemainLen, ' ');
			buffers.emplace_back(buffer);
			length.emplace_back(itr->size());
		}

		gisUINT version = 1;
		gisUINT tilesLength = in.size();

		gisUINT byteLength = 16 + 4 * tilesLength;

		for (int i = 0; i < tilesLength; i++)
		{
			byteLength += 8 + buffers[i].size();
		}

		out.append("tid\0", 4);
		out.append((const char*)&version, 4);
		out.append((const char*)&byteLength, 4);
		out.append((const char*)&tilesLength, 4);

		gisUINT offset = 0;

		for (int i = 0; i < tilesLength; i++)
		{
			out.append((const char*)&offset, 4);
			offset += 8 + buffers[i].size();
		}

		for (int i = 0; i < tilesLength; i++)
		{
			out.append((const char*)&type[i], 4);

			out.append((const char*)&length[i], 4);
			out.append((const char*)buffers[i].data(), buffers[i].size());
		}
	}
	return 1;
}
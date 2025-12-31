#pragma once
#include <vector>

class CGByteArray
{
public:

	CGByteArray();
	CGByteArray(const CGByteArray &p);
	CGByteArray(const char *, int size = -1);
	CGByteArray(int size, char c);
	CGByteArray(CGByteArray&& other);

	~CGByteArray();
	CGByteArray &operator=(const CGByteArray &other);
	CGByteArray &operator=(const char *str);
	CGByteArray& operator=(CGByteArray&& other);

	char *data();
	const char *data() const;
	const char *constData() const;
	void clear();

	char at(int i) const;
	char back() const;
	char front() const;

	int size() const;
	bool isEmpty() const;
	void resize(int size);

	int capacity() const;
	void reserve(int size);

	int count() const;
	int length() const;
	bool isNull() const;

	CGByteArray &append(char c);
	CGByteArray &append(int count, char c);
	CGByteArray &append(const char *s);
	CGByteArray &append(const char *s, int len);
	CGByteArray &append(const CGByteArray &a);

	CGByteArray &insert(int i, char c);
	CGByteArray &insert(int i, int count, char c);
	CGByteArray &insert(int i, const char *s);
	CGByteArray &insert(int i, const char *s, int len);
	CGByteArray &insert(int i, const CGByteArray &a);

	CGByteArray &replace(int index, int len, const char *s);
	CGByteArray &replace(int index, int len, const char *s, int alen);
	CGByteArray &replace(int index, int len, const CGByteArray &s);

	enum CompressType
	{
		CompressType_None = 0,
		CompressType_GZip = 1
	};

	static CGByteArray Compress(const CGByteArray& data, CGByteArray::CompressType type);
	static CGByteArray Decompress(const CGByteArray& data, CGByteArray::CompressType type);

private:
	std::vector<char> m_d;
};
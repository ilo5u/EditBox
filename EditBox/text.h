#pragma once
#include <Windows.h>

constexpr int g_cSize = 0xfff;
template <class Valty>
class CText
{
public:
	typedef struct RowHead
	{
		LPWSTR pText;
		RowHead pNext;
	} ROW, *LPROW;

public:
	CText() :
		m_size(0), m_pRow(nullptr)
	{
	}
	~CText()
	{
		free();
	}

public:
	BOOL free()
	{
		LPROW p = m_pRow;
		while (p != nullptr)
		{
			LPROW q = p->pNext;
			delete[] p;
			p = q;
		}
		m_pRow = nullptr;
	}

private:
	int m_size;
	LPROW m_pRow;
};
#pragma once
#include"CText.h"
#include"Cursor.h"
#include<Windows.h>

//��¼������Ϣ
#define RD_DELETE		0x00000001
#define RD_INSERT		0x00000010
#define RD_RETURN		0x00000100
#define RD_MERGE_LINE	0x00010000
struct Record
{
	Record(UINT flag);
	~Record();
	void ReDo(CText* p);				//����
	void Save_Delete_Data(CText* pc, Position first, Position last);	//ɾ��ǰ����ɾ����Ϣ
	void Save_Merge_Line_Data(CText* p, int LineNumber);			//��¼�ϲ�����Ϣ
	void Set_Choose_Data(Position first, Position last);			//����ѡ����Ϣ
	void Clear_Choose_Data();
	Position start;						//ѡ�п�ʼ
	Position end;						//ѡ�н���
	CText* pText;						//�󶨵��ı����
	UINT ACT;							//��һ���Ķ���
	std::wstring Deleted_Data;			//�洢ɾ����Ϣ
};


/*
Ӧ�ñ����״̬
ɾ��
����
�س�
ѡ����Ϣ
*/


/*
RD_RETURN �� �س�
start ��¼���»س���λ�� (�����start���»س�)
eg. 1   1234|56    start={1,4}
result:
1	1234
2	56

������ָ�ԭ״��start�Լ�¼���ǰ���ַ�λ��
*/

/*
RD_INSERT
start ��¼�������ʼλ��
end   ��¼�������ֹλ��
�� ���������λ[start,end]
*/



/*
RD_DELETE
start			��¼��ɾ�������
Deleted_Data	��¼��ɾ��������
*/

/*
RD_MERGE_LINE
�Ժϲ��еĳ���
�ϲ��У�
1    ABCDE
2	 |123456
����backspace
1	 ABCDE123456

start ��¼���»س���λ�ã�������Ϊ{1,5}��
Save_Merge_Line_Data(CText* p,int LineNumber);
�ϲ�ǰ�ڶ����к�
*/
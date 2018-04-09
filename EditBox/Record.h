#pragma once
#include"CText.h"
#include"Cursor.h"
#include<Windows.h>
//��¼������Ϣ
#define RD_DELETE 0x00000001
#define RD_INSERT 0x00000010
#define RD_RETURN 0x00000100
#define RD_CHOOSE 0x00001000
struct Record
{
	Record(UINT flag);
	~Record();
	void ReDo(CText* p);				//����
	void Save_Delete_Data(CText* pc,Position first,Position last);	//ɾ��ǰ����ɾ����Ϣ
	void Save_Insert_Data(Position first,Position last);			//���������Ϣ
	void Set_Choose_Data(Position first, Position last);			//����ѡ����Ϣ
	void Clear_Choose_Data();
	Position start;						//ѡ�п�ʼ
	Position end;						//ѡ�н���
	CText* pText;						//�󶨵��ı����
	CText* pData;						//������Ϣ
	UINT ACT;							//��һ���Ķ���
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

	������ָ�ԭ״��statr�Լ�¼���ǰ���ַ�λ��
*/

/*
RD_INSERT
start ��¼�������ʼλ��
end   ��¼�������ֹλ��
�� ���������λ[start,end]
*/

/*
RD_CHOOSE
start �� end ��¼�˹��ѡ����Ϣ
*/

/*
RD_DELETE
start ��¼��ɾ�������
end   ��¼��ɾ�����յ�
pData ��¼��ɾ��������
*/
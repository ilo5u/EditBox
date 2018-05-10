#pragma once
#include"CText.h"
#include"Cursor.h"
#include<Windows.h>

//记录操作信息
#define RD_DELETE		0x00000001
#define RD_INSERT		0x00000010
#define RD_RETURN		0x00000100
#define RD_MERGE_LINE	0x00010000
struct Record
{
	Record(UINT flag);
	~Record();
	void ReDo(CText* p);				//撤销
	void Save_Delete_Data(CText* pc, Position first, Position last);	//删除前保留删除信息
	void Save_Merge_Line_Data(CText* p, int LineNumber);			//记录合并行信息
	void Set_Choose_Data(Position first, Position last);			//设置选中信息
	void Clear_Choose_Data();
	Position start;						//选中开始
	Position end;						//选中结束
	CText* pText;						//绑定的文本句柄
	UINT ACT;							//上一步的动作
	std::wstring Deleted_Data;			//存储删除信息
};


/*
应该保存的状态
删除
插入
回车
选中信息
*/


/*
RD_RETURN ： 回车
start 记录按下回车的位置 (光标在start后按下回车)
eg. 1   1234|56    start={1,4}
result:
1	1234
2	56

撤销后恢复原状，start仍记录光标前的字符位置
*/

/*
RD_INSERT
start 记录插入的起始位置
end   记录插入的终止位置
即 插入的内容位[start,end]
*/



/*
RD_DELETE
start			记录了删除的起点
Deleted_Data	记录了删除的内容
*/

/*
RD_MERGE_LINE
对合并行的撤销
合并行：
1    ABCDE
2	 |123456
按下backspace
1	 ABCDE123456

start 记录按下回车的位置（此例中为{1,5}）
Save_Merge_Line_Data(CText* p,int LineNumber);
合并前第二行行号
*/
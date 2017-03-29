#pragma once
class DB
{
public:
	DB();
	~DB();

public:
	void Init(void);
	bool Start(void);
	bool Stop(void);
	void Fini(void);

private:
	//添加一个指向Connection对象的指针:
	_ConnectionPtr m_pConnection;
	//添加一个指向Recordset对象的指针:
	_RecordsetPtr m_pRecordset;
	//数据库链接句柄锁
	CRITICAL_SECTION m_csDB;

public:
	// 初始化—连接数据库
	void OnInitADOConn();
	// 执行查询
	_RecordsetPtr& GetRecordSet(_bstr_t bstrSQL);
	// 执行SQL语句，Insert Update _variant_t
	BOOL ExecuteSQL(_bstr_t bstrSQL);

	void ExitConnect();
};


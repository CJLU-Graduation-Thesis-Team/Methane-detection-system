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
	//���һ��ָ��Connection�����ָ��:
	_ConnectionPtr m_pConnection;
	//���һ��ָ��Recordset�����ָ��:
	_RecordsetPtr m_pRecordset;
	//���ݿ����Ӿ����
	CRITICAL_SECTION m_csDB;

public:
	// ��ʼ�����������ݿ�
	void OnInitADOConn();
	// ִ�в�ѯ
	_RecordsetPtr& GetRecordSet(_bstr_t bstrSQL);
	// ִ��SQL��䣬Insert Update _variant_t
	BOOL ExecuteSQL(_bstr_t bstrSQL);

	void ExitConnect();
};


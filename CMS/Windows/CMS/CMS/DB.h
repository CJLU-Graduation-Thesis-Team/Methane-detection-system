#pragma once


typedef   struct   DBInfo
{
	std::string strDSN;
	std::string strServer;
	std::string strDataBase;
	std::string strUserName;
	std::string strPassWd;

	DBInfo()
	{
		strDSN = "MySql";
		strServer = "localhost";
		strDataBase = "methane-detection-system";
		strUserName = "root";
		strPassWd = "cjlu12345+";
	}
} DBConfig;



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

	static bool SetDBConfig(DBConfig strDBInfo);

private:
	static DBConfig m_dbConfig;
	static std::string strConnInfo;

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


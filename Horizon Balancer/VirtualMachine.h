#pragma once

// �����, �������� ���������� � ������
#include "Host.h"

class Host;

class VM
{
public:

	VM()
	{
		priority = 1000;
	};


	/*
		����������� ������ VM
		-----------
		��������� :
			��������� �� ���� ������ VM.
	*/
	VM(int id_in, int priority_in, Host* host_in, std::vector<Host*> approved_hosts_in) :
		id(id_in), priority(priority_in), host(host_in), approved_hosts(approved_hosts_in)
	{};

	~VM() {};


	/*
		���������� ��������� ����������.
		����������� ��� ���� ������������ �������
		-----------
		��������� :
			VM obj : ������������� ������ ������
	*/
	void operator=(VM obj)
	{
		id = obj.id;
		priority = obj.priority;
		host = obj.host;
		approved_hosts = obj.approved_hosts;

	};


	/*
		���������� ��������� ���������.
		���������� ��� ������ ������� ���� VM.
		-----------
		��������� :
			VM obj : ������ ��� ���������
	*/
	bool operator==(VM obj)
	{
		return id == obj.id;
	}

	bool operator==(VM* obj)
	{
		return id == obj->id;
	}


	/*
		������ ����������� ������:
		--------------------------
		int id :
			id ������

		int datastore_id:
			id ���������, �� ������� ����������� ������

		int priority:
			��������� ������

		Host host:
			����, �� ������� VM ������ ����������

		vector<Host> approved_hosts:
			id ������, �� ������� ������ ����� ����������

		std::vector<string> labels:
			������, � ������� ��������� ������
	*/

	int id;

	int datastore_id;

	int priority;

	Host* host;

	std::vector<Host*> approved_hosts;

	std::vector<std::string> labels;
};
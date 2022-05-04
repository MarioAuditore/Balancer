#pragma once

// �����, �������� ���������� � ����������� �������
#include "VirtualMachine.h"
// STL
#include <vector>
#include <string>


class VM;

class Host
{
public:

	// ����������� �� ���������
	Host() {};

	// �����������, ���� ������ ����� ��������
	Host(int n_loads)
	{
		loads = std::vector<double>(n_loads);
		max_load_trigger = std::vector<double>(n_loads, 0.85);
		max_time_trigger = std::vector<int>(n_loads, 30);
	};


	/*
		����������� ������ Host
		----------
		���������:
			������ �����.

		������ ���� overload_rick �� ��������� false
	*/
	Host(int id_in,
		std::vector<VM*> deployed_machines_in,
		std::vector<double> max_load_trigger_in,
		std::vector<int> max_time_trigger_in
	) : id(id_in),
		deployed_machines(deployed_machines_in),
		max_load_trigger(max_load_trigger_in),
		max_time_trigger(max_time_trigger_in)
	{
		overload_risk = false;
		loads = std::vector<double>(max_load_trigger_in.size());
	};

	~Host() {};


	/*
		���������� ��������� ���������.
		���������� ��� ������ �������� ���� Host
		-----------
		��������� :
			Host obj : ������ ��� ���������
	*/
	bool operator== (Host obj)
	{
		return id == obj.id;
	}

	bool operator== (Host* obj)
	{
		return id == obj->id;
	}


	/*
		������ �����:
		-------------
		int id :
			id �����

		vector<VM> deployed_machines :
			����������� ������, ���������� �� ������ �����

		vector<double> loads :
			 ������ �������� �������� � ������ ������:
			 1) cpu
			 2) ram
			 3) disk
	*/

	int id;

	std::vector<VM*> deployed_machines;

	std::vector<double> loads;


	/*
		�������� ��� ������������:
		--------------------------
		vector<double> max_load_trigger:
			 ������ ������������ �������� ������� ����, ��� ������� �������� ������ �������
			 1) max cpu load
			 2) max ram load
			 3) max disk load
			 0 - ��� ��� �����

		vector<int> max_time_trigger:
			 ������ ������������� ����������� ������� ����������� ��������.
			 ���� �������� ����� �� ������ � ������� ������� ������, ��������� �������������
			 1) max time of cpu load
			 2) max time of ram load
			 3) max time of disk load
			 0 - ��� ��� �����

		bool overload_risk:
			 ����, ������������ ���� ���������� �����.
			 True ��� �� 1 - �� ������ ��������� �������������
			 False ��� �� 0 - �� ������ ���������� �� ���������
	*/

	std::vector<double> max_load_trigger;

	std::vector<int> max_time_trigger;

	bool overload_risk = false;

};
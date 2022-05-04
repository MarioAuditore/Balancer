#pragma once

// Класс, хранящий информацию о хостах
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
		Конструктор класса VM
		-----------
		Параметры :
			Принимает на вход данные VM.
	*/
	VM(int id_in, int priority_in, Host* host_in, std::vector<Host*> approved_hosts_in) :
		id(id_in), priority(priority_in), host(host_in), approved_hosts(approved_hosts_in)
	{};

	~VM() {};


	/*
		Перегрузка оператора присвоения.
		Присваивает все поля присваимаего объекта
		-----------
		Параметры :
			VM obj : присваиваемый объект класса
	*/
	void operator=(VM obj)
	{
		id = obj.id;
		priority = obj.priority;
		host = obj.host;
		approved_hosts = obj.approved_hosts;

	};


	/*
		Перегрузка оператора равенства.
		Необходима при поиске объекта типа VM.
		-----------
		Параметры :
			VM obj : объект для сравнения
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
		Данные виртуальной машины:
		--------------------------
		int id :
			id машины

		int datastore_id:
			id хранилища, на котором расположена машина

		int priority:
			Приоритет машины

		Host host:
			Хост, на котором VM сейчас развернута

		vector<Host> approved_hosts:
			id хостов, на которых машину можно разместить

		std::vector<string> labels:
			Лейблы, к которым привязана машина
	*/

	int id;

	int datastore_id;

	int priority;

	Host* host;

	std::vector<Host*> approved_hosts;

	std::vector<std::string> labels;
};
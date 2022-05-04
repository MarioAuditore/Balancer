#pragma once

// Класс, хранящий информацию о виртуальных машинах
#include "VirtualMachine.h"
// STL
#include <vector>
#include <string>


class VM;

class Host
{
public:

	// Конструктор по умолчанию
	Host() {};

	// Конструктор, если задаем число нагрузок
	Host(int n_loads)
	{
		loads = std::vector<double>(n_loads);
		max_load_trigger = std::vector<double>(n_loads, 0.85);
		max_time_trigger = std::vector<int>(n_loads, 30);
	};


	/*
		Конструктор класса Host
		----------
		Параметры:
			данные хоста.

		Ставит флаг overload_rick по умолчанию false
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
		Перегрузка оператора равенства.
		Необходима при поиске элемента типа Host
		-----------
		Параметры :
			Host obj : объект для сравнения
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
		Данные хоста:
		-------------
		int id :
			id хоста

		vector<VM> deployed_machines :
			Виртуальные машины, работающие на данном хосте

		vector<double> loads :
			 Вектор значений нагрузок в данный момент:
			 1) cpu
			 2) ram
			 3) disk
	*/

	int id;

	std::vector<VM*> deployed_machines;

	std::vector<double> loads;


	/*
		Триггеры для балансировки:
		--------------------------
		vector<double> max_load_trigger:
			 Вектор максимальных нагрузок каждого типа, при которых начнется отсчет времени
			 1) max cpu load
			 2) max ram load
			 3) max disk load
			 0 - нам все равно

		vector<int> max_time_trigger:
			 Вектор максимального допустимого времени критической нагрузки.
			 Если нагрузка хоста не спадет в течение данного лимита, вызовется балансировщик
			 1) max time of cpu load
			 2) max time of ram load
			 3) max time of disk load
			 0 - нам все равно

		bool overload_risk:
			 Флаг, отображающий риск перегрузки хоста.
			 True или же 1 - за хостом наблюдает балансировщик
			 False или же 0 - за хостом наблюдения не требуется
	*/

	std::vector<double> max_load_trigger;

	std::vector<int> max_time_trigger;

	bool overload_risk = false;

};
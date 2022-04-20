#pragma once
// Генератор случайной нагрузки
#include <random>
#include <list>
#include <iostream>
#include "Balancer.h"


// Специально забиваем высокую нагрузку
void set_high_load(Host& host)
{
	for (int i = 0; i < host.loads.size(); ++i)
	{
		if (host.max_load_trigger[i] > 0)
		{
			host.loads[i] = host.max_load_trigger[i];
		}
	}
}


// Генератор случайных чисел для нагрузки
std::random_device rd;
std::mt19937 mersenne(rd());

// Функция генерации нагрузки
void set_random_load(Host& host)
{
	for (int i = 0; i < host.loads.size(); ++i)
	{
		if (host.loads[i] == 0)
		{
			//std::uniform_int_distribution<int> uni(0, 100);
			//host.loads[i] = uni(mersenne);
			host.loads[i] = 50;
		}
		else
		{
			std::uniform_int_distribution<int> uni(std::max(0, int(host.loads[i] - 10)), std::min(100, int(host.loads[i] + 10)));
			//std::uniform_int_distribution<int> uni(std::max(0, int(host.loads[i])), std::min(100, int(host.loads[i] + 10)));
			host.loads[i] = uni(mersenne);
		}

	}
};

void create_vm(std::list<VM>& all_vms, std::list<Host>& all_hosts, Balancer balancer)
{
	/*
		Сперва создадим новую машину
	*/
	std::cout << "=====\n";
	std::cout << "Creating new VM\n";

	std::cout << "Enter VM id: ";
	int vm_id;
	std::cin >> vm_id;
	auto vm_iter = std::find_if
	(
		all_vms.begin(),
		all_vms.end(),
		[target_id = vm_id](VM x)
		{ return x.id == target_id; }
	);

	while (vm_iter != all_vms.end())
	{
		std::cout << "List of occupied ids: ";
		for (VM print_vm_id : all_vms)
		{
			std::cout << print_vm_id.id << " ";
		}
		std::cout << "\nVM with such id already exists\nEnter a valid one: ";
		std::cin >> vm_id;
		vm_iter = std::find_if
		(
			all_vms.begin(),
			all_vms.end(),
			[target_id = vm_id](VM x)
			{ return x.id == target_id; }
		);
	}

	VM new_vm_tmp = VM();
	all_vms.push_back(new_vm_tmp);
	auto buf_iter = all_vms.end();
	--buf_iter;
	VM* our_new_vm_ptr = &*(buf_iter);

	our_new_vm_ptr->id = vm_id;
	std::cout << '\n';

	std::cout << "Enter priority: ";
	std::cin >> our_new_vm_ptr->priority;
	std::cout << '\n';

	int id = 0;
	std::cout << "Enter ids of approved hosts, finish by writing -1.\n";
	while (id >= 0)
	{
		std::cout << "id: ";
		std::cin >> id;
		std::cout << "\n";

		if (id >= 0)
		{
			auto approved_host_iter = std::find_if
			(
				all_hosts.begin(),
				all_hosts.end(),
				[target_id = id](Host x)
				{ return x.id == target_id; }
			);

			if (approved_host_iter != all_hosts.end())
			{
				our_new_vm_ptr->approved_hosts.push_back(&*approved_host_iter);
			}
			else
			{
				std::cout << "Such host does not exist\n";
			}
		}
		else
		{
			std::cout << "All approved hosts are saved\n";
		}
	}


	/*
		Теперь, чтобы не ссылаться на удаленный объект,
		начнем оперировать через вектор всех машин.
	*/


	int host_choice;
	std::cout << "Which host to choose for deployment?\nEnter host's id or enter -1 if you want the balancer to choose it\n";
	std::cin >> host_choice;

	if (host_choice > -1)
	{
		auto host_to_deploy_iter = std::find_if
		(
			our_new_vm_ptr->approved_hosts.begin(),
			our_new_vm_ptr->approved_hosts.end(),
			[target_id = host_choice](const Host* x)
			{ return x->id == target_id; }
		);

		while (host_to_deploy_iter == our_new_vm_ptr->approved_hosts.end())
		{
			std::cout << "No such host among approved ones\n";
			std::cout << "List of approved hosts: ";
			for (Host* appr_host : our_new_vm_ptr->approved_hosts)
			{
				std::cout << appr_host->id << " ";
			}
			std::cout << "\nEnter valid one: ";
			std::cin >> host_choice;
			host_to_deploy_iter = std::find_if
			(
				our_new_vm_ptr->approved_hosts.begin(),
				our_new_vm_ptr->approved_hosts.end(),
				[target_id = host_choice](const Host* x)
				{ return x->id == target_id; }
			);
		}

		if (host_to_deploy_iter != our_new_vm_ptr->approved_hosts.end())
		{
			our_new_vm_ptr->host = *host_to_deploy_iter;
			our_new_vm_ptr->host->deployed_machines.push_back(our_new_vm_ptr);
		}
	}
	else
	{
		our_new_vm_ptr->host = balancer.find_host_for_migration(our_new_vm_ptr->approved_hosts);
		our_new_vm_ptr->host->deployed_machines.push_back(our_new_vm_ptr);
		std::cout << "VM " << our_new_vm_ptr->id << " will be deployed on host " << our_new_vm_ptr->host->id << '\n';
	}
}

void create_host(std::list<Host>& all_hosts)
{
	std::cout << "=====\n";
	std::cout << "Creating new Host\n";
	// Нужно понять первый ли хост или уже существуют хосты с заданными нагрузками
	bool first_host;
	if (all_hosts.empty())
		first_host = true;
	else
		first_host = false;

	int host_id;
	std::cout << "Enter host id: ";
	std::cin >> host_id;

	auto host_with_same_id_itr = std::find_if(
		all_hosts.begin(),
		all_hosts.end(),
		[host_id](Host host) {return host.id == host_id; }
	);
	while (host_with_same_id_itr != all_hosts.end())
	{
		std::cout << "Host with such id already exists, try another one\n";
		std::cout << "Hint: list of occupied ids: ";
		for (Host print_id : all_hosts)
		{
			std::cout << print_id.id << " ";
		}
		std::cout << '\n';
		std::cin >> host_id;

		host_with_same_id_itr = std::find_if(
			all_hosts.begin(),
			all_hosts.end(),
			[host_id](Host host) {return host.id == host_id; }
		);
	}

	// Создадим временную переменную
	Host host_tmp;

	double max_load = 0;
	int max_time = 0;
	if (first_host)
	{
		// Меняем теперь элемент вектора
		host_tmp.id = host_id;
		all_hosts.push_back(host_tmp);
		auto curr_host_itr = all_hosts.end();
		--curr_host_itr;

		std::cout << "This is the first host to be ever created.\n The amount of loads you enter will define load parameters for other hosts\n";
		std::cout << "Now, you will need to specify the trigger values for critical loads and the maximum time they are allowed to exist. Enter -1 when you finish.\n";
		int count = 0;
		while ((max_load >= 0) && (max_time >= 0))
		{
			std::cout << "Enter max value for load " << count << ": ";
			std::cin >> max_load;
			if (max_load > -1)
			{
				curr_host_itr->max_load_trigger.push_back(max_load);
				std::cout << "\n";
			}
			else
			{
				std::cout << "Loads saved\n";
				break;
			}

			std::cout << "Enter max time for load " << count << ": ";
			std::cin >> max_time;
			if (max_time > -1)
			{
				++count;
				curr_host_itr->max_time_trigger.push_back(max_time);
				std::cout << "\n";
			}
			else
			{
				std::cout << "Loads and time triggers saved\n";
				break;
			}
		}
		curr_host_itr->loads = std::vector<double>(curr_host_itr->max_load_trigger.size());
	}
	else
	{
		auto first_host_itr = all_hosts.begin();
		int n_loads = first_host_itr->loads.size();
		// Применяем конструктор, где инициализируем векторы нулями
		host_tmp = Host(n_loads);
		host_tmp.id = host_id;
		all_hosts.push_back(host_tmp);
		auto curr_host_itr = all_hosts.end();
		--curr_host_itr;

		std::cout << "There are " << n_loads << " amount of load types\n";
		std::cout << "Now, you will need to specify the trigger values for critical loads and the maximum time they are allowed to exist. Enter -1 if you want to skip.\n";

		int i = 0;
		while ((i < n_loads) && (max_load >= 0) && (max_time >= 0))
		{

			std::cout << "Enter max value for load " << i << ": ";
			std::cin >> max_load;
			if (max_load > -1)
			{
				curr_host_itr->max_load_trigger[i] = max_load;
				std::cout << "\n";
			}
			else
			{
				std::cout << "Loads saved\n";
				break;
			}

			std::cout << "Enter max time for load " << i << ": ";
			std::cin >> max_time;
			if (max_time > -1)
			{
				curr_host_itr->max_time_trigger[i] = max_time;
				std::cout << "\n";
			}
			else
				std::cout << "Loads and time triggers saved\n";
			++i;
		}
	}

}

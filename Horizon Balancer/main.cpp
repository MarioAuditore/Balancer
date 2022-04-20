// pugixml (или же xmlrpc-c) использует умные указатели, 
// которые уже перестали быть стандартом
#define _HAS_AUTO_PTR_ETC 1

// Ввод-вывод
#include <iomanip>
#include <iostream>
// Время
#include <chrono>
// Для заморозки потока
#include <thread>
// STL
#include <list>
// Наш класс
#include "Balancer.h"
// xml запросы в OpenNebula
#include "opennebula_xmlrpc.h"



void print_VM_info(std::vector<VM*> machines)
{
	if (machines.empty())
	{
		std::cout << "No machines deployed\n";
		return;
	}
	std::cout << "Deployed machines:\n";

	for (VM* vm : machines)
		std::cout << "+" << std::setw(10) << std::setfill('-') << "+" << '\t';
	std::cout << '\n';

	for (VM* vm : machines)
		std::cout << std::format("|{:^9}|\t",
			"VM " + std::to_string(vm->id));
	std::cout << '\n';

	std::stringstream stream;
	for (VM* vm : machines)
	{
		std::cout << std::format("|{:^9}|\t",
			"prior " + std::to_string(vm->priority));
		stream.str(std::string());
	}
	std::cout << '\n';

	for (VM* vm : machines)
		std::cout << "+" << std::setw(10) << std::setfill('-') << "+" << '\t';
	std::cout << '\n';
}

void print_host_info(Host& host)
{
	std::stringstream stream;
	std::cout << "+" << std::setw(31) << std::setfill('-') << "+" << '\n';
	std::cout << std::format("|{:^30}|\n",
		"Host " + std::to_string(host.id));
	std::cout << "+" << std::setw(31) << std::setfill('-') << "+" << '\n';

	for (int i = 0; i < host.loads.size(); ++i)
	{
		stream << std::fixed << std::setprecision(2) << host.loads[i];
		std::string value = stream.str();

		if (!host.max_load_trigger.empty())
		{
			if ((host.loads[i] >= host.max_load_trigger[i]) && (host.max_load_trigger[i] > 0))
			{
				std::cout << std::format("|{:^30}|\n",
					"Load " + std::to_string(i) + ": " + value + " CRITICAL!!!");
			}
			else
			{
				std::cout << std::format("|{:^30}|\n",
					"Load " + std::to_string(i) + ": " + value);
			}
		}
		else
		{
			std::cout << std::format("|{:^30}|\n",
				"Load " + std::to_string(i) + ": " + value);
		}
		stream.str(std::string());
	}
	std::cout << "+" << std::setw(31) << std::setfill('-') << "+" << '\n';

	// Отсортируем машины по приоритету
	std::sort(host.deployed_machines.begin(), host.deployed_machines.end(),
		[](VM* vm_1, VM* vm_2) {
			return vm_1->priority < vm_2->priority;
		});

	print_VM_info(host.deployed_machines);

}


int main(int argc, char** argv)
{
	using namespace std;

// Определим систему чтобы понять тип транспорта для запросов
#ifdef _WIN32
	const string transport_type = "wininet";
#endif

#ifdef linux
	const string transport_type = "curl";
#endif

	// Проинициализируем остальные параметры для xml запросов
	string const serverUrl;
	string const session_string;

	if (argc > 0)
	{
		if (argc != 3)
		{
			cout << "Balancer accepts 2 arguments:\n"
				<< "1) Url of a server: http://server_adress:2633/RPC2\n"
				<< "2) Session string: login:password\n"
				<< endl;
			exit(1);
		}
		else
		{
			string const serverUrl = argv[0];
			string const session_string = argv[1];
		}
	}
	else
	{
		string const serverUrl = "http://192.168.92.123:2633/RPC2";
		string const session_string = "admin:admin";
	}

	// Хранилища виртуальных машин и хостов
	list<Host> all_hosts;
	list<VM> all_VMs;
	
	update_host_vm_lists(all_hosts, all_VMs, transport_type, serverUrl, session_string);

	for (list<Host>::iterator host = all_hosts.begin(); host != all_hosts.end(); ++host)
	{
		cout << "Host " << host->id << '\n';
		for (VM* vm : host->deployed_machines)
		{
			cout << "-- VM " << vm->id << '\n';
		}
	}

	for (Host host : all_hosts)
	{
		print_host_info(host);
	}

	/*
	create_host(all_hosts);

	// ОбЪявим хосты
	Host host_1 = Host(3);
	host_1.id = 1;
	host_1.max_load_trigger = vector<double>{ 50, 50, 50 };
	host_1.max_time_trigger = vector<int>{ 1, 10, 10 };
	all_hosts.push_back(host_1);

	Host host_2 = Host(3);
	host_2.id = 2;
	all_hosts.push_back(host_2);

	Host host_3 = Host(3);
	host_3.id = 3;
	host_3.max_load_trigger = vector<double>{ 10, 20, 30 };
	host_3.max_time_trigger = vector<int>{ 1, 5, 10 };
	all_hosts.push_back(host_3);

	Host host_4 = Host(3);
	host_4.id = 4;
	all_hosts.push_back(host_4);

	create_host(all_hosts);

	auto all_hosts_start_itr = all_hosts.begin();
	auto all_hosts_start_itr_copy = all_hosts_start_itr;
	// Так как это список, двигаемся через advance


	// Объявим машины
	VM vm_1;
	vm_1.id = 1;
	for (int i = 0; i < 4; ++i)
	{
		vm_1.approved_hosts.push_back(&*all_hosts_start_itr_copy);
		std::advance(all_hosts_start_itr_copy, 1);
	}

	vm_1.priority = 10;
	all_VMs.push_back(vm_1);
	// Запомним указатель на нашу машину
	auto curr_vm_itr = all_VMs.end();
	--curr_vm_itr;
	// Найдем хост в векторе всех хостов
	auto host_to_deploy_itr = std::find_if(
		all_hosts.begin(),
		all_hosts.end(),
		[host_1](Host host) {return host_1.id == host.id; }
	);
	// Расположим машину
	curr_vm_itr->host = &*host_to_deploy_itr;
	host_to_deploy_itr->deployed_machines.push_back(&*curr_vm_itr);


	VM vm_2;
	vm_2.id = 2;

	all_hosts_start_itr_copy = all_hosts_start_itr;
	for (int i = 2; i < 4; ++i)
	{
		vm_2.approved_hosts.push_back(&*all_hosts_start_itr_copy);
		std::advance(all_hosts_start_itr_copy, 1);
	}

	vm_2.priority = 20;
	all_VMs.push_back(vm_2);
	// Запомним указатель на нашу машину
	curr_vm_itr = all_VMs.end();
	--curr_vm_itr;
	// Найдем хост в векторе всех хостов
	host_to_deploy_itr = std::find_if(
		all_hosts.begin(),
		all_hosts.end(),
		[host_1](Host host) {return host_1.id == host.id; }
	);
	// Расположим машину
	curr_vm_itr->host = &*host_to_deploy_itr;
	host_to_deploy_itr->deployed_machines.push_back(&*curr_vm_itr);


	VM vm_3;
	vm_3.id = 3;

	all_hosts_start_itr_copy = all_hosts_start_itr;
	for (int i = 2; i < 4; ++i)
	{
		vm_3.approved_hosts.push_back(&*all_hosts_start_itr_copy);
		std::advance(all_hosts_start_itr_copy, 1);
	}

	vm_3.priority = 30;
	all_VMs.push_back(vm_3);
	// Запомним указатель на нашу машину
	curr_vm_itr = all_VMs.end();
	--curr_vm_itr;
	// Найдем хост в векторе всех хостов
	host_to_deploy_itr = std::find_if(
		all_hosts.begin(),
		all_hosts.end(),
		[host_4](Host host) {return host_4.id == host.id; }
	);
	// Расположим машину
	curr_vm_itr->host = &*host_to_deploy_itr;
	host_to_deploy_itr->deployed_machines.push_back(&*curr_vm_itr);


	VM vm_4;
	vm_4.id = 4;

	all_hosts_start_itr_copy = all_hosts_start_itr;
	for (int i = 0; i < 4; ++i)
	{
		vm_4.approved_hosts.push_back(&*all_hosts_start_itr_copy);
		std::advance(all_hosts_start_itr_copy, 1);
	}

	vm_4.priority = 40;
	all_VMs.push_back(vm_4);
	// Запомним указатель на нашу машину
	curr_vm_itr = all_VMs.end();
	--curr_vm_itr;
	// Найдем хост в векторе всех хостов
	host_to_deploy_itr = std::find_if(
		all_hosts.begin(),
		all_hosts.end(),
		[host_3](Host host) {return host_3.id == host.id; }
	);
	// Расположим машину
	curr_vm_itr->host = &*host_to_deploy_itr;
	host_to_deploy_itr->deployed_machines.push_back(&*curr_vm_itr);



	VM vm_5;
	vm_5.id = 5;

	all_hosts_start_itr_copy = all_hosts_start_itr;
	std::advance(all_hosts_start_itr_copy, 1);
	for (int i = 0; i < 3; ++i)
	{
		vm_5.approved_hosts.push_back(&*all_hosts_start_itr_copy);
		std::advance(all_hosts_start_itr_copy, 1);
	}

	vm_5.priority = 50;
	all_VMs.push_back(vm_5);
	// Запомним указатель на нашу машину
	curr_vm_itr = all_VMs.end();
	--curr_vm_itr;
	// Найдем хост в векторе всех хостов
	host_to_deploy_itr = std::find_if(
		all_hosts.begin(),
		all_hosts.end(),
		[host_3](Host host) {return host_3.id == host.id; }
	);
	// Расположим машину
	curr_vm_itr->host = &*host_to_deploy_itr;
	host_to_deploy_itr->deployed_machines.push_back(&*curr_vm_itr);





	// Создадим связь между вектором хостов вектором, 
	// который мы скормим балансировщику
	vector<Host*> all_hosts_to_vector_ptr;
	for (int i = 0; i < all_hosts.size(); ++i)
	{
		auto all_hosts_start_ptr = all_hosts.begin();
		// Так как это список, двигаемся через advance
		std::advance(all_hosts_start_ptr, i);
		all_hosts_to_vector_ptr.push_back(&*all_hosts_start_ptr);
	}


	Balancer balancer(all_hosts_to_vector_ptr);


	create_vm(all_VMs, all_hosts, balancer);

	for (Host host : all_hosts)
	{
		print_host_info(host);
	}

	//create_vm(all_VMs, all_hosts, balancer);

	//for (Host host : all_hosts)
	//{
	//	print_host_info(host);
	//}


	// секунды
	int time_period;
	cout << "Enter time period in seconds: ";
	cin >> time_period;
	std::chrono::seconds chrono_period = std::chrono::seconds(time_period);

	cout << "\nEnter amount of iterations to produce:";
	int n_iter;
	cin >> n_iter;

	for (int i = 0; i < n_iter; ++i)
	{
		if (i < 2)
		{
			set_high_load(*all_hosts.begin());
		}
		else
		{
			for (Host& host : all_hosts)
			{
				set_random_load(host);
			}
		}
		cout << "\n";
		std::cout << std::format("--- {:^30} ---\n", "Iteration " + std::to_string(i));
		cout << "\n";
		for (Host host : all_hosts)
		{
			print_host_info(host);
		}
		for (int load_type = 0; load_type < all_hosts.begin()->max_load_trigger.size(); ++load_type)
		{
			cout << "\n\n Checking load type " << load_type << "\n\n";
			balancer.check_load(time_period, load_type);
		}
		std::this_thread::sleep_for(chrono_period);
	}*/

	return 0;
}
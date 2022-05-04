// pugixml (или же xmlrpc-c) использует умные указатели, 
// которые уже перестали быть стандартом
#define _HAS_AUTO_PTR_ETC 1

// Определим систему чтобы понять тип транспорта для запросов
#ifdef _WIN32
#define	TRANSPORT_TYPE  "wininet"
#endif

#ifdef linux
#define TRANSPORT_TYPE "curl"
#endif

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

void print_host_info(Host host)
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

void print_host_info_rough(list<Host> all_hosts)
{
	for (list<Host>::iterator host = all_hosts.begin(); host != all_hosts.end(); ++host)
	{
		cout << "Host " << host->id << '\n';

		for (double load : host->max_load_trigger)
		{
			cout << " - load trigger " << load << '\n';
		}

		for (double load : host->loads)
		{
			cout << " - curr load " << load << '\n';
		}

		for (int load_time : host->max_time_trigger)
		{
			cout << " - time trigger " << load_time << '\n';
		}

		for (VM* vm : host->deployed_machines)
		{
			cout << " - VM " << vm->id
				<< " with datastore " << vm->datastore_id << '\n';
			for (string label : vm->labels)
			{
				cout << " -- " << label << '\n';
			}
			for (Host* host : vm->approved_hosts)
			{
				cout << " -- Approved host " << host->id << '\n';
			}
		}
	}
}

int main(int argc, char** argv)
{
	using namespace std;


	// Проинициализируем остальные параметры для xml запросов
	string serverUrl;
	string session_string;
	string transport_type = TRANSPORT_TYPE;
	// Времянные интервалы ожидания
	int time_period;
	std::chrono::minutes chrono_period = std::chrono::minutes(time_period);

	if (argc > 1)
	{
		if (argc != 3)
		{
			cout << "Balancer accepts 2 arguments:\n"
				<< "1) Url of a server: http://server_adress:2633/RPC2\n"
				<< "2) Session string: login:password\n"
				<< "3) Time interval between each iteration\n"
				<< endl;
			exit(1);
		}
		else
		{
			serverUrl = argv[0];
			session_string = argv[1];

			// Вводят в формате char, значит надо конвертировать по ASCII
			time_period = (int)argv[2] - '0';
			chrono_period = std::chrono::minutes(time_period);
		}
	}
	else
	{
		serverUrl = "http://192.168.92.39:2633/RPC2";
		session_string = "admin:admin";

		time_period = 1;
		chrono_period = std::chrono::minutes(time_period);
	}

	// Хранилища виртуальных машин и хостов
	list<Host> all_hosts;
	list<VM> all_VMs;
	// Вектор ссылок на хосты, которым будет пользоваться балансировщик
	vector<Host*> all_hosts_to_vector_ptr;
	
	
	update_host_vm_lists(all_hosts, all_VMs, all_hosts_to_vector_ptr, transport_type, serverUrl, session_string);

	print_host_info_rough(all_hosts);

	update_host_vm_lists(all_hosts, all_VMs, all_hosts_to_vector_ptr, transport_type, serverUrl, session_string);


	//auto vm_to_migrate = std::find_if(
	//	all_VMs.begin(),
	//	all_VMs.end(),
	//	[](VM vm) {return vm.id == 21; }
	//);
	//cout << "Want to migrate vm " << vm_to_migrate->id;
	//int migration_result = migrate_vm(&*vm_to_migrate, 0, transport_type, serverUrl, "admin:admin", false, true, 0);
	//if (migration_result != 0)
	//{
	//	cout << "migration failure\n";
	//}

	for (Host host : all_hosts)
	{
		print_host_info(host);
	}

	Balancer balancer(all_hosts_to_vector_ptr);

	cout << "Balancer's hosts:\n";
	for (Host* host : balancer.hosts)
	{
		cout << "Host " << host->id << '\n';
		print_host_info(*host);
	}


	cout << "\nEnter amount of iterations to produce:";
	int n_iter;
	cin >> n_iter;


	for (int i = 0; i < n_iter; ++i)
	{
		
		cout << "\n";
		std::cout << std::format("--- {:^30} ---\n", "Iteration " + std::to_string(i));
		cout << "\n";

		// Всегда обновляем информацию
		update_host_vm_lists(all_hosts, all_VMs, all_hosts_to_vector_ptr, transport_type, serverUrl, session_string);
		// И поставляем обновленную информацию балансировщику
		balancer.set_hosts_vector(all_hosts_to_vector_ptr);

		for (Host host : all_hosts)
		{
			print_host_info(host);
		}
		
		for (int load_type = 0; load_type < all_hosts.begin()->max_load_trigger.size(); ++load_type)
		{
			cout << "\n\n Checking load type " << load_type << "\n\n";
			balancer.check_load(time_period, load_type, transport_type, serverUrl, session_string);
		}

		// Спим
		std::this_thread::sleep_for(chrono_period);
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





	// Создадим связь между вектором хостов и вектором, 
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
// Ввод-вывод
#include <iomanip>
#include <iostream>
// Время
#include <chrono>
// Генератор случайной нагрузки
#include <random>
// Для заморозки потока
#include <thread>
// STL
#include <list>
// Наш класс
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


int main()
{
	using namespace std;


	/*
	* Test 2
	*/

	list<Host> all_hosts;
	list<VM> all_VMs;

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
			/*for (Host& host : all_hosts)
			{
				set_random_load(host);
			}*/
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
	}

	/*
	* Test 1
	*/

	/*
	vector<Host*> all_hosts;

	// ОбЪявим хосты
	Host host_1;
	host_1.id = 1;
	host_1.max_load_trigger = vector<double>{ 50, 50, 50 };
	host_1.max_time_trigger = vector<int>{ 1, 10, 10 };
	all_hosts.push_back(&host_1);

	Host host_2;
	host_2.id = 2;
	all_hosts.push_back(&host_2);

	Host host_3;
	host_3.id = 3;
	host_3.max_load_trigger = vector<double>{ 10, 20, 30 };
	host_3.max_time_trigger = vector<int>{ 10, 5, 10 };// секунды
	all_hosts.push_back(&host_3);


	Host host_4;
	host_4.id = 4;
	all_hosts.push_back(&host_4);


	// Объявим машины
	VM vm_1;
	vm_1.id = 1;
	vm_1.approved_hosts = vector<Host*>{ &host_1, &host_2 , &host_3 , &host_4 };
	vm_1.priority = 100;
	// Расположим машину
	vm_1.host = &host_1;
	host_1.deployed_machines.push_back(&vm_1);

	VM vm_2;
	vm_2.id = 2;
	vm_2.approved_hosts = vector<Host*>{ &host_3 , &host_4 };
	vm_2.priority = 10;
	// Расположим машину
	vm_2.host = &host_3;
	host_2.deployed_machines.push_back(&vm_2);

	VM vm_3;
	vm_3.id = 3;
	vm_3.approved_hosts = vector<Host*>{ &host_3 , &host_4 };
	vm_3.priority = 100;
	// Расположим машину
	vm_3.host = &host_4;
	host_4.deployed_machines.push_back(&vm_3);

	VM vm_4;
	vm_4.id = 4;
	vm_4.approved_hosts = vector<Host*>{ &host_1, &host_2 , &host_3 , &host_4 };
	vm_4.priority = 10;
	// Расположим машину
	vm_4.host = &host_1;
	host_1.deployed_machines.push_back(&vm_4);

	Balancer balancer = Balancer(all_hosts);

	// секунды
	int time_period = 1;
	std::chrono::seconds chrono_period = std::chrono::seconds(time_period);

	print_host_info(host_1);
	print_host_info(host_2);
	print_host_info(host_3);
	print_host_info(host_4);

	for (int i = 0; i < 6; ++i)
	{
		std::this_thread::sleep_for(chrono_period);
		if (i < 3)
		{
			set_high_load(host_1);
			set_high_load(host_3);
		}
		else
		{
			set_random_load(host_1);
			set_random_load(host_2);
			set_random_load(host_3);
			set_random_load(host_4);
		}
		cout << "host 1 cpu: " << host_1.loads[0] << '\n';
		cout << "host 2 cpu: " << host_2.loads[0] << '\n';
		cout << "host 3 cpu: " << host_3.loads[0] << '\n';
		cout << "host 4 cpu: " << host_4.loads[0] << '\n';
		cout << "----" << '\n';

		balancer.check_load(time_period, 0);
	}
	*/

	return 0;

/*
	Заголовочный файл с объявлением ключевых классов.

	class Host :
		Описывает необходимые данные и методы Хоста
	class VM :
		Описывает необходимые данные и методы Виртуальной машины
	class Balancer:
		Описывает весь функционал Балансировщика

		
	TODO:
	1) getter и setter балансировщика
*/
#pragma once
#include <stdexcept>
#include <algorithm>
#include <sstream>
// STL
#include <numeric>
#include <vector>
#include <set>
#include <string>


// Объявляем класс заранее, так как он используется в классе Host
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

		int priority:
			Приоритет машины

		Host host:
			Хост, на котором VM сейчас развернута

		vector<Host> approved_hosts:
			id хостов, на которых машину можно разместить
	*/

	int id;

	int priority;

	Host* host;

	std::vector<Host*> approved_hosts;
};


class Balancer
{
private:

	// Класс, чтобы отслеживать время, в течение которого хост перегружен.
	class Host_under_danger
	{
	public:

		Host_under_danger() {};


		/*
			Перегрзука оператора равенства.
			Необходима для поика элемента типа Host_under_danger
			----------
			Параметры:
			Host_under_danger const obj :
				Сравниваемый объект
		*/
		bool operator==(Host_under_danger const obj)
		{
			return id == obj.id;
		}

		bool operator==(Host_under_danger* const obj)
		{
			return id == obj->id;
		}


		/*
			Перегрзука оператора равенства.
			Необходима для поика элемента близкого типа Host
			----------
			Параметры:
			Host const obj :
				Сравниваемый объект
		*/
		bool operator==(Host const obj)
		{
			return id == obj.id;
		}

		~Host_under_danger() {};


		/*
			Данные о проблемном хосте:
			--------------------------
			int id:
				id хоста

			vector<int> time:
				 Вектор, содержащий время, в течение которого хост был под критической нагрузкой
				 По умолчанию 0.
				 1) time of critical cpu load
				 2) time of critical ram load
				 3) time of critical disk load
		*/

		int id;

		std::vector<int> time = std::vector<int>(3);
	};


public:
	/*
		Public данные Балансировщика:
		------------------------------
		vector<Host>:
			 Все хосты, находящиеся в зоне ответственности данного балансировщика

		vector<Host_under_danger> hosts_under_danger:
			 Хосты, за которыми установлено наблюдение из-за повышенной нагрузки
	*/

	std::vector<Host_under_danger> hosts_under_danger;
	std::vector<Host*> hosts;

	Balancer() {};

	~Balancer() {};

	Balancer(std::vector<Host*> hosts_in) : hosts(hosts_in) {};


	/*
		Сортировка хостов по параметру
		----------
		Параметры:
		param :
			0 для cpu
			1 для ram
			2 для disk

	*/
	void sort_hosts(std::vector<Host*>& hosts, int param)
	{
		std::sort(hosts.begin(), hosts.end(),
			[](Host* host_1, Host* host_2) {
				return host_1->loads[0] < host_2->loads[0];
			}
		);
	}


	/*
		Функция поиска Парето фронта оптимальных хостов разрешенных по заданным параметрам
		----------
		Параметры:
		param_1,2:
			0 для cpu
			1 для ram
			2 для disk

		Если один из параметров равен -1, он не важен, ищем оптимальный хост по второму параметру
		0 && 1 - cpu && ram
		1 && 2 - ram && disk
		2 && 0 - disk && cpu
		Также передаем переменную, отвечающую за множество оптимальных хостов
	*/
	void find_optimal_hosts(std::set<Host*>& pareto_set, std::vector<Host*>& approved_hosts, int param_1 = 0, int param_2 = 1)
	{
		/*if ((param_1 == -1) && (param_2 == -1))
		{
			throw std::invalid_argument("Invalid parameters provided for find_optimal_hosts function\n");
		}*/

		// Сперва отсортируем вектор наших хостов по первому параметру нагрузки
		sort_hosts(approved_hosts, param_1);

		// Добавляем во множество первый элемент отсортированного вектора
		pareto_set.insert(approved_hosts[0]);


		double load_2 = approved_hosts[0]->loads[param_2];
		// Начинаем сравнение по второму параметру
		for (int i = 1; i < approved_hosts.size(); ++i)
		{
			if (approved_hosts[i]->loads[param_2] < load_2)
			{
				pareto_set.insert(approved_hosts[i]);
				load_2 = approved_hosts[i]->loads[param_2];
			}
		}
	}


	/*
		Поиск оптимального хоста для миграции.
		----------
		Параметры:
		host_to_balance :
			нужен, чтобы оптимизатор не выдал тот же самый хост

		 load_types :
			для понимания по каким параметрам оптимизируемся,
			по умолчанию это три параметра со значениями true
	*/
	Host* find_host_for_migration(std::vector<Host*>& approved_hosts_in, Host* host_to_balance = nullptr)
	{
		/*
		 Если просят хост с мин cpu, то напрямую вызывай сортировку
		 Если параметры нагрузок не у всех одинаковые, то два варианта:
		   1) Уравнивать длину векторов, заполнять неизвестное нулями
		   2) Смотреть по пересечению параметров нагрузки
		*/

		// Сперва подкорректируем вектор разрешеных хостов так, 
		// чтобы в нем были только хосты не под критической нагрузкой
		std::vector<Host*> approved_hosts;
		for (int i = 0; i < approved_hosts_in.size(); ++i)
		{
			if (approved_hosts_in[i]->overload_risk == false)
			{
				approved_hosts.push_back(approved_hosts_in[i]);
			}
		}

		// Проверим, что у нас вообще есть хосты для миграции
		if (approved_hosts.size() == 0)
		{
			if (host_to_balance != nullptr)
			{
				std::cout << "All approved hosts for migration of VM for Host " << host_to_balance->id
					<< " are under critical load\n"
					<< "In this case, optimization will be performed on hosts with critical load\n";

			}
			else
			{
				std::cout << "All approved hosts for migration of VM"
					<< " are under critical load\n"
					<< " In this case, optimization will be performed on hosts with criticol load\n";
			}
			approved_hosts = approved_hosts_in;

		}


		// Множество оптимальных хостов
		std::set<Host*> pareto_set;
		// Вектор-маска по которой будем искать оптимальный хост
		std::vector<bool> load_types;
		// Число нагрузок
		int n_loads = approved_hosts[0]->loads.size();
		// Ответ на вопрос
		bool answer;
		std::cout << "Total amount of params " << n_loads << '\n'
			<< "Do you want to customize the choice of optimal host?\n0 - No\n1 - Yes\n";
		std::cin >> answer;
		if (answer)
		{
			bool param;
			std::cout << "Choose 1 (Yes) or 0 (No) to choose if you want to optimize by parameter or not\n";
			for (int i = 0; i < n_loads; ++i)
			{
				std::cout << "Optimize by load " << i << " ? - ";
				std::cin >> param;
				load_types.push_back(param);
				std::cout << '\n';
			}
		}
		else
			load_types = std::vector<bool>(3, true);

		// Если параметтро вообще нет
		if (std::accumulate(load_types.begin(), load_types.end(), 0) == 0)
		{
			std::cout << "No optimization params provided, program will assume that all are included\n";
			load_types = std::vector<bool>(3, true);
		}

		// Находим индекс первого ненулевого параметра в векторе (ненулевой - по нему нужно оптимизировать)
		int param_1_idx = distance(begin(load_types), find_if(begin(load_types), end(load_types), [](bool x) { return x != 0; }));

		// Если параметр только один
		if (std::accumulate(load_types.begin(), load_types.end(), 0) == 1)
		{
			sort_hosts(approved_hosts, param_1_idx);


			return approved_hosts[0];

		}


		// Итерируемся по второму параметру
		for (int i = param_1_idx + 1; i < load_types.size(); ++i)
		{
			// Нашли ненулевой второй параметр
			if (load_types[i] != 0)
			{
				// Ищем оптимальные хосты
				find_optimal_hosts(pareto_set, approved_hosts, load_types[param_1_idx], load_types[i]);
			}
		}

		// Инициализируем оптимальный хост
		Host* optimal_host = nullptr;
		if (!(*pareto_set.begin() == host_to_balance) || (host_to_balance == nullptr))
		{
			optimal_host = *pareto_set.begin();
		}
		else
		{
			if (pareto_set.size() > 1)
			{
				optimal_host = *(++pareto_set.begin());
			}
			else
			{
				std::cout << "Pareto set consists of one host and it is the same one, which we need to balance: "
					<< host_to_balance->id << '\n'
					<< "Returning same host as optimal. Please, choose approved hosts for your VMs wisely.\n";
				optimal_host = host_to_balance;
				return host_to_balance;
			}
		}

		// Инициализируем минимальное расстояние
		double min_score = 0;
		for (int i = 0; i < optimal_host->loads.size(); ++i)
		{
			if (load_types[i])
				min_score += optimal_host->loads[i];
		}

		// Удалим этот хост из множества, чтобы не обрабатывать его второй раз
		pareto_set.erase(optimal_host);

		// Среди всех оптимальных хостов ищем тот, 
		// у которого минимально расстояние
		double score;
		if (!pareto_set.empty())
		{
			for (Host* elem : pareto_set)
			{
				score = 0;

				for (int i = 0; i < elem->loads.size(); ++i)
				{
					if (load_types[i])
						score += elem->loads[i];
				}

				if ((score < min_score) && (!(elem == host_to_balance) || (host_to_balance == nullptr)))
				{
					min_score = score;
					optimal_host = elem;
				}
			}
		}
		return optimal_host;
	}


	/*
		Функция для поиска VM внутри хоста и ее последующей миграции
		----------
		Параметры:
			host_to_balance :
				Хост, с которого надо убрать VM

			load_types :
				Типы нагрузки, которые надо учесть.
				{1,1,1} - учитываем все
				{1,1,0} - учитываем только cpu и ram
				TODO: сценарий для {1,0,0} и подобных
	*/
	int balance_host(Host* host_to_balance)
	{
		std::cout << "A balancer for host " << host_to_balance->id << " is called\n";
		// Самая важная машина имеет приоритет 0 -- ее не будет трогать балансировщик.
		// Все остальные машины с ненулевым приоритетом {1,2,3,...} будут рассматриваться балансировщиком.
		int lowest_priority = 1;
		// Просто инициализация
		if (host_to_balance->deployed_machines.empty())
		{
			std::cout << "There are no VMs on host " << host_to_balance->id << " to migrate.\n Balancer ends it's work here\n";
			return 1;
		}

		if (host_to_balance->deployed_machines[0]->priority == 0)
		{
			std::cout << "Host " << host_to_balance->id << " consists of one immovable VM, can't optimize anything\n Balancer ends it's work here\n";
			return 2;
		}

		VM* machine_to_migrate = host_to_balance->deployed_machines[0];

		// Поиск VM для миграции
		for (VM* machine : host_to_balance->deployed_machines)
		{
			if ((machine->priority >= lowest_priority) && (machine->approved_hosts.size() > 1))
			{
				lowest_priority = machine->priority;
				machine_to_migrate = machine;
			}
		}
		// Поиск хоста для VM 
		Host* host_for_VM = find_host_for_migration(machine_to_migrate->approved_hosts, host_to_balance);
		// Миграция
		std::cout << "Migration of VM " << machine_to_migrate->id << " from host " << machine_to_migrate->host->id << " to host " << host_for_VM->id << "\n";
		// Удалим из списка старого хоста данную машину
		machine_to_migrate->host->deployed_machines.erase(remove(machine_to_migrate->host->deployed_machines.begin(), machine_to_migrate->host->deployed_machines.end(), machine_to_migrate));
		// Обозначим в машине ее новый хост
		machine_to_migrate->host = host_for_VM;
		// Добавим новую машину в список машин нового хоста
		host_for_VM->deployed_machines.push_back(machine_to_migrate);

		return 0;
	}


	/*
		 Ишем хосты в зоне риска.
		 ----------
		 Параметры:
			 time_period :
				с какой временной периодичностью мы это делаем
			 load_type :
				по какому параметру смотрим нагрузку
				 -- 0 для cpu
				 -- 1 для ram
				 -- 2 для disk
	*/
	void check_load(int time_period, int load_type)
	{
		for (int i = 0; i < hosts.size(); i++)
		{
			// Если нагрузка не меньше критической, а значение критической нагрузки задано как положительное ненулевое число,
			// вносим хост в список кандидатов на балансировку
			if ((hosts[i]->loads[load_type] >= hosts[i]->max_load_trigger[load_type]) && (hosts[i]->max_load_trigger[load_type] > 0))
			{
				if (hosts[i]->overload_risk == true)
				{
					// Найдем нужный хост
					Host_under_danger& problematic_host = *std::find_if(
						hosts_under_danger.begin(),
						hosts_under_danger.end(),
						[target_id = hosts[i]->id](Host_under_danger x)
					{ return x.id == target_id; });

					// Увеличиваем счетчик времени, в течение которого хост испытывал критическую нагрузку
					problematic_host.time[load_type] += time_period;
					std::cout << "Host " << problematic_host.id << " is under critical load for " << problematic_host.time[load_type] << " seconds\n";

					// Если времени работы под критической нагрузкой прошло больше, чем дозволено
					if (problematic_host.time[load_type] >= hosts[i]->max_time_trigger[load_type])
					{
						// Вызываем балансировщик хоста
						int balance_host_result = balance_host(hosts[i]);

						if (balance_host_result != 2)
						{
							hosts[i]->overload_risk = false;
							hosts_under_danger.erase(std::remove(hosts_under_danger.begin(), hosts_under_danger.end(), problematic_host), hosts_under_danger.end());
						}
					}


				}
				// Иначе заносим хост в список наблюдаемых, ведь его там еще не было
				else
				{
					std::cout << "Host " << hosts[i]->id << " is under balancer's control now\n";
					Host_under_danger new_host;
					new_host.id = hosts[i]->id;
					new_host.time[load_type] += time_period;
					std::cout << "Time for Host " << hosts[i]->id << " load type " << load_type << " is increased by " << time_period << '\n';
					hosts[i]->overload_risk = true;
					hosts_under_danger.push_back(new_host);
				}

			}
			else
			{
				// Если хост уже под наблюдением
				if (hosts[i]->overload_risk)
				{
					bool other_ovarload_occured = false;
					for (double l = 0; l < hosts[i]->loads.size(); ++l)
					{
						if (hosts[i]->loads[l] >= hosts[i]->max_load_trigger[l])
							other_ovarload_occured = true;
					}

					// Если есть перегруз в другой категории, то не удаляем
					if (!other_ovarload_occured)
					{
						hosts[i]->overload_risk = false;
						std::cout << "Host " << hosts[i]->id << " is ok. No need for balancer's control\n";
						// Найдем нужный хост
						Host_under_danger& host_to_erase = *std::find_if
						(
							hosts_under_danger.begin(),
							hosts_under_danger.end(),
							[target_id = hosts[i]->id](Host_under_danger x)
						{ return x.id == target_id; }
						);

						hosts_under_danger.erase(std::remove(hosts_under_danger.begin(), hosts_under_danger.end(), host_to_erase), hosts_under_danger.end());
					}

				}
			}
		}
	}
};

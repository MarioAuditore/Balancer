/*
	������������ ���� � ����������� �������� �������.

	class Host :
		��������� ����������� ������ � ������ �����
	class VM :
		��������� ����������� ������ � ������ ����������� ������
	class Balancer:
		��������� ���� ���������� ��������������

		
	TODO:
	1) getter � setter ��������������
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


// ��������� ����� �������, ��� ��� �� ������������ � ������ Host
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

		int priority:
			��������� ������

		Host host:
			����, �� ������� VM ������ ����������

		vector<Host> approved_hosts:
			id ������, �� ������� ������ ����� ����������
	*/

	int id;

	int priority;

	Host* host;

	std::vector<Host*> approved_hosts;
};


class Balancer
{
private:

	// �����, ����� ����������� �����, � ������� �������� ���� ����������.
	class Host_under_danger
	{
	public:

		Host_under_danger() {};


		/*
			���������� ��������� ���������.
			���������� ��� ����� �������� ���� Host_under_danger
			----------
			���������:
			Host_under_danger const obj :
				������������ ������
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
			���������� ��������� ���������.
			���������� ��� ����� �������� �������� ���� Host
			----------
			���������:
			Host const obj :
				������������ ������
		*/
		bool operator==(Host const obj)
		{
			return id == obj.id;
		}

		~Host_under_danger() {};


		/*
			������ � ���������� �����:
			--------------------------
			int id:
				id �����

			vector<int> time:
				 ������, ���������� �����, � ������� �������� ���� ��� ��� ����������� ���������
				 �� ��������� 0.
				 1) time of critical cpu load
				 2) time of critical ram load
				 3) time of critical disk load
		*/

		int id;

		std::vector<int> time = std::vector<int>(3);
	};


public:
	/*
		Public ������ ��������������:
		------------------------------
		vector<Host>:
			 ��� �����, ����������� � ���� ��������������� ������� ��������������

		vector<Host_under_danger> hosts_under_danger:
			 �����, �� �������� ����������� ���������� ��-�� ���������� ��������
	*/

	std::vector<Host_under_danger> hosts_under_danger;
	std::vector<Host*> hosts;

	Balancer() {};

	~Balancer() {};

	Balancer(std::vector<Host*> hosts_in) : hosts(hosts_in) {};


	/*
		���������� ������ �� ���������
		----------
		���������:
		param :
			0 ��� cpu
			1 ��� ram
			2 ��� disk

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
		������� ������ ������ ������ ����������� ������ ����������� �� �������� ����������
		----------
		���������:
		param_1,2:
			0 ��� cpu
			1 ��� ram
			2 ��� disk

		���� ���� �� ���������� ����� -1, �� �� �����, ���� ����������� ���� �� ������� ���������
		0 && 1 - cpu && ram
		1 && 2 - ram && disk
		2 && 0 - disk && cpu
		����� �������� ����������, ���������� �� ��������� ����������� ������
	*/
	void find_optimal_hosts(std::set<Host*>& pareto_set, std::vector<Host*>& approved_hosts, int param_1 = 0, int param_2 = 1)
	{
		/*if ((param_1 == -1) && (param_2 == -1))
		{
			throw std::invalid_argument("Invalid parameters provided for find_optimal_hosts function\n");
		}*/

		// ������ ����������� ������ ����� ������ �� ������� ��������� ��������
		sort_hosts(approved_hosts, param_1);

		// ��������� �� ��������� ������ ������� ���������������� �������
		pareto_set.insert(approved_hosts[0]);


		double load_2 = approved_hosts[0]->loads[param_2];
		// �������� ��������� �� ������� ���������
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
		����� ������������ ����� ��� ��������.
		----------
		���������:
		host_to_balance :
			�����, ����� ����������� �� ����� ��� �� ����� ����

		 load_types :
			��� ��������� �� ����� ���������� ��������������,
			�� ��������� ��� ��� ��������� �� ���������� true
	*/
	Host* find_host_for_migration(std::vector<Host*>& approved_hosts_in, Host* host_to_balance = nullptr)
	{
		/*
		 ���� ������ ���� � ��� cpu, �� �������� ������� ����������
		 ���� ��������� �������� �� � ���� ����������, �� ��� ��������:
		   1) ���������� ����� ��������, ��������� ����������� ������
		   2) �������� �� ����������� ���������� ��������
		*/

		// ������ ��������������� ������ ���������� ������ ���, 
		// ����� � ��� ���� ������ ����� �� ��� ����������� ���������
		std::vector<Host*> approved_hosts;
		for (int i = 0; i < approved_hosts_in.size(); ++i)
		{
			if (approved_hosts_in[i]->overload_risk == false)
			{
				approved_hosts.push_back(approved_hosts_in[i]);
			}
		}

		// ��������, ��� � ��� ������ ���� ����� ��� ��������
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


		// ��������� ����������� ������
		std::set<Host*> pareto_set;
		// ������-����� �� ������� ����� ������ ����������� ����
		std::vector<bool> load_types;
		// ����� ��������
		int n_loads = approved_hosts[0]->loads.size();
		// ����� �� ������
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

		// ���� ���������� ������ ���
		if (std::accumulate(load_types.begin(), load_types.end(), 0) == 0)
		{
			std::cout << "No optimization params provided, program will assume that all are included\n";
			load_types = std::vector<bool>(3, true);
		}

		// ������� ������ ������� ���������� ��������� � ������� (��������� - �� ���� ����� ��������������)
		int param_1_idx = distance(begin(load_types), find_if(begin(load_types), end(load_types), [](bool x) { return x != 0; }));

		// ���� �������� ������ ����
		if (std::accumulate(load_types.begin(), load_types.end(), 0) == 1)
		{
			sort_hosts(approved_hosts, param_1_idx);


			return approved_hosts[0];

		}


		// ����������� �� ������� ���������
		for (int i = param_1_idx + 1; i < load_types.size(); ++i)
		{
			// ����� ��������� ������ ��������
			if (load_types[i] != 0)
			{
				// ���� ����������� �����
				find_optimal_hosts(pareto_set, approved_hosts, load_types[param_1_idx], load_types[i]);
			}
		}

		// �������������� ����������� ����
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

		// �������������� ����������� ����������
		double min_score = 0;
		for (int i = 0; i < optimal_host->loads.size(); ++i)
		{
			if (load_types[i])
				min_score += optimal_host->loads[i];
		}

		// ������ ���� ���� �� ���������, ����� �� ������������ ��� ������ ���
		pareto_set.erase(optimal_host);

		// ����� ���� ����������� ������ ���� ���, 
		// � �������� ���������� ����������
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
		������� ��� ������ VM ������ ����� � �� ����������� ��������
		----------
		���������:
			host_to_balance :
				����, � �������� ���� ������ VM

			load_types :
				���� ��������, ������� ���� ������.
				{1,1,1} - ��������� ���
				{1,1,0} - ��������� ������ cpu � ram
				TODO: �������� ��� {1,0,0} � ��������
	*/
	int balance_host(Host* host_to_balance)
	{
		std::cout << "A balancer for host " << host_to_balance->id << " is called\n";
		// ����� ������ ������ ����� ��������� 0 -- �� �� ����� ������� �������������.
		// ��� ��������� ������ � ��������� ����������� {1,2,3,...} ����� ��������������� ���������������.
		int lowest_priority = 1;
		// ������ �������������
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

		// ����� VM ��� ��������
		for (VM* machine : host_to_balance->deployed_machines)
		{
			if ((machine->priority >= lowest_priority) && (machine->approved_hosts.size() > 1))
			{
				lowest_priority = machine->priority;
				machine_to_migrate = machine;
			}
		}
		// ����� ����� ��� VM 
		Host* host_for_VM = find_host_for_migration(machine_to_migrate->approved_hosts, host_to_balance);
		// ��������
		std::cout << "Migration of VM " << machine_to_migrate->id << " from host " << machine_to_migrate->host->id << " to host " << host_for_VM->id << "\n";
		// ������ �� ������ ������� ����� ������ ������
		machine_to_migrate->host->deployed_machines.erase(remove(machine_to_migrate->host->deployed_machines.begin(), machine_to_migrate->host->deployed_machines.end(), machine_to_migrate));
		// ��������� � ������ �� ����� ����
		machine_to_migrate->host = host_for_VM;
		// ������� ����� ������ � ������ ����� ������ �����
		host_for_VM->deployed_machines.push_back(machine_to_migrate);

		return 0;
	}


	/*
		 ���� ����� � ���� �����.
		 ----------
		 ���������:
			 time_period :
				� ����� ��������� �������������� �� ��� ������
			 load_type :
				�� ������ ��������� ������� ��������
				 -- 0 ��� cpu
				 -- 1 ��� ram
				 -- 2 ��� disk
	*/
	void check_load(int time_period, int load_type)
	{
		for (int i = 0; i < hosts.size(); i++)
		{
			// ���� �������� �� ������ �����������, � �������� ����������� �������� ������ ��� ������������� ��������� �����,
			// ������ ���� � ������ ���������� �� ������������
			if ((hosts[i]->loads[load_type] >= hosts[i]->max_load_trigger[load_type]) && (hosts[i]->max_load_trigger[load_type] > 0))
			{
				if (hosts[i]->overload_risk == true)
				{
					// ������ ������ ����
					Host_under_danger& problematic_host = *std::find_if(
						hosts_under_danger.begin(),
						hosts_under_danger.end(),
						[target_id = hosts[i]->id](Host_under_danger x)
					{ return x.id == target_id; });

					// ����������� ������� �������, � ������� �������� ���� ��������� ����������� ��������
					problematic_host.time[load_type] += time_period;
					std::cout << "Host " << problematic_host.id << " is under critical load for " << problematic_host.time[load_type] << " seconds\n";

					// ���� ������� ������ ��� ����������� ��������� ������ ������, ��� ���������
					if (problematic_host.time[load_type] >= hosts[i]->max_time_trigger[load_type])
					{
						// �������� ������������� �����
						int balance_host_result = balance_host(hosts[i]);

						if (balance_host_result != 2)
						{
							hosts[i]->overload_risk = false;
							hosts_under_danger.erase(std::remove(hosts_under_danger.begin(), hosts_under_danger.end(), problematic_host), hosts_under_danger.end());
						}
					}


				}
				// ����� ������� ���� � ������ �����������, ���� ��� ��� ��� �� ����
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
				// ���� ���� ��� ��� �����������
				if (hosts[i]->overload_risk)
				{
					bool other_ovarload_occured = false;
					for (double l = 0; l < hosts[i]->loads.size(); ++l)
					{
						if (hosts[i]->loads[l] >= hosts[i]->max_load_trigger[l])
							other_ovarload_occured = true;
					}

					// ���� ���� �������� � ������ ���������, �� �� �������
					if (!other_ovarload_occured)
					{
						hosts[i]->overload_risk = false;
						std::cout << "Host " << hosts[i]->id << " is ok. No need for balancer's control\n";
						// ������ ������ ����
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

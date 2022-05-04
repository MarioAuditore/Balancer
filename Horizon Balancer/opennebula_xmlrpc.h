#pragma once

/* - стандартные библиотеки -*/
#include <cassert>
#include <cstdlib>
#include <iostream>
// Для работы со строками
#include <string>
#include <sstream>
// Для парсинга конфигурационного json файла с леблами
#include "json_parser.h"


/* ------ xmlrpc-c ------ */
// базовые модули
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client.hpp>
// модуль для создания xml строки
#include <xmlrpc-c/xml.hpp>
// модуль для парсинга xml
#include "pugixml.hpp"

using namespace std;


/*
    Создание запроса xml и прием ответа

    Params:
    -------
    transport_type:
        тип трансорта для передачи запроса
        curl    - Linux и остальные
        wininet - средствами windows

    method_string:
        строка с методом типа "one.methodname"

    serverUrl:
        url сервера "http://ip_adress:2633/RPC2"

    sampleAddParms:
        параметры запроса

    Return:
    -------
    response: xmlrpc_c::value
        ответ на запрос, который надо парсить

*/
xmlrpc_c::value generate_call_and_response(string transport_type, string method_string, string serverUrl, xmlrpc_c::paramList sampleAddParms)
{
    try
    {
        /* Выбор технологии для передачи запроса */
        // curl используется практически везде
        if (transport_type == "curl")
        {
            xmlrpc_c::clientXmlTransport_curl myTransport;
            xmlrpc_c::client_xml myClient(&myTransport);
            xmlrpc_c::carriageParm_curl0 myCarriageParm(serverUrl);

            string const methodName(method_string);
            xmlrpc_c::rpcPtr rpc1P(methodName, sampleAddParms);

            rpc1P->call(&myClient, &myCarriageParm);
            assert(rpc1P->isFinished());

            xmlrpc_c::value response = rpc1P->getResult();
            return response;
        }
        // wininet встроен в Windows
        if (transport_type == "wininet")
        {
            xmlrpc_c::clientXmlTransport_wininet myTransport;
            xmlrpc_c::client_xml myClient(&myTransport);
            xmlrpc_c::carriageParm_wininet0 myCarriageParm(serverUrl);

            string const methodName(method_string);
            xmlrpc_c::rpcPtr rpc1P(methodName, sampleAddParms);

            rpc1P->call(&myClient, &myCarriageParm);
            assert(rpc1P->isFinished());

            xmlrpc_c::value response = rpc1P->getResult();
            return response;
        }
    }
    catch (exception const& e) {
        cerr << "Client threw error: " << e.what() << endl;
    }
    catch (...) {
        cerr << "Client threw unexpected error." << endl;
    }
}


// TODO: сделать возвращение вектора
void get_hostpool_load(string transport_type, string serverUrl, string session_string = "admin:admin")
{
    string method_string = "one.hostpool.info";

    xmlrpc_c::paramList xml_params_list;
    xml_params_list.add(xmlrpc_c::value_string(session_string));

    xmlrpc_c::value response = generate_call_and_response(transport_type, method_string, serverUrl, xml_params_list);
    xmlrpc_c::rpcOutcome outcome(response);

    string responseXml;
    xmlrpc_c::value result;
    pugi::xml_document doc;

    xmlrpc_c::xml::generateResponse(outcome, &responseXml);
    xmlrpc_c::xml::parseSuccessfulResponse(responseXml, &result);

    xmlrpc_c::value_array response_array(result);
    vector<xmlrpc_c::value> const response_array_vector_values(response_array.vectorValueValue());


    if (xmlrpc_c::value_boolean(response_array_vector_values[0]))
    {
        string xml_to_parse(static_cast<string>(xmlrpc_c::value_string(response_array_vector_values[1])));
        pugi::xml_parse_result res = doc.load_string(xml_to_parse.c_str());

        for (pugi::xml_node tool = doc.first_child().first_child(); tool; tool = tool.next_sibling())
        {
            //std::cout << "Node " << tool.name() << ":";

            cout << tool.name() << " ID " << tool.child("ID").text().as_int() << '\n';

            cout << "--- CPU " << tool.child("HOST_SHARE").child("USED_CPU").text().as_double() << '\n';
            cout << "--- RAM " << tool.child("HOST_SHARE").child("USED_MEM").text().as_double() << '\n';
            cout << "--- DISK " << tool.child("HOST_SHARE").child("USED_DISK").text().as_double() << '\n';

            pugi::xml_node host_vms = tool.child("VMS");

            for (pugi::xml_node vm_node : host_vms.children())
            {
                cout << "==== With deployed VM " << vm_node.text().as_int() << '\n';
            }

            cout << "===================";
            std::cout << std::endl;
        }
    }
    else
    {
        cout << "get_hostpool_load failed:\n"
            << static_cast<string>(xmlrpc_c::value_string(response_array_vector_values[1]))
            << "Error code: " << xmlrpc_c::value_int(response_array_vector_values[2])
            << "ID of the object that caused the error: " << xmlrpc_c::value_int(response_array_vector_values[3])
            << endl;
    }
}


void set_vm_info_via_xml(VM* vm, string transport_type, string serverUrl, string session_string = "admin:admin")
{
    string method_string = "one.vm.info";

    xmlrpc_c::paramList xml_params_list;
    xml_params_list.add(xmlrpc_c::value_string(session_string));
    xml_params_list.add(xmlrpc_c::value_int(vm->id));

    xmlrpc_c::value response = generate_call_and_response(transport_type, method_string, serverUrl, xml_params_list);
    xmlrpc_c::rpcOutcome outcome(response);

    string responseXml;
    xmlrpc_c::value result;
    pugi::xml_document doc;

    xmlrpc_c::xml::generateResponse(outcome, &responseXml);
    xmlrpc_c::xml::parseSuccessfulResponse(responseXml, &result);

    xmlrpc_c::value_array response_array(result);
    vector<xmlrpc_c::value> const response_array_vector_values(response_array.vectorValueValue());

    // Преобразуем ответ в строку
    string xml_to_parse(static_cast<string>(xmlrpc_c::value_string(response_array_vector_values[1])));
    pugi::xml_parse_result res = doc.load_string(xml_to_parse.c_str());

    // Добавим лейблы в класс
    std::vector<std::string> vector_labels;
    std::string vm_labels_string = doc.child("VM").child("USER_TEMPLATE").child("LABELS").text().as_string();
    vm->datastore_id = doc.child("VM").child("TEMPLATE").child("DISK").child("DATASTORE_ID").text().as_int();


    // Если лейблов несколько, необходимо разбить строку
    if (vm_labels_string.find(',') != std::string::npos)
    {
        std::stringstream vm_labels_stream(vm_labels_string);
        std::string label;
        while (std::getline(vm_labels_stream, label, ','))
        {
            vector_labels.push_back(label);
        }
    }
    // Если лейбл один, то просто добавляем его
    else
    {
        vector_labels.push_back(vm_labels_string);
    }

    vm->labels = vector_labels;
}

void update_vm_info(VM* vm_ptr, Host* host_for_deploy, std::list<Host>& all_hosts, json labels_config, std::string transport_type, std::string serverUrl, std::string session_string = "admin:admin")
{
    
    // Зафиксируем лейбл машины
    set_vm_info_via_xml(vm_ptr, transport_type, serverUrl, session_string);

    bool no_host_restrictions = true;

    for (string label : vm_ptr->labels)
    {
        if (!labels_config[label].is_null())
        {
            // Зафиксируем приоритет
            if (labels_config[label]["priority"] < vm_ptr->priority)
            {
                vm_ptr->priority = labels_config[label]["priority"];
            }

            // Обновим ограничения на нагрузку хоста
            for (int i = 0; i < labels_config[label]["max_load"].size(); ++i)
            {
                if (host_for_deploy->max_load_trigger[i] > labels_config[label]["max_load"][i])
                {
                    host_for_deploy->max_load_trigger[i] = labels_config[label]["max_load"][i];
                }
            }

            // Обновим ограничения на время нагрузки хоста
            for (int i = 0; i < labels_config[label]["max_time"].size(); ++i)
            {
                
                if (host_for_deploy->max_time_trigger[i] > labels_config[label]["max_time"][i])
                {
                    host_for_deploy->max_time_trigger[i] = labels_config[label]["max_time"][i];
                }
            }

            // Среди списка лейблов есть ограничения?
            if (labels_config[label]["hosts"] != -1)
            {
                no_host_restrictions = false;

                for (int host_id : labels_config[label]["hosts"])
                {
                    // Есть ли хост в списке дозволенных внутри самой машины
                    auto approved_host_in_vm = std::find_if(
                        vm_ptr->approved_hosts.begin(),
                        vm_ptr->approved_hosts.end(),
                        [id = host_id](Host* host) {return id == host->id; }
                    );

                    // Если не нашелся, то добавим
                    if (approved_host_in_vm == vm_ptr->approved_hosts.end())
                    {
                        // Найдем его среди списка всех хостов
                        auto approved_host_list = std::find_if(
                            all_hosts.begin(),
                            all_hosts.end(),
                            [id = host_id](Host host) {return id == host.id; }
                        );

                        // Если нашли, то смело добавляем
                        if (approved_host_list != all_hosts.end())
                        {
                            vm_ptr->approved_hosts.push_back(&*approved_host_list);
                        }
                    }
                }
            }
        }
    }

    // Если по итогу ограничений на хосты не было, добавим их всех
    if (no_host_restrictions)
    {
        vm_ptr->approved_hosts.clear();

        // all_hosts это список и по нему нужно двигаться через advanced
        for (int i = 0; i < all_hosts.size(); ++i)
        {
            auto all_hosts_start_ptr = all_hosts.begin();
            // Так как это список, двигаемся через advance
            std::advance(all_hosts_start_ptr, i);
            vm_ptr->approved_hosts.push_back(&*all_hosts_start_ptr);
        }

        /*for (Host host : all_hosts)
        {
            vm_ptr->approved_hosts.push_back(&host);
        }*/
    }
}

VM* create_vm_class(int id, Host* host_for_deploy, std::list<Host>& all_hosts, std::list<VM>& all_VMs, json labels_config, std::string transport_type, std::string serverUrl, std::string session_string = "admin:admin")
{
    // Создаем
    VM new_vm_tmp = VM();
    new_vm_tmp.id = id;

    // Сразу добавим в список и будем работать с нми
    all_VMs.push_back(new_vm_tmp);
    auto buf_iter = all_VMs.end();
    --buf_iter;
    VM* our_new_vm_ptr = &*(buf_iter);

    // Зафиксируем на каком она хосте
    our_new_vm_ptr->host = host_for_deploy;

    // Заполним информацию о машине
    update_vm_info(our_new_vm_ptr, host_for_deploy, all_hosts, labels_config, transport_type, serverUrl, session_string);

    return our_new_vm_ptr;
}


void update_host_vm_lists(std::list<Host>& all_hosts, std::list<VM>& all_VMs, std::vector<Host*>& all_hosts_to_vector_ptr, std::string transport_type, std::string serverUrl, std::string session_string = "admin:admin", std::string config_path = "labels_config.json")
{
    // Инициализация
    string method_string = "one.hostpool.info";
    string responseXml;
    xmlrpc_c::value result;
    pugi::xml_document doc;

    // Итератор для обновления списка
    std::list<Host>::iterator curr_host_itr;
    std::list<VM>::iterator curr_vm_itr;
    std::vector<VM*>::iterator deployed_vm_itr;

    // Для просмотра правил, описанных в леблах машин
    json labels_config = parse_json_config(config_path);

    // параметры запроса
    xmlrpc_c::paramList xml_params_list;
    xml_params_list.add(xmlrpc_c::value_string(session_string));

    xmlrpc_c::value response = generate_call_and_response(transport_type, method_string, serverUrl, xml_params_list);
    xmlrpc_c::rpcOutcome outcome(response);
    
    xmlrpc_c::xml::generateResponse(outcome, &responseXml);
    xmlrpc_c::xml::parseSuccessfulResponse(responseXml, &result);

    xmlrpc_c::value_array response_array(result);
    vector<xmlrpc_c::value> const response_array_vector_values(response_array.vectorValueValue());

    // Если запрос успешен
    if (xmlrpc_c::value_boolean(response_array_vector_values[0]))
    {
        string xml_to_parse(static_cast<string>(xmlrpc_c::value_string(response_array_vector_values[1])));
        pugi::xml_parse_result res = doc.load_string(xml_to_parse.c_str());

        for (pugi::xml_node tool = doc.first_child().first_child(); tool; tool = tool.next_sibling())
        {
            
            //cout << tool.name() << " ID " << tool.child("ID").text().as_int() << '\n';

            // Найдем в списке хост на текущей итерации
            curr_host_itr = std::find_if(
                all_hosts.begin(),
                all_hosts.end(),
                [id = tool.child("ID").text().as_int()](Host host) {return id == host.id; }
            );
            
            // Если хост в списке нашелся
            if (curr_host_itr != all_hosts.end())
            {
                // Обновим его нагрузки
                curr_host_itr->loads[0] = tool.child("HOST_SHARE").child("USED_CPU").text().as_double() / tool.child("HOST_SHARE").child("TOTAL_CPU").text().as_double();
                curr_host_itr->loads[1] = tool.child("HOST_SHARE").child("USED_MEM").text().as_double() / tool.child("HOST_SHARE").child("TOTAL_MEM").text().as_double();
                curr_host_itr->loads[2] = tool.child("HOST_SHARE").child("USED_DISK").text().as_double() / tool.child("HOST_SHARE").child("MAX_DISK").text().as_double();

                // Обновим информацию о машинах
                for (pugi::xml_node vm_node : tool.child("VMS").children())
                {
                    // Проверим существование такой машины
                    curr_vm_itr = std::find_if(
                        all_VMs.begin(),
                        all_VMs.end(),
                        [id = vm_node.text().as_int()](VM vm) {return id == vm.id; }
                    );

                    // Если такой машины нет в списке, создадим
                    if (curr_vm_itr == all_VMs.end())
                    {
                        // Создадим ее
                        VM* our_new_vm_ptr = create_vm_class(vm_node.text().as_int(), &*curr_host_itr, all_hosts, all_VMs, labels_config, transport_type, serverUrl, session_string);
                        // Добавим теперь ее к хосту
                        curr_host_itr->deployed_machines.push_back(our_new_vm_ptr);
                    }
                    else
                    {   
                        // Проверим наличие машины на хосте
                        deployed_vm_itr = std::find_if(
                            curr_host_itr->deployed_machines.begin(),
                            curr_host_itr->deployed_machines.end(),
                            [id = vm_node.text().as_int()](VM* vm) {return id == vm->id; }
                        );

                        // Если такой виртальной машины мы не обнаружили на хосте из нашего списка, то добавим ее 
                        if (deployed_vm_itr == curr_host_itr->deployed_machines.end())
                        {
                            // Обновим информацию о машине
                            update_vm_info(&*curr_vm_itr, &*curr_host_itr, all_hosts, labels_config, transport_type, serverUrl, session_string);
                            // Добавим уже свежую
                            curr_host_itr->deployed_machines.push_back(&*curr_vm_itr);
                            curr_vm_itr->host = &*curr_host_itr;
                        }
                        else
                        {
                            // Обновим информацию о машине
                            update_vm_info(&*curr_vm_itr, &*curr_host_itr, all_hosts, labels_config, transport_type, serverUrl, session_string);
                        }
                    }
                }
            }
            // Если рассматриваемого хоста в списке еще нет
            else
            {
                // Создадим его
                Host host_tmp(3);
                host_tmp.id = tool.child("ID").text().as_int();
                host_tmp.loads[0] = tool.child("HOST_SHARE").child("USED_CPU").text().as_double() / tool.child("HOST_SHARE").child("TOTAL_CPU").text().as_double();
                host_tmp.loads[1] = tool.child("HOST_SHARE").child("USED_MEM").text().as_double() / tool.child("HOST_SHARE").child("TOTAL_MEM").text().as_double();
                host_tmp.loads[2] = tool.child("HOST_SHARE").child("USED_DISK").text().as_double() / tool.child("HOST_SHARE").child("MAX_DISK").text().as_double();

                cout << "ram " << tool.child("HOST_SHARE").child("USED_MEM").text().as_double() << " out of " << tool.child("HOST_SHARE").child("TOTAL_MEM").text().as_double() << endl;

                // Добавим в общий список
                all_hosts.push_back(host_tmp);
                auto curr_host_itr = all_hosts.end();
                --curr_host_itr;

                // Добавим его в балансировщик
                all_hosts_to_vector_ptr.push_back(&*curr_host_itr);

                // Добавим ему информацию о его виртуальных машинах
                for (pugi::xml_node vm_node : tool.child("VMS").children())
                {
                    //cout << "==== With deployed VM " << vm_node.text().as_int() << '\n';

                    // Поищем виртуальную машину в списке всех машин
                    curr_vm_itr = std::find_if(
                        all_VMs.begin(),
                        all_VMs.end(),
                        [id = vm_node.text().as_int()](VM vm) {return id == vm.id; }
                    );

                    // Если VM не существует
                    if (curr_vm_itr == all_VMs.end())
                    {
                        // Создадим ее
                        VM* our_new_vm_ptr = create_vm_class(vm_node.text().as_int(), &*curr_host_itr, all_hosts, all_VMs, labels_config, transport_type, serverUrl, session_string);
                        // Добавим теперь ее к хосту
                        curr_host_itr->deployed_machines.push_back(our_new_vm_ptr);
                    }
                    // Если VM нашлась
                    else
                    {
                        // Обновим информацию о ней
                        update_vm_info(&*curr_vm_itr, &*curr_host_itr, all_hosts, labels_config, transport_type, serverUrl, session_string);
                        // Добавим в хост уже свежую весрию
                        curr_host_itr->deployed_machines.push_back(&*curr_vm_itr);
                        curr_vm_itr->host = &*curr_host_itr;
                    }
                }
            }

            //cout << "--- CPU " << tool.child("HOST_SHARE").child("USED_CPU").text().as_double() << '\n';
            //cout << "--- RAM " << tool.child("HOST_SHARE").child("USED_MEM").text().as_double() << '\n';
            //cout << "--- DISK " << tool.child("HOST_SHARE").child("USED_DISK").text().as_double() << '\n';

            //cout << "===================";
            //std::cout << std::endl;
        }
    }
    else
    {
        cout << "get_hostpool_load failed:\n"
            << static_cast<string>(xmlrpc_c::value_string(response_array_vector_values[1]))
            << "Error code: " << xmlrpc_c::value_int(response_array_vector_values[2])
            << "ID of the object that caused the error: " << xmlrpc_c::value_int(response_array_vector_values[3])
            << endl;
    }
}


void print_xml_response(string responseXml)
{
    xmlrpc_c::value result;
    xmlrpc_c::xml::parseSuccessfulResponse(responseXml, &result);
    xmlrpc_c::value_array response_array(result);
    vector<xmlrpc_c::value> const response_array_vector_values(response_array.vectorValueValue());

    cout << "Array size is " << response_array_vector_values.size() << endl;
    cout << "Element 1\n" << xmlrpc_c::value_boolean(response_array_vector_values[0]) << '\n';
    cout << "Element 2\n" << static_cast<string>(xmlrpc_c::value_string(response_array_vector_values[1])) << '\n';
    cout << "Element 3\n" << xmlrpc_c::value_int(response_array_vector_values[2]) << '\n';
}


int migrate_vm(VM* vm, int target_host_id, string transport_type, string serverUrl, string session_string = "admin:admin", bool live_migration = true, bool host_capacity = true, int migration_type = 0)
{
    string method_string = "one.vm.migrate";

    xmlrpc_c::paramList xml_params_list;
    xml_params_list.add(xmlrpc_c::value_string(session_string));
    xml_params_list.add(xmlrpc_c::value_int(vm->id));
    xml_params_list.add(xmlrpc_c::value_int(target_host_id));
    xml_params_list.add(xmlrpc_c::value_boolean(live_migration));
    xml_params_list.add(xmlrpc_c::value_boolean(host_capacity));
    xml_params_list.add(xmlrpc_c::value_int(vm->datastore_id));
    xml_params_list.add(xmlrpc_c::value_int(migration_type));

    xmlrpc_c::value response = generate_call_and_response(transport_type, method_string, serverUrl, xml_params_list);

    xmlrpc_c::rpcOutcome outcome(response);

    string responseXml;
    xmlrpc_c::xml::generateResponse(outcome, &responseXml);
    xmlrpc_c::value result;
    xmlrpc_c::xml::parseSuccessfulResponse(responseXml, &result);

    xmlrpc_c::value_array response_array(result);
    vector<xmlrpc_c::value> const response_array_vector_values(response_array.vectorValueValue());

    if (xmlrpc_c::value_boolean(response_array_vector_values[0]))
    {
        cout << "\nSuccess " << response_array_vector_values.size();
        cout << "\nVM id: "
            << xmlrpc_c::value_int(response_array_vector_values[1]);
        cout << "\n";
        
        return 0;
    }
    else
    {
        cout << "migrate_vm failed for VM "
            << vm->id
            << " with target host "
            << target_host_id
            << " and DataStore "
            << vm->datastore_id << ":\n"
            << xmlrpc_c::value_boolean(response_array_vector_values[0])
            << "\n - Error string: "
            << static_cast<string>(xmlrpc_c::value_string(response_array_vector_values[1]))
            << "\n - Error code: "
            << xmlrpc_c::value_int(response_array_vector_values[2])
            << "\n - Host that caused error: "
            << xmlrpc_c::value_int(response_array_vector_values[3])
            << endl;

        return 1;
    }

}


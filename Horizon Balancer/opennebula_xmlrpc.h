#pragma once

/* - стандартные библиотеки -*/
#include <cassert>
#include <cstdlib>
#include <string>
#include <iostream>


/* ------ xmlrpc-c ------ */
// базовые модули
#include <xmlrpc-c/girerr.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/client.hpp>
// модуль дл€ создани€ xml строки
#include <xmlrpc-c/xml.hpp>
// модуль дл€ парсинга xml
#include "pugixml.hpp"

using namespace std;


/*
    —оздание запроса xml и прием ответа

    Params:
    -------
    transport_type:
        тип трансорта дл€ передачи запроса
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
        /* ¬ыбор технологии дл€ передачи запроса */
        // curl используетс€ практически везде
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


// TODO: помен€ть всю логику с минимизации нагрузки на максимизацию свободных ресурсов
void update_host_vm_lists(std::list<Host>& all_hosts, std::list<VM>& all_VMs, std::string transport_type, std::string serverUrl, std::string session_string = "admin:admin")
{
    // »нициализаци€
    string method_string = "one.hostpool.info";
    string responseXml;
    xmlrpc_c::value result;
    pugi::xml_document doc;

    // »тератор дл€ обновлени€ списка
    std::list<Host>::iterator curr_host_itr;
    std::list<VM>::iterator curr_vm_itr;
    std::vector<VM*>::iterator deployed_vm_itr;

    // параметры запроса
    xmlrpc_c::paramList xml_params_list;
    xml_params_list.add(xmlrpc_c::value_string(session_string));

    xmlrpc_c::value response = generate_call_and_response(transport_type, method_string, serverUrl, xml_params_list);
    xmlrpc_c::rpcOutcome outcome(response);
    
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
            
            cout << tool.name() << " ID " << tool.child("ID").text().as_int() << '\n';

            curr_host_itr = std::find_if(
                all_hosts.begin(),
                all_hosts.end(),
                [id = tool.child("ID").text().as_int()](Host host) {return id == host.id; }
            );

            if (curr_host_itr != all_hosts.end())
            {
                curr_host_itr->loads[0] = tool.child("HOST_SHARE").child("USED_CPU").text().as_double() / tool.child("HOST_SHARE").child("TOTAL_CPU").text().as_double();
                curr_host_itr->loads[1] = tool.child("HOST_SHARE").child("USED_MEM").text().as_double() / tool.child("HOST_SHARE").child("TOTAL_MEM").text().as_double();
                curr_host_itr->loads[2] = tool.child("HOST_SHARE").child("USED_DISK").text().as_double() / tool.child("HOST_SHARE").child("MAX_DISK").text().as_double();

                for (pugi::xml_node vm_node : tool.child("VMS").children())
                {

                    // ѕроверим существование такой машины
                    curr_vm_itr = std::find_if(
                        all_VMs.begin(),
                        all_VMs.end(),
                        [id = vm_node.text().as_int()](VM vm) {return id == vm.id; }
                    );

                    // ≈сли такой машины нет в списке, создадим
                    if (curr_vm_itr == all_VMs.end())
                    {
                        // —оздаем
                        VM new_vm_tmp = VM();
                        new_vm_tmp.id = vm_node.text().as_int();
                        // TODO:
                        // Ќужна организаци€ по лейблам, пока настройки по умолчанию
                        // Ќужно читать конфиги
                        for (Host host : all_hosts)
                        {
                            new_vm_tmp.approved_hosts.push_back(&host);
                        }
                        new_vm_tmp.host = &*curr_host_itr;

                        //ƒобавим в список всех виртуальных машин
                        all_VMs.push_back(new_vm_tmp);
                        // ƒобавим теперь ее к хосту
                        auto buf_iter = all_VMs.end();
                        --buf_iter;
                        VM* our_new_vm_ptr = &*(buf_iter);
                        curr_host_itr->deployed_machines.push_back(our_new_vm_ptr);
                    }
                    else
                    {
                        // ѕроверим наличие машины на хосте
                        deployed_vm_itr = std::find_if(
                            curr_host_itr->deployed_machines.begin(),
                            curr_host_itr->deployed_machines.end(),
                            [id = vm_node.text().as_int()](VM* vm) {return id == vm->id; }
                        );

                        // такой виртальной машины мы не обнаружили на хосте своего класса
                        if (deployed_vm_itr == curr_host_itr->deployed_machines.end())
                        {
                            curr_host_itr->deployed_machines.push_back(&*curr_vm_itr);
                            curr_vm_itr->host = &*curr_host_itr;
                        }
                    }
                }
            }
            else
            {
                Host host_tmp(3);
                host_tmp.id = tool.child("ID").text().as_int();
                host_tmp.loads[0] = tool.child("HOST_SHARE").child("USED_CPU").text().as_double() / tool.child("HOST_SHARE").child("TOTAL_CPU").text().as_double();
                host_tmp.loads[1] = tool.child("HOST_SHARE").child("USED_MEM").text().as_double() / tool.child("HOST_SHARE").child("TOTAL_MEM").text().as_double();
                host_tmp.loads[2] = tool.child("HOST_SHARE").child("USED_DISK").text().as_double() / tool.child("HOST_SHARE").child("MAX_DISK").text().as_double();


                all_hosts.push_back(host_tmp);
                auto curr_host_itr = all_hosts.end();
                --curr_host_itr;

                for (pugi::xml_node vm_node : tool.child("VMS").children())
                {
                    cout << "==== With deployed VM " << vm_node.text().as_int() << '\n';

                    curr_vm_itr = std::find_if(
                        all_VMs.begin(),
                        all_VMs.end(),
                        [id = vm_node.text().as_int()](VM vm) {return id == vm.id; }
                    );

                    // ≈сли VM не существует
                    if (curr_vm_itr == all_VMs.end())
                    {
                        // —оздаем
                        VM new_vm_tmp = VM();
                        new_vm_tmp.id = vm_node.text().as_int();
                        // TODO:
                        // Ќужна организаци€ по лейблам, пока настройки по умолчанию
                        // Ќужно читать конфиги
                        for (Host host : all_hosts)
                        {
                            new_vm_tmp.approved_hosts.push_back(&host);
                        }
                        new_vm_tmp.host = &*curr_host_itr;

                        //ƒобавим в список всех виртуальных машин
                        all_VMs.push_back(new_vm_tmp);
                        // ƒобавим теперь ее к хосту
                        auto buf_iter = all_VMs.end();
                        --buf_iter;
                        VM* our_new_vm_ptr = &*(buf_iter);
                        curr_host_itr->deployed_machines.push_back(our_new_vm_ptr);
                    }
                    else
                    {
                        curr_host_itr->deployed_machines.push_back(&*curr_vm_itr);
                        curr_vm_itr->host = &*curr_host_itr;

                        
                    }
                }

            }

            cout << "--- CPU " << tool.child("HOST_SHARE").child("USED_CPU").text().as_double() << '\n';
            cout << "--- RAM " << tool.child("HOST_SHARE").child("USED_MEM").text().as_double() << '\n';
            cout << "--- DISK " << tool.child("HOST_SHARE").child("USED_DISK").text().as_double() << '\n';

            

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


void migrate_vm(string transport_type, string serverUrl, int vm_id, int target_host_id, int target_ds_id, string session_string = "admin:admin", bool live_migration = true, bool host_capacity = true, int migration_type = 0)
{
    string method_string = "one.vm.migrate";

    xmlrpc_c::paramList xml_params_list;
    xml_params_list.add(xmlrpc_c::value_string(session_string));
    xml_params_list.add(xmlrpc_c::value_int(vm_id));
    xml_params_list.add(xmlrpc_c::value_int(target_host_id));
    xml_params_list.add(xmlrpc_c::value_boolean(live_migration));
    xml_params_list.add(xmlrpc_c::value_boolean(host_capacity));
    xml_params_list.add(xmlrpc_c::value_int(target_ds_id));
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
        cout << "Success " << response_array_vector_values.size();
        cout << "\nVM id: "
            << xmlrpc_c::value_int(response_array_vector_values[1]);
        cout << "\nError code: "
            << xmlrpc_c::value_int(response_array_vector_values[2]);
    }
    else
    {
        cout << "migrate_vm failed for VM "
            << vm_id
            << " with target host "
            << target_host_id
            << " and DataStore "
            << target_ds_id << ":\n"
            << xmlrpc_c::value_boolean(response_array_vector_values[0])
            << "\n - Error string: "
            << static_cast<string>(xmlrpc_c::value_string(response_array_vector_values[1]))
            << "\n - Error code: "
            << xmlrpc_c::value_int(response_array_vector_values[2])
            << "\n - Host that caused error: "
            << xmlrpc_c::value_int(response_array_vector_values[3])
            << endl;
    }

}


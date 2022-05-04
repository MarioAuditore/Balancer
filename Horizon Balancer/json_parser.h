#pragma once
#include <iomanip>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
    ������� ��� �������� ����������������� json �����.
    � ����� �������� �������� � ���������� ��������� ��� ������� ������ (����) ���������� �����
*/
json parse_json_config(std::string path = "labels_config.json")
{
    /*
        ��������� ��� json
        {
         "Label_1":{
                    "cpu": cpu_value,
                    "ram": ram_value,
                    "disk": disk_value,
                    "priority": priority_value,
                    "hosts": -1 or [host_id, host_id, ...]
                    }
        },
        "Label_2":...
    */
    std::ifstream cFile(path);
    if (cFile.is_open())
    {
        json jf = json::parse(cFile);

        return jf;
    }
    else {
        std::cerr << "Couldn't open config file for reading.\n";
    }
}

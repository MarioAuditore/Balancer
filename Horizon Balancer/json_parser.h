#pragma once
#include <iomanip>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*
    Функция для парсинга конфигурационного json файла.
    В файле хранятся сведения о допустимых нагрузках для каждого лейбла (вида) виртальных машин
*/
json parse_json_config(std::string path = "labels_config.json")
{
    /*
        Ожидаемый вид json
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

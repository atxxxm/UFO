#ifndef UFO_ARRAY_H
#define UFO_ARRAY_H

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <iomanip>
#include <Windows.h>
#include <iostream>

/**
 * @brief Класс UFOArray предназначен для хранения пар ключ-значение,
 * организованных по классам, и предоставляет методы для работы с этими данными.
 *
 * @tparam T Тип значения, хранящегося в массиве.
 */
template <typename T>
class UFOArray {
public:
    /**
     * @brief Конструктор по умолчанию.
     */
    UFOArray() = default;

    /**
     * @brief Установка имени класса.
     *
     * @param className Имя класса, которое будет установлено.
     * @return Ссылку на текущий объект UFOArray.
     */
    UFOArray<T>& setClassName(const std::string& className) {
        class_name = className;
        return *this;
    }

    /**
     * @brief Метод для добавления одной пары ключ-значение.
     *
     * @param key Ключ для хранения значения.
     * @param value Значение, связанное с ключом.
     * @return Ссылку на текущий объект UFOArray.
     */
    UFOArray& add(const std::string& key, const T& value) {
        data[class_name][key] = value; // Добавляем по имени класса
        return *this;
    }

    /**
     * @brief Метод для добавления нескольких пар ключ-значение.
     *
     * @param key Ключ для первого значения.
     * @param value Значение для первого ключа.
     * @param args Остальные пары ключ-значение.
     * @return Ссылку на текущий объект UFOArray.
     */
    template <typename... Args>
    UFOArray& add(const std::string& key, const T& value, Args... args) {
        add(key, value);
        return add(args...);
    }

    /**
     * @brief Преобразование в строку для вывода.
     *
     * @return Строковое представление данных текущего класса.
     */
    std::string toString() const {
        std::ostringstream result;
        result << "(" << class_name << ")\n" << "{\n";
        for (auto it = data.at(class_name).begin(); it != data.at(class_name).end(); ++it) {
            result << "\t\"" << it->first << "\": " << it->second;
            if (std::next(it) != data.at(class_name).end()) {
                result << ",\n";
            }
        }
        result << "\n}";
        return result.str();
    }

    /**
     * @brief Сохранение данных в файл.
     *
     * @param filename Путь к файлу для сохранения.
     * @param append Указывает, добавлять ли данные в конец файла (по умолчанию true).
     * @throw std::runtime_error Если не удается открыть файл для записи.
     */
    void save(const std::string& filename, bool append = true) const {
        std::unique_ptr<std::ofstream> outFile(new std::ofstream(filename, append ? std::ios::app : std::ios::out));

        if (!*outFile) {
            throw std::runtime_error("Невозможно открыть файл для записи");
        }

        *outFile << toString() << "\n";
    }

    /**
     * @brief Загрузка данных из файла.
     *
     * @param filename Путь к файлу для загрузки данных.
     * @throw std::runtime_error Если не удается открыть файл для чтения.
     */
    void load(const std::string& filename) {
        std::ifstream inFile(filename);
        if (!inFile) {
            throw std::runtime_error("Невозможно открыть файл для чтения");
        }

        data.clear(); // Очистка текущего хранилища
        std::string line;
        std::string currentClass;

        while (std::getline(inFile, line)) {
            // Обработка строки для получения имени класса
            if (line.find("(") != std::string::npos) {
                currentClass = line.substr(1, line.find(")") - 1); // Получение имени класса
                continue;
            }
            // Обработка строки с данными
            if (line.find(":") != std::string::npos && !currentClass.empty()) {
                size_t pos = line.find(":");
                std::string key = line.substr(1, pos - 3); // Извлечение ключа
                T value = valueFromString(line.substr(pos + 1)); // Извлечение значения
                data[currentClass][key] = value; // Добавление по имени класса
            }
        }
    }

    /**
     * @brief Получение всех элементов по классу.
     *
     * @param className Имя класса для извлечения данных.
     * @return Словарь (map) пар ключ-значение данного класса.
     * @throw std::runtime_error Если класс не найден.
     */
    std::map<std::string, T> getAllByClass(const std::string& className) const {
        if (data.find(className) != data.end()) {
            return data.at(className);
        }
        throw std::runtime_error("Класс не найден");
    }

    /**
     * @brief Получение первого элемента по классу.
     *
     * @param className Имя класса для извлечения данных.
     * @return Пара ключ-значение первого элемента класса.
     * @throw std::runtime_error Если нет элементов для извлечения.
     */
    std::pair<std::string, T> getFirstByClass(const std::string& className) const {
        if (data.find(className) != data.end() && !data.at(className).empty()) {
            return *data.at(className).begin();
        }
        throw std::runtime_error("Нет элементов для извлечения");
    }

    /**
     * @brief Получение последнего элемента по классу.
     *
     * @param className Имя класса для извлечения данных.
     * @return Пара ключ-значение последнего элемента класса.
     * @throw std::runtime_error Если нет элементов для извлечения.
     */
    std::pair<std::string, T> getLastByClass(const std::string& className) const {
        if (data.find(className) != data.end() && !data.at(className).empty()) {
            return *data.at(className).rbegin();
        }
        throw std::runtime_error("Нет элементов для извлечения");
    }

    /**
     * @brief Получение элемента по индексу и классу.
     *
     * @param className Имя класса для извлечения данных.
     * @param index Индекс элемента.
     * @return Пара ключ-значение элемента по индексу.
     * @throw std::out_of_range Если индекс вне диапазона или класс не найден.
     */
    std::pair<std::string, T> getByIndexAndClass(const std::string& className, size_t index) const {
        if (data.find(className) != data.end() && index < data.at(className).size()) {
            auto it = std::next(data.at(className).begin(), index);
            return *it;
        }
        throw std::out_of_range("Индекс вне диапазона или класс не найден");
    }

    /**
     * @brief Открытие файла в приложении.
     *
     * @param filename Путь к файлу для открытия.
     */
    void openFile(const std::string& filename) const {
        std::string command = "start " + filename;
        system(command.c_str());
    }

private:
    std::map<std::string, std::map<std::string, T>> data; // Хранилище пар ключ-значение по классам
    std::string class_name; // Имя класса

    /**
     * @brief Преобразование строки в значение типа T.
     *
     * @param str Строка для преобразования.
     * @return Значение типа T.
     */
    T valueFromString(const std::string& str) {
        T value;
        std::istringstream iss(str);
        iss >> value; // Здесь требуется обработка в зависимости от типа T
        return value;
    }
};

#endif // !UFO_ARRAY_H

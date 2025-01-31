Описание проекта

Консольное приложение для поиска наиболее релевантных документов из предварительно проиндексированной базы.

    Индексация: Движок строит обратный индекс документов для быстрого поиска.

    Ранжирование: Результаты сортируются по убыванию релевантности (на основе частоты слов запроса).

    Формат работы: Конфигурация через JSON-файлы, результаты выводятся в answers.json.

Основные функции:

    Загрузка списка документов из config.json.

    Обработка поисковых запросов из requests.json.

    Поддержка многопоточности (опционально).

    Юнит-тесты для проверки корректности.



Стек технологий

    Язык: C++17

    Библиотеки:

        nlohmann/json — работа с JSON.

        Google Test — тестирование.

    Инструменты:

        CMake — сборка проекта.

        Git — контроль версий.

        Doxygen (опционально) — генерация документации.



Запуск проекта
Предварительные требования

    1. Установите компилятор C++ (например, g++ или clang).

    2. Установите CMake (версия 3.10+).

    3. Установите менеджер пакетов vcpkg (опционально).

Установка зависимостей

    # Установка nlohmann/json через vcpkg
    vcpkg install nlohmann-json
    
    # Или вручную (для Linux/macOS)
    sudo apt-get install nlohmann-json3-dev  # Debian/Ubuntu
    brew install nlohmann-json               # macOS

Сборка и запуск

    # Клонировать репозиторий
    git clone https://github.com/your-username/search-engine.git
    cd search-engine
    
    # Сборка проекта
    mkdir build && cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake  # если используется vcpkg
    make
    
    # Запуск тестов
    ctest --output-on-failure
    
    # Запуск приложения
    ./search_engine

Настройка конфигурации

    Создайте файл config.json в корне проекта:

    {
      "config": {
        "max_responses": 5,
        "files": ["doc1.txt", "doc2.txt", "doc3.txt"]
      }
    }

    Добавьте запросы в requests.json:

    {
      "requests": ["погода в Москве", "новости IT"]
    }

Пример работы

    После запуска программа создаст файл answers.json с результатами:

    {
      "answers": {
        "request1": {
          "result": true,
          "relevance": [
            {"docid": 2, "rank": 0.95},
            {"docid": 0, "rank": 0.75}
          ]
        }
      }
    }

Примечание:
Если вы используете IDE (Visual Studio, CLion и т.д.), импортируйте проект через CMake для автоматической настройки зависимостей.

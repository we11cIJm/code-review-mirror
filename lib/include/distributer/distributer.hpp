/*!
    @file 
    @brief Библиотека distribution
    @author Ковалева М. А.
    @version 1.0
*/

#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

/*! 
    \defgroup Collect Cобрание файлов на ревью
    \brief Данный модуль, предназначен для сканирования склонированных директорий и поиска в них файлов, предназначенных для ревью.
*/
///@{
/*!
    @brief Сканирует директорию на вложенные папки. Файлы не учитываются
    @param admin_dir Путь к директории преподавателя
    @return Вектор путей к вложенным в директории репозиториям
*/
std::vector<std::filesystem::path> scanner(const std::filesystem::path& admin_dir){
    std::vector<std::filesystem::path> list_of_repos;
    
    // Looping until all the items of the directory are exhausted
    for (const auto& entry : std::filesystem::directory_iterator(admin_dir)) {

    // Converting the path to const char * in the subsequent lines
    std::filesystem::path outfilename = entry.path();

    if(std::filesystem::is_directory(outfilename)){
        list_of_repos.push_back(outfilename);
        }        
    }

    return list_of_repos;
}

/*!
    @brief Считывает строки из файла .for_review и создает из них пути до файлов, предназначенных для проверки
    @param student_dir Путь к репозиторию
    @return Вектор путей к файлам, которые владелец репозитория предоставляет для ревью
*/
std::vector<std::filesystem::path> readForReview(std::filesystem::path student_dir)
{
    std::vector<std::filesystem::path> student_owns_those_files;

    std::string for_review_path_str = student_dir.string() + "/.for_review";
    std::filesystem::path for_review_path = std::filesystem::u8path(for_review_path_str);
    std::ifstream for_review(for_review_path, std::ios_base::in); // открытие с помощью конструктора
    if (for_review.is_open()) 
    {
        std::string line;
        int i = 0;
        while(getline(for_review, line))
        {
            std::filesystem::path file_path = std::filesystem::u8path(student_dir.string() + line);
            student_owns_those_files.push_back(file_path);
            i++;
        }
        for_review.close();
    }

    return student_owns_those_files;
}
///@}

static std::random_device rd;
static std::mt19937_64 gen(rd());
static std::uniform_int_distribution<> dis(0, 15);
static std::uniform_int_distribution<> dis2(8, 11);

/*! 
    \defgroup Describe Создание списка распределения
    \brief Данный модуль, предназначен для создания списка распределения файлов, предоставленных для ревью. 
    
    Алгоритм распределения позволяет создать такое соответствие, что все участвующие в ревью получают одинаковое количество файлов.
    При этом появляется множество файлов, которые будут отложены до следующего распределения.
*/
///@{
/*!
    @brief Генерирует случайное id для копируемого файла
    @return Случайно сгенерированная строка
*/
std::string generate_id() {
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    for (i = 0; i < 6; i++) {
        ss << dis2(gen);
    }
    return ss.str();
}

/// @brief Структура для хранения события во время создания списка распределения 
/// @tparam P Обычно std::filesystem::path, но для тестирования может использоваться как std::string
/// @tparam F Обычно File, но для тестирования может использоваться как int
template <typename P, typename F> struct Transaction{
    P from_dir;
    P to_dir;
    F file;
};

/// @brief Структура для хранения информации о файле: его уникальное id и путь до файла в его родном репозитории
struct File{
    std::string id; //randomly generated file_id
    std::filesystem::path file_path;
};

/// @brief Структура для хранения вектора элементов, расположенных в репозитории
/// @tparam T Обычно File, но для тестрования может использоваться как int
template <typename T> struct Directory{
    std::vector<T> files;    
};

/// @brief Компоратор для сортировки двух Directory по возрастанию 
template <typename P, typename F>
bool dir_comp(const std::pair<P, Directory<F>>& a, const std::pair<P, Directory<F>>& b){
    return a.second.files.size() < b.second.files.size();
}

/*!
    @brief Функция для создания списка распределения.

    Алгоритм работы:
    Имеются три структуры: изначальный вектор директорий, новый пустой вектор директорий и дополнительный вектор файлов. 
    Когда файл оказывается распределен, он копируется из первой директории во вторую. Если файл не может быть распределен ни в одну из директорий, то он попадает в дополнительный вектор файлов.
    Распределение происходит следующим образом:
        1. Файлы перебираются по директориям, т.е. сначала рассматривается 1ый файл из 1ой директории, потом 1ый файл из 2ой директории и т.д., потом 2ой файл из 1ой директории... Для каждой директории проверяется, что её размер больше текущего индекса файла, то есть, условие его наличия в этой директории. Если файл не найден, то совершается переход к следующей директории.
        2. Новые директории для каждой итерации (во втором цикле) сортируются по возрастанию кол-ва элементов. Если путь к минимальной директории (с индексом 0) не совпадает с путем текущей изначальной директорией, то файл помещается в эту минимальную директорию.
        3. Иначе, проверяется, что разница между текущим минимальным размером файлов следующим (размер новой директории с индексом 1) размером равна 0. Если это так, то файл помещается в новую директорию с индексом 1.
        4. Иначе, файл помещается в дополнительный вектор файлов.
        5. После обхода всех файлов из списка происходит проход по дополнительному вектору файлов. Это необходимо для того, чтобы проверить, есть ли там файлы, которые ещё можно куда-то распределить, при этом не нарушив равнозначность кол-ва файлов в директориях.
    @param directories Пары Путь:Директория (которая явл. структурой, хранящей файлы)
    @param key_for_extra Путь для дополительного вектора файлов
    @return Вектор транзакций, который необходим для сохранения истории распределения
*/
template <typename P, typename F>
std::vector<Transaction<P, F>> describeDistribution(const std::vector<std::pair<P, Directory<F>>>& directories, P key_for_extra){
    std::vector<Transaction<P, F>> transactions;
    std::vector<std::pair<P, Directory<F>>> newDirectories;
    for (auto const & [k, v] : directories)
    {
        newDirectories.push_back({k, Directory<F>()});
    }
    

    int max_files = 0;
    for (int i = 0; i < directories.size(); i++){
        if (directories[i].second.files.size() > max_files){
            max_files = directories[i].second.files.size();
        }
    }


    std::vector<std::pair<P, F>> extraFiles; 
    

    for (int i = 0; i < max_files; i++){
        for (int j = 0; j < directories.size(); j++){

            if(directories[j].second.files.size() < i + 1)
                break;

            F current_file = directories[j].second.files[i];
            
            
            sort(newDirectories.begin(), newDirectories.end(), dir_comp<P, F>);

            
            if(newDirectories[0].first == directories[j].first){
                if(newDirectories[1].second.files.size() - newDirectories[0].second.files.size() > 0){
                    extraFiles.push_back({directories[j].first, current_file});
                    transactions.push_back(Transaction<P, F>{
                        directories[j].first,
                        key_for_extra,
                        current_file
                    });
                }
                else{
                    newDirectories[1].second.files.push_back(current_file);
                    transactions.push_back(Transaction<P, F>{
                        directories[j].first,
                        newDirectories[1].first,
                        current_file
                    });
                }
            }
            else{
                newDirectories[0].second.files.push_back(current_file);
                transactions.push_back(Transaction<P, F>{
                        directories[j].first,
                        newDirectories[0].first,
                        current_file
                    });
            }
        }
    }

        for(int i = 0; i < extraFiles.size(); i++){
        F current_file = extraFiles[i].second;
        sort(newDirectories.begin(), newDirectories.end(), dir_comp<P, F>);

        if (newDirectories[0].first == extraFiles[i].first){
            continue;
        }
        else{
            newDirectories[0].second.files.push_back(current_file);
            extraFiles.erase(extraFiles.begin()+i);
            if(newDirectories[1].second.files.size() - newDirectories[0].second.files.size() > 0){
                break;
            }
        }
    }

    Directory<F> extraFilesDirectory;
    for(int i = 0; i < extraFiles.size(); i++){
        extraFilesDirectory.files.push_back(extraFiles[i].second);
    }
    newDirectories.push_back({key_for_extra, extraFilesDirectory});

    return transactions;
}
///@}

/*! 
    \defgroup Distribute Распределение
    \brief Данный модуль, предназначен для распределения файлов.
*/
///@{

/// @brief Функция для чтения csv-файлов, необходима для проверки прошлых распределений
std::vector<Transaction<std::filesystem::path, std::filesystem::path>>  readCSVFile(const std::string& filename) {
    std::vector<Transaction<std::filesystem::path, std::filesystem::path>>  transactions;

    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;

        // Пропускаем заголовок CSV файла
        std::getline(file, line);

        // Читаем строки с данными
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string file_id;
            std::string from_path;
            std::string to_path;

            // Разделяем строку на значения по запятой
            std::getline(ss, file_id, ',');
            std::getline(ss, from_path, ',');
            std::getline(ss, to_path, ',');

            // Создаем объект Transaction и добавляем его в вектор
            transactions.push_back({ file_id, std::filesystem::path(from_path), std::filesystem::path(to_path) });
        }
        file.close();
    }
    return transactions;
}

/// @brief Функция, создающая csv-файл, который хранит историю распределения
void writeTransactionsInCSV(
    const std::string& filename,
    const std::vector<Transaction<std::filesystem::path, File>>& transactions, 
    const std::filesystem::path& admin_dir)
{
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "id,from_path,to_path\n";

        for (size_t i = 0; i < transactions.size(); i++) {
            file << transactions[i].file.id << "," << transactions[i].from_dir << "," << transactions[i].to_dir << '\n';
        }
        file.close();
    }
}

/*!
    @brief Структура для возвращения статуса выполнения функции distributeFiles
    
    statusCode возвращает 0 в случае успешного выполнения распределения, 2 в случае, если кол-во директорий больше, чем кол-во файлов
*/
struct FunctionStatus{
    int statusCode;
    std::string message;
    std::filesystem::path file_of_transactions;
};

/*!
    @brief Функция, осуществляющая распределение файлов по полученному в прошлом модуле списку. Файлы передаются копированием
    @param admin_dir_path Путь до директории преподавателя
    @return Статус работы функции
*/
FunctionStatus distributeFiles(std::filesystem::path admin_dir_path){
    FunctionStatus result;

    std::vector<std::filesystem::path> admin_dir_folders = scanner(admin_dir_path);

    std::vector<std::pair<std::filesystem::path, Directory<File>>> scanned_files_in_directories;
    for (int i = 0; i < admin_dir_folders.size(); i++){
        std::vector<File> output_files;

        std::filesystem::path directory = admin_dir_folders[i];
        std::vector<std::filesystem::path> files = readForReview(directory);
        for (int j = 0; j < files.size(); j++){
            File temp_file{generate_id(), files[j]};
            output_files.push_back(temp_file);
        }
        Directory<File> files_in_directory;
        files_in_directory.files = output_files;

        scanned_files_in_directories.push_back({directory, files_in_directory});
    }

    
    int total_number_of_files = 0;
    for(int i = 0; i < scanned_files_in_directories.size(); i++){
        total_number_of_files += scanned_files_in_directories[i].second.files.size();
    }
    
    if(total_number_of_files < admin_dir_folders.size()){
        result.statusCode = 2;
        result.message = "Not possible to do a distribution. Number of students is bigger than number of files. Please, try again later.";
        return result;
    }
    else{

        std::filesystem::path extra_directory = std::filesystem::u8path(admin_dir_path.string() + "/others");
        std::filesystem::create_directory(extra_directory);
    
        std::vector<Transaction<std::filesystem::path, File>> destributionlist = describeDistribution(scanned_files_in_directories, extra_directory);
        writeTransactionsInCSV("transactions.csv", destributionlist, admin_dir_path);

        
        for(int i = 0; i < destributionlist.size(); i++){

            std::filesystem::path from_path = destributionlist[i].file.file_path;
            

            std::string filename = destributionlist[i].file.id;
            std::filesystem::path to_path = std::filesystem::u8path(destributionlist[i].to_dir.string() + std::string("/to_review"));
            std::filesystem::create_directory(to_path);
            std::filesystem::path copy_path = std::filesystem::u8path(to_path.string() + "/" + filename + ".cpp");

            std::filesystem::copy(from_path, copy_path);
        }
        


        result.statusCode = 0;
        result.message = "Distribution is done successfully!";
        return result;
    }
}
///@}

#endif // DISTRIBUTION_H
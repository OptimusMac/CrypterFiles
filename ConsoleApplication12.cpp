#include <iostream>
#include <map>
#include <functional>
#include <string>
#include <stdexcept>
#include <fstream>
#include <filesystem>

int64_t decode = 0xFBBA0C9FA;
char* path = nullptr;
bool remove_file = false;

class CommandLineParser {
public:
    CommandLineParser() : force(false) {
        commands["--help"] = [this]() { showHelp(); };
        commands["--version"] = [this]() { showVersion(); };
    }

    void encryptDecryptFile(const std::string& inputFilePath, const std::string& outputFilePath, char mask) {
        std::ifstream inputFile(inputFilePath, std::ios::binary);
        std::ofstream outputFile(outputFilePath, std::ios::binary);

        if (!inputFile.is_open()) {
            std::cerr << "Ошибка: Не удалось открыть файл для чтения: " << inputFilePath << std::endl;
            return;
        }

        if (!outputFile.is_open()) {
            std::cerr << "Ошибка: Не удалось открыть файл для записи: " << outputFilePath << std::endl;
            return;
        }

        char buffer;
        while (inputFile.get(buffer)) {
            buffer ^= mask;
            outputFile.put(buffer);
        }

        inputFile.close();
        outputFile.close();
    }



    void removeFile(std::string path)
    {
        if (remove_file) {
            std::filesystem::remove(path);
        }
    }

    void processFile(const std::string& filePath, bool encrypt) {
        std::filesystem::path inputPath(filePath);
        std::filesystem::path outputDir = encrypt ? "encrypt" : "decrypt";
        std::filesystem::create_directory(outputDir);

        std::filesystem::path outputFilePath = outputDir / inputPath.filename();

        encryptDecryptFile(filePath, outputFilePath.string(), decode);

        if (encrypt) {
            std::cout << "Файл зашифрован и сохранен в папке 'encrypt': " << outputFilePath <<
                " Ключ шифрования " << std::to_string(decode) << std::endl;
        }
        else {
            std::cout << "Файл расшифрован и сохранен в папке 'decrypt': " << outputFilePath << std::endl;
        }
        removeFile(filePath);
    
    }

    void parse(int64_t argc, char* argv[]) {
        bool encryptFlag = false;
        bool decryptFlag = false;

        for (int64_t i = 1; i < argc; ++i) {
            std::string arg(argv[i]);
            if (commands.find(arg) != commands.end()) {
                commands[arg]();
            }
            else if (arg == "--code") {
                if (i + 1 < argc) {
                    handleCode(argv[i + 1]);
                    i++;
                }
                else {
                    std::cout << "Ошибка: Не указано значение для --code." << std::endl;
                }
            }
            else if (arg == "--encrypt") {
                encryptFlag = true;
            }
            else if (arg == "--decrypt") {
                decryptFlag = true;
            }
            else if (arg == "--force") {
                force = true;
            }
            else if (arg == "--delete") {
                remove_file = true;
            }
            else {
                path = argv[i];
            }
        }

        if (encryptFlag) {
            if (path != nullptr) {
                if (std::filesystem::is_directory(path)) {
                    std::string result;
                    if (!force) {
                        std::cout << "Вы указали деррикторию, вы согласны на криптинг всей дерриктории?" << '\n';
                        std::cout << "(1) Да." << '\n' << "(2) Нет" << '\n';
                        std::cin >> result;
                    }
                    if (result == "1" || force) {
                        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                            if (entry.is_regular_file()) {
                                processFile(entry.path().string(), true);
                            }
                        }
                    }
                    else {
                        exit(7);
                    }
                }
                else {
                    processFile(path, true);
                }
            }
            else {
                std::cout << "Ошибка: Не указан файл для шифрования." << std::endl;
            }
        }

        if (decryptFlag) {
            if (path != nullptr) {
                if (std::filesystem::is_directory(path)) {
                    std::string result;
                    if (!force) {
                        std::cout << "Вы указали деррикторию, вы согласны на декриптинг всей дерриктории?" << '\n';
                        std::cout << "(1) Да." << '\n' << "(2) Нет" << '\n';

                        std::cin >> result;
                    }
                    if (result == "1" || force) {
                        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                            if (entry.is_regular_file()) {
                                processFile(entry.path().string(), false);
                            }
                        }
                    }
                    else {
                        exit(7);
                    }
                }
                else {
                    processFile(path, false);
                }
            }
            else {
                std::cout << "Ошибка: Не указан файл для дешифрования." << std::endl;
            }
        }
    }

    void setDecode(int64_t& input, int64_t* out) {
        *out = input;
    }

private:
    std::map<std::string, std::function<void()>> commands;
    bool force; // Переменная для флага force

    void showHelp() {
        std::cout << "Доступные команды:\n";
        std::cout << "--help       Показать эту справку\n";
        std::cout << "--version    Показать версию программы\n";
        std::cout << "--code      Указать свой код для крипта декрипта файла\n";
        std::cout << "--decrypt     Декриптинг файла\n";
        std::cout << "--encrypt     Криптинг файла\n";
        std::cout << "--force      Принудительное выполнение операций\n";
        std::cout << "--delete      Удалит исходный файл после крипта/декрипта\n";
    }

    void showVersion() {
        std::cout << "Версия программы 1.0.0\n";
    }

    void handleCode(const std::string& input) {
        try {
            int64_t code = static_cast<int64_t>(std::stoi(input));
            setDecode(code, &decode);

            std::cout << "Вы указали код: " << code << ", результат: " << decode << std::endl;
        }
        catch (const std::invalid_argument&) {
            std::cout << "Ошибка: введено некорректное число." << std::endl;
        }
        catch (const std::out_of_range&) {
            std::cout << "Ошибка: число выходит за пределы диапазона int64_t." << std::endl;
        }
    }
};

int main(int64_t argc, char* argv[]) {
    setlocale(LC_ALL, "RU");
    CommandLineParser parser;
    parser.parse(argc, argv);

    if (path == nullptr) {
        std::cout << "Ошибка: путь не указан!" << std::endl;
        return 3;
    }

    std::cout << "Путь: " << path << std::endl;

    std::fstream file = std::fstream(path);

    if (!file.good()) {
        std::cout << "Файла не существует!" << '\n';
        return 2;
    }

    return 0;
}

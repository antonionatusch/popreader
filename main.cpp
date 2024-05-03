#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>

struct Municipality {
    int code;
    std::string name;
    std::string province;
    std::string department;
    int population;
};

class PopulationReader {
public:
    PopulationReader(const std::string& excelFile, const std::string& binaryFile)
        : excelFile(excelFile), binaryFile(binaryFile) {}

    void loadDataFromExcel() {
        std::ifstream file(excelFile);
        if (!file) {
            std::cerr << "Error al abrir el archivo CSV." << std::endl;
            exit(1);
        }

        std::string line;
        for (int i = 0; i < 2; ++i) {
            std::getline(file, line);
        }
        // Ignorar la primera línea que contiene los encabezados
        getline(file, line);

        int lineNumber = 2; // Empezar desde la segunda línea (1-indexed)
        while (getline(file, line)) {
            std::stringstream ss(line);
            std::string token;
            Municipality municipality;

            getline(ss, token, ';'); // Leer código INE
            municipality.code = stoi(token);

            getline(ss, municipality.name, ';'); // Leer nombre
            getline(ss, municipality.province, ';'); // Leer provincia
            getline(ss, municipality.department, ';'); // Leer departamento

            getline(ss, token, ';'); // Leer población
            // Eliminar cualquier punto en la población
            token.erase(remove(token.begin(), token.end(), '.'), token.end());
            try {
                municipality.population = stoi(token);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error al convertir población en la línea " << lineNumber << ": " << e.what() << std::endl;
                std::cerr << "Línea: " << line << std::endl;
                file.close();
                exit(1);
            }

            municipalities.push_back(municipality);
            lineNumber++;
        }
        // Guardar los datos en el archivo binario
        saveDataToBinary();
    }

    void createIndex() {
        index.clear();
        for (const auto& municipality : municipalities) {
            index[municipality.code] = municipality;
        }
    }

    void mergeSortByPopulation() {
        // Ordenar por población en orden descendente
        std::sort(municipalities.begin(), municipalities.end(), [](const Municipality& a, const Municipality& b) {
            return a.population > b.population;
        });

        // Guardar los datos ordenados en el archivo binario
        saveDataToBinary();
    }

    void calculatePopulationSumByProvinceAndDepartment(const std::string& outputFile) {
        std::map<std::string, std::map<std::string, int>> populationSum;

        for (const auto& municipality : municipalities) {
            populationSum[municipality.province][municipality.department] += municipality.population;
        }

        std::ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            std::cerr << "Error: No se pudo abrir el archivo de salida.\n";
            return;
        }

        for (const auto& provinceEntry : populationSum) {
            outFile << "PROVINCIA:\n" << provinceEntry.first << '\n';
            for (const auto& departmentEntry : provinceEntry.second) {
                outFile << "DEPARTAMENTO:\n" << departmentEntry.first << '\n';
                outFile << "POBLACIÓN:\n" << departmentEntry.second << '\n';
            }
        }

        outFile.close();
        std::cout << "Sumatoria de población por provincia y departamento guardada en: " << outputFile << '\n';
    }

private:
    std::string excelFile;
    std::string binaryFile;
    std::vector<Municipality> municipalities;
    std::map<int, Municipality> index;

    void saveDataToBinary() {
        std::ofstream outFile(binaryFile, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Error: No se pudo abrir el archivo binario.\n";
            return;
        }

        for (const auto& municipality : municipalities) {
            outFile.write(reinterpret_cast<const char*>(&municipality), sizeof(Municipality));
        }

        outFile.close();
    }
};

int main() {
    std::string excelFile = "C:\\Users\\antho\\CLionProjects\\popreader\\datos-ine-csv.csv"; // Nombre del archivo de Excel
    std::string binaryFile = "data.bin"; // Nombre del archivo binario
    std::string outputFile = "population_sum.txt"; // Nombre del archivo de salida

    PopulationReader reader(excelFile, binaryFile);
    reader.loadDataFromExcel();
    reader.createIndex();
    std::cout<<"Desplegando la información de la municipalidad con índice: ";
    // implementación de, dado un índice input, desplegar su información
    reader.mergeSortByPopulation();
    reader.calculatePopulationSumByProvinceAndDepartment(outputFile);

    return 0;
}

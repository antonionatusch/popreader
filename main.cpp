#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <limits>
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

        // Omitir la primera línea
        std::string line;
        std::getline(file, line);

        int lineNumber = 2;
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
            token.erase(remove(token.begin(), token.end(), ','), token.end());
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

        saveDataToBinary();
    }

    void createIndex() {
        index.clear();
        for (const auto& municipality : municipalities) {
            index[municipality.code] = municipality;
        }
    }


    void mergeSortByPopulation() {
        mergeSortByPopulationAux(0, municipalities.size() - 1);
        saveDataToBinary();
    }

    void mergeSortByPopulationAux(int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSortByPopulationAux(left, mid);
            mergeSortByPopulationAux(mid + 1, right);
            merge(left, mid, right);
        }
    }

    void merge(int left, int mid, int right) {
        int n1 = mid - left + 1;
        int n2 = right - mid;

        std::vector<Municipality> leftArray(n1);
        std::vector<Municipality> rightArray(n2);


        for (int i = 0; i < n1; i++) {
            leftArray[i] = municipalities[left + i];
        }
        for (int j = 0; j < n2; j++) {
            rightArray[j] = municipalities[mid + 1 + j];
        }

        int i = 0;
        int j = 0;
        int k = left;


        while (i < n1 && j < n2) {
            if (leftArray[i].population >= rightArray[j].population) {
                municipalities[k] = leftArray[i];
                i++;
            } else {
                municipalities[k] = rightArray[j];
                j++;
            }
            k++;
        }


        while (i < n1) {
            municipalities[k] = leftArray[i];
            i++;
            k++;
        }


        while (j < n2) {
            municipalities[k] = rightArray[j];
            j++;
            k++;
        }
    }


    void PopulationSum(const std::string& outputFile) {
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

    void displayMunicipalityInfo(int code) {
        auto it = index.find(code);
        if (it == index.end()) {
            std::cout << "No se encontró una municipalidad con el código INE proporcionado.\n";
        } else {
            const Municipality& municipality = it->second;
            std::cout << "Código: " << municipality.code << '\n';
            std::cout << "Nombre: " << municipality.name << '\n';
            std::cout << "Provincia: " << municipality.province << '\n';
            std::cout << "Departamento: " << municipality.department << '\n';
            std::cout << "Población: " << municipality.population << '\n';
        }
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
            writeMunicipality(outFile, municipality);
        }

        outFile.close();
    }

    void writeMunicipality(std::ofstream& outFile, const Municipality& municipality) {
        outFile.write(reinterpret_cast<const char*>(&municipality.code), sizeof(municipality.code));
        writeString(outFile, municipality.name);
        writeString(outFile, municipality.province);
        writeString(outFile, municipality.department);
        outFile.write(reinterpret_cast<const char*>(&municipality.population), sizeof(municipality.population));
    }

    void writeString(std::ofstream& outFile, const std::string& str) {
        size_t len = str.size();
        outFile.write(reinterpret_cast<const char*>(&len), sizeof(len));
        outFile.write(str.data(), len);
    }
};




std::string readString(std::ifstream& inFile) {
    // Leer la longitud de la cadena seguida de la cadena desde el archivo binario
    size_t len;
    inFile.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string str(len, '\0');
    inFile.read(&str[0], len);
    return str;
}
bool readMunicipality(std::ifstream& inFile, Municipality& municipality) {
    // Leer cada campo desde el archivo binario
    if (!inFile.read(reinterpret_cast<char*>(&municipality.code), sizeof(municipality.code))) {
        return false;
    }
    municipality.name = readString(inFile);
    municipality.province = readString(inFile);
    municipality.department = readString(inFile);
    if (!inFile.read(reinterpret_cast<char*>(&municipality.population), sizeof(municipality.population))) {
        return false;
    }
    return true;
}

void displayRecords(const std::string& binaryFile) {
    std::ifstream inFile(binaryFile, std::ios::binary);
    if (!inFile.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo binario para lectura.\n";
        return;
    }

    std::cout << "Registros del archivo binario:\n";

    Municipality municipality;

    while (readMunicipality(inFile, municipality)) {
        std::cout << "Código: " << municipality.code << '\n';
        std::cout << "Nombre: " << municipality.name << '\n';
        std::cout << "Provincia: " << municipality.province << '\n';
        std::cout << "Departamento: " << municipality.department << '\n';
        std::cout << "Población: " << municipality.population << "\n\n";
    }

    inFile.close();
}



int main() {
    std::string excelFile = "C:\\Users\\antho\\CLionProjects\\popreader\\datos-ine-sinpuntos.csv"; // formato csv para mejor legibilidad
    std::string binaryFile = "data.bin";
    char op;
    PopulationReader reader(excelFile, binaryFile);
    reader.loadDataFromExcel();
    reader.createIndex();
    std::cerr<<"AVISO: EL ARCHIVO DEBE ESTAR EN FORMATO .CSV, SI NO, NO FUNCIONARÁ. \n";
    do
    {
        std::cout<<"Datos de población INE 2022 \n";
        std::cout<<"a.- Consultar un registro dado su código de INE \n";
        std::cout<<"b.- Listar registros según población en orden descendente \n";
        std::cout<<"c.- Realizar sumatoria y guardarlo en un archivo \n";
        std::cout<<"d.- Salir \n";
        std::cout<<"Ingrese una opcion: ";
        std::cin >> op;

        // Validar que el usuario solo haya ingresado un carácter
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (std::cin.gcount() > 1) {
            std::cerr << "Error: Debe ingresar solo un caracter.\n";
            continue;
        }

        switch (op)
        {
            case 'a':
            {
                std::cout << "Ingrese el código INE de la municipalidad para desplegar su información: ";
                int INEcode;
                std::cin >> INEcode;
                reader.displayMunicipalityInfo(INEcode);
                break;
            }

            case 'b':
            {
                reader.mergeSortByPopulation();
                displayRecords(binaryFile);
                break;
            }

            case 'c':
            {
                std::string outputFile, choice;
                std::cout << "¿Desea especificar la ruta del archivo? (s/n): ";
                std::cin >> choice;
                if (tolower(choice[0]) == 's') {
                    std::cout << "Ingrese la ruta completa del archivo (incluyendo el nombre y la extensión): ";
                    std::cin >> outputFile;
                } else {
                    std::cout << "Ingrese el nombre del archivo (separando las palabras con guiones): ";
                    std::cin >> outputFile;
                    outputFile += ".txt"; // Agregar la extensión .txt si no la proporciona el usuario
                }
                reader.PopulationSum(outputFile);
                std::cin.clear();
                std::cin.ignore(256, '\n');
                break;
            }
            case 'd':
            {
                std::cout<<"Gracias. \n";
                break;
            }
            default:
            {
                std::cout<<"Escoja alguna de las opciones o digite 'd' para salir. \n";
                break;
            }
        }

    }
    while (op != 'd');

    return 0;
}
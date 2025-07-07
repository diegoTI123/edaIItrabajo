#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

using namespace std;

class Persona {
private:
    string nombre;
    int dni;
    int prioridad;
    string zona;
public:
    Persona(string n, int d, int p, string z) : nombre(n), dni(d), prioridad(p), zona(z) {} //constructor y sus parametros

    string getNombre() {
        return nombre;
    }
    int getDni() {
        return dni;
    }
    int getPrioridad() {
        return prioridad;
    }
    string getZona() {
        return zona;
    }

    void setPrioridad(int p) {
        prioridad = p;
    }
    void setZona(string z) {
        zona = z;
    }
};

class RegistroAcceso {
public:
    string zona;
    string hora;
    int dni;

    RegistroAcceso(string z, string h, int d) : zona(z), hora(h), dni(d) {}

    //
    bool operator<(const RegistroAcceso& otro) const {
        if (hora != otro.hora) return hora < otro.hora;
        return dni < otro.dni;
    }

    bool operator>(const RegistroAcceso& otro) const {
        if (hora != otro.hora) return hora > otro.hora;
        return dni > otro.dni;
    }
};

class TablaHashPersonas {
private:
    struct Par { //contiene valor hash y el puntero de persona (-1 indica pos vacia)
        int hash;
        Persona* persona;
        Par() : hash(-1), persona(nullptr) {}
        Par(int h, Persona* p) : hash(h), persona(p) {}
    };

    Par* tabla;
    int tam;
    int cantidad;
    const float factorCargaMaximo = 0.7; //para activar rehashing

    int funcionHash(int clave) {
        return abs(clave) % tam;
    }
    //se duplica el tamaño, usando sondeo lineal
    void rehashing() {
        int tamViejo = tam;
        tam *= 2;
        Par* nuevaTabla = new Par[tam];

        for (int i = 0; i < tamViejo; i++) {
            if (tabla[i].hash != -1) {
                int pos = funcionHash(tabla[i].persona->getDni());
                while (nuevaTabla[pos].hash != -1) {
                    pos = (pos + 1) % tam;
                }
                nuevaTabla[pos] = tabla[i];
            }
        }

        delete[] tabla;
        tabla = nuevaTabla;
    }

public:
    TablaHashPersonas(int t) : tam(t), cantidad(0) { //crea tabla
        tabla = new Par[tam];
    }

    ~TablaHashPersonas() { //la elimina para el rehashing
        for (int i = 0; i < tam; i++) {
            if (tabla[i].persona != nullptr) {
                delete tabla[i].persona;
            }
        }
        delete[] tabla;
    }
    //inserta una persona en tabla
    void insertar(Persona* persona) {
        if ((float)cantidad / tam >= factorCargaMaximo) {
            rehashing();
        }
        //obtiene dni y calcula pos hash
        int dni = persona->getDni();
        int pos = funcionHash(dni);
        int intentos = 0;
        //si la linea esta ocupada, recorre linealmente hasta encontrar espacio, sino hay, fue pe
        while (tabla[pos].hash != -1 && intentos < tam) {
            if (tabla[pos].persona->getDni() == dni) {
                cout << "Persona con DNI " << dni << " ya existe." << endl;
                return;
            }
            pos = (pos + 1) % tam;
            intentos++;
        }

        if (intentos < tam) {
            tabla[pos] = Par(funcionHash(dni), persona);
            cantidad++;
        } else {
            cout << "Error: No se pudo insertar a " << persona->getNombre() << endl;
        }
    }
    //se busca por dni
    Persona* buscar(int dni) {
        int pos = funcionHash(dni);
        int intentos = 0;

        while (intentos < tam) {
            if (tabla[pos].hash == -1) {
                return nullptr;
            }
            if (tabla[pos].persona->getDni() == dni) {
                return tabla[pos].persona;
            }
            pos = (pos + 1) % tam;
            intentos++;
        }

        return nullptr;
    }

    bool validar(int dni) {
        return buscar(dni) != nullptr;
    }
    //obtiene todas las personas
    vector<Persona*> obtenerTodasLasPersonas() {
        vector<Persona*> personas;
        for (int i = 0; i < tam; i++) {
            if (tabla[i].hash != -1 && tabla[i].persona != nullptr) {
                personas.push_back(tabla[i].persona);
            }
        }
        return personas;
    }
};

class MaxHeap {
private:
    struct ElementoHeap {
        int dni;
        int prioridad;

        bool operator<(const ElementoHeap& otro) const {
            return prioridad < otro.prioridad;
        }
    };

    ElementoHeap* heap;
    int capacidad;
    int tam;

    int padre(int i) { return (i - 1) / 2; }
    int izquierdo(int i) { return 2 * i + 1; }
    int derecho(int i) { return 2 * i + 2; }

    void flotar(int i) {
        while (i > 0 && heap[padre(i)] < heap[i]) {
            ElementoHeap temp = heap[i];
            heap[i] = heap[padre(i)];
            heap[padre(i)] = temp;
            i = padre(i);
        }
    }

    void hundir(int i) {
        int maxIdx = i;
        int izq = izquierdo(i);
        int der = derecho(i);

        if (izq < tam && heap[maxIdx] < heap[izq]) {
            maxIdx = izq;
        }

        if (der < tam && heap[maxIdx] < heap[der]) {
            maxIdx = der;
        }

        if (i != maxIdx) {
            ElementoHeap temp = heap[i];
            heap[i] = heap[maxIdx];
            heap[maxIdx] = temp;
            hundir(maxIdx);
        }
    }

public:
    MaxHeap(int capacidad) : capacidad(capacidad), tam(0) {
        heap = new ElementoHeap[capacidad];
    }

    ~MaxHeap() {
        delete[] heap;
    }

    void insertar(int dni, int prioridad) {
        if (tam == capacidad) {
            cout << "Heap lleno, no se puede insertar." << endl;
            return;
        }

        tam++;
        heap[tam - 1] = {dni, prioridad};
        flotar(tam - 1);
    }

    ElementoHeap extraerMaximo() {
        if (tam <= 0) {
            return {-1, -1};
        }

        ElementoHeap raiz = heap[0];
        heap[0] = heap[tam - 1];
        tam--;
        hundir(0);

        return raiz;
    }

    void mostrarSiguientes5() {
        cout << "Proximos 5 accesos:" << endl;
        int limite = tam < 5 ? tam : 5;
        for (int i = 0; i < limite; i++) {
            cout << i+1 << ": DNI " << heap[i].dni << " (Prioridad: " << heap[i].prioridad << ")" << endl;
        }
        if (tam == 0) {
            cout << "No hay personas en la cola." << endl;
        }
    }

    bool vacio() const {
        return tam == 0;
    }
};

class AVLAccesos {
private:
    struct Node {
        RegistroAcceso dato;
        Node* izquierda;
        Node* derecha;
        int altura;

        Node(RegistroAcceso d) : dato(d), izquierda(nullptr), derecha(nullptr), altura(1) {}
    };

    Node* raiz;

    int max(int a, int b) {
        return (a > b) ? a : b;
    }

    int altura(Node* nodo) {
        return nodo ? nodo->altura : 0;
    }

    int factorBalance(Node* nodo) {
        return nodo ? altura(nodo->izquierda) - altura(nodo->derecha) : 0;
    }

    void actualizarAltura(Node* nodo) {
        if (nodo) {
            nodo->altura = 1 + max(altura(nodo->izquierda), altura(nodo->derecha));
        }
    }

    Node* rotacionDerecha(Node* y) {
        Node* x = y->izquierda;
        Node* T2 = x->derecha;

        x->derecha = y;
        y->izquierda = T2;

        actualizarAltura(y);
        actualizarAltura(x);

        return x;
    }

    Node* rotacionIzquierda(Node* x) {
        Node* y = x->derecha;
        Node* T2 = y->izquierda;

        y->izquierda = x;
        x->derecha = T2;

        actualizarAltura(x);
        actualizarAltura(y);

        return y;
    }

    Node* insertar(Node* nodo, RegistroAcceso dato) {
        if (!nodo) return new Node(dato);

        if (dato < nodo->dato) {
            nodo->izquierda = insertar(nodo->izquierda, dato);
        } else if (dato > nodo->dato) {
            nodo->derecha = insertar(nodo->derecha, dato);
        } else {
            return nodo;
        }

        actualizarAltura(nodo);
        int balance = factorBalance(nodo);

        if (balance > 1 && dato < nodo->izquierda->dato) {
            return rotacionDerecha(nodo);
        }

        if (balance < -1 && dato > nodo->derecha->dato) {
            return rotacionIzquierda(nodo);
        }

        if (balance > 1 && dato > nodo->izquierda->dato) {
            nodo->izquierda = rotacionIzquierda(nodo->izquierda);
            return rotacionDerecha(nodo);
        }

        if (balance < -1 && dato < nodo->derecha->dato) {
            nodo->derecha = rotacionDerecha(nodo->derecha);
            return rotacionIzquierda(nodo);
        }

        return nodo;
    }

    void consultarFranjaHoraria(Node* nodo, const string& horaInicio, const string& horaFin, vector<RegistroAcceso>& resultados) {
        if (!nodo) return;

        if (nodo->dato.hora >= horaInicio) {
            consultarFranjaHoraria(nodo->izquierda, horaInicio, horaFin, resultados);
        }

        if (nodo->dato.hora >= horaInicio && nodo->dato.hora <= horaFin) {
            resultados.push_back(nodo->dato);
        }

        if (nodo->dato.hora <= horaFin) {
            consultarFranjaHoraria(nodo->derecha, horaInicio, horaFin, resultados);
        }
    }

    void contarPorZona(Node* nodo, vector<pair<string, int>>& conteo) {
        if (!nodo) return;

        bool encontrado = false;
        for (auto& item : conteo) {
            if (item.first == nodo->dato.zona) {
                item.second++;
                encontrado = true;
                break;
            }
        }
        if (!encontrado) {
            conteo.push_back(make_pair(nodo->dato.zona, 1));
        }

        contarPorZona(nodo->izquierda, conteo);
        contarPorZona(nodo->derecha, conteo);
    }

public:
    AVLAccesos() : raiz(nullptr) {}

    void insertar(RegistroAcceso dato) {
        raiz = insertar(raiz, dato);
    }

    vector<RegistroAcceso> consultarFranjaHoraria(const string& horaInicio, const string& horaFin) {
        vector<RegistroAcceso> resultados;
        consultarFranjaHoraria(raiz, horaInicio, horaFin, resultados);
        return resultados;
    }

    string zonaConMasEntradas() {
        vector<pair<string, int>> conteo;
        contarPorZona(raiz, conteo);

        string zonaMax;
        int max = 0;

        for (const auto& par : conteo) {
            if (par.second > max) {
                max = par.second;
                zonaMax = par.first;
            }
        }

        return zonaMax;
    }
};

class SistemaControlAccesos {
private:
    TablaHashPersonas tablaPersonas;
    MaxHeap colaAcceso;
    AVLAccesos registrosAcceso;
    int totalAsignados;

    string generarHoraAleatoria() {
        int hora = rand() % 24;
        int minuto = rand() % 60;
        char buffer[6];
        sprintf(buffer, "%02d:%02d", hora, minuto);
        return string(buffer);
    }

    string generarZonaAleatoria() {
        const string ZONAS[] = {"Zona_A", "Zona_B", "Zona_C", "Zona_D", "Zona_E"};
        return ZONAS[rand() % 5];
    }

    void ordenarPorHora(vector<RegistroAcceso>& registros) {
        for (size_t i = 0; i < registros.size(); i++) {
            for (size_t j = i + 1; j < registros.size(); j++) {
                if (registros[j] < registros[i]) {
                    RegistroAcceso temp = registros[i];
                    registros[i] = registros[j];
                    registros[j] = temp;
                }
            }
        }
    }

public:
    SistemaControlAccesos(int tamTablaHash, int capacidadHeap)
        : tablaPersonas(tamTablaHash), colaAcceso(capacidadHeap), totalAsignados(0) {
        srand(time(0));
    }

    void cargarParticipantes(const string& archivo) {
        ifstream file(archivo);
        if (!file.is_open()) {
            cerr << "Error al abrir el archivo: " << archivo << endl;
            return;
        }

        string linea;
        while (getline(file, linea)) {
            stringstream ss(linea);
            string nombre, zona;
            int dni, prioridad;
            char delim;

            getline(ss, nombre, ',');
            ss >> dni >> delim >> prioridad >> delim;
            getline(ss, zona);

            Persona* p = new Persona(nombre, dni, prioridad, zona);
            tablaPersonas.insertar(p);
        }
        cout << "Participantes cargados desde: " << archivo << endl;
    }

    void cargarEvento(const string& archivo) {
        ifstream file(archivo);
        if (!file.is_open()) {
            cerr << "Error al abrir el archivo: " << archivo << endl;
            return;
        }

        int dni;
        while (file >> dni) {
            Persona* p = tablaPersonas.buscar(dni);
            if (p) {
                colaAcceso.insertar(dni, p->getPrioridad());
            }
        }
        cout << "Evento cargado desde: " << archivo << endl;
    }

    void procesarAcceso(const string& zona, const string& hora) {
        if (colaAcceso.vacio()) {
            cout << "No hay personas en la cola de acceso." << endl;
            return;
        }

        auto siguiente = colaAcceso.extraerMaximo();
        Persona* p = tablaPersonas.buscar(siguiente.dni);

        if (p) {
            p->setZona(zona);
            RegistroAcceso registro(zona, hora, siguiente.dni);
            registrosAcceso.insertar(registro);

            cout << "Acceso concedido: " << p->getNombre() << " DNI: " << p->getDni()
                 << ", Prioridad: " << p->getPrioridad() << " en zona " << zona
                 << " a las " << hora << endl;
        }
    }

    void mostrarSiguientes5() {
        colaAcceso.mostrarSiguientes5();
    }

    void consultarAccesosPorHora(const string& inicio, const string& fin) {
        vector<RegistroAcceso> resultados = registrosAcceso.consultarFranjaHoraria(inicio, fin);

        cout << "\nAccesos entre " << inicio << " y " << fin << ":" << endl;
        if (resultados.empty()) {
            cout << "No se encontraron accesos en este rango horario." << endl;
            return;
        }

        ordenarPorHora(resultados);

        for (const auto& registro : resultados) {
            Persona* p = tablaPersonas.buscar(registro.dni);
            if (p) {
                cout << p->getNombre() << " DNI: " << p->getDni()
                     << " - Zona: " << registro.zona
                     << ", Hora: " << registro.hora << endl;
            }
        }
    }

    void mostrarZonaMasConcurrida() {
        string zona = registrosAcceso.zonaConMasEntradas();
        cout << "Zona con mas entradas: " << zona << endl;
    }

    void asignarAccesosAleatorios() {
        const int LOTE = 5000;
        int asignados = 0;

        while (!colaAcceso.vacio() && asignados < LOTE) {
            auto siguiente = colaAcceso.extraerMaximo();
            Persona* p = tablaPersonas.buscar(siguiente.dni);

            if (p) {
                string zona = generarZonaAleatoria();
                string hora = generarHoraAleatoria();

                p->setZona(zona);
                RegistroAcceso registro(zona, hora, siguiente.dni);
                registrosAcceso.insertar(registro);

                asignados++;
                totalAsignados++;
            }
        }

        cout << "Se han asignado " << asignados << " accesos aleatorios." << endl;
        cout << "Total acumulado: " << totalAsignados << endl;

        if (asignados == 0) {
            cout << "No quedan personas en la cola de acceso." << endl;
        }
    }
};

int main() {
    SistemaControlAccesos sistema(100000, 50000);

    //si cambia un txt nuevo o no esta en su acceso de ruta no funciona, y F todo
    sistema.cargarParticipantes("C:/Users/diego/Desktop/proyectov.1/participantes.txt");
    sistema.cargarEvento("C:/Users/diego/Desktop/proyectov.1/evento.txt");

    int opcion;
    do {
        cout << "\n=== SISTEMA DE CONTROL DE ACCESOS ===" << endl;
        cout << "1. Procesar nuevo acceso" << endl;
        cout << "2. Mostrar siguientes 5 accesos" << endl;
        cout << "3. Consultar por franja horaria" << endl;
        cout << "4. Mostrar zona mas concurrida" << endl;
        cout << "5. Asignar 5000 accesos aleatorios" << endl;
        cout << "6. Salir" << endl;
        cout << "Opcion: ";
        cin >> opcion;

        switch(opcion) {
            case 1: {
                string zona, hora;
                cout << "Ingrese zona (Zona_A-E): ";
                cin >> zona;
                cout << "Ingrese hora (HH:MM): ";
                cin >> hora;
                sistema.procesarAcceso(zona, hora);
                break;
            }
            case 2:
                sistema.mostrarSiguientes5();
                break;
            case 3: {
                string inicio, fin;
                cout << "Hora inicio (HH:MM): ";
                cin >> inicio;
                cout << "Hora fin (HH:MM): ";
                cin >> fin;
                sistema.consultarAccesosPorHora(inicio, fin);
                break;
            }
            case 4:
                sistema.mostrarZonaMasConcurrida();
                break;
            case 5:
                sistema.asignarAccesosAleatorios();
                break;
            case 6:
                cout << "Saliendo del sistema..." << endl;
                break;
            default:
                cout << "Opción no válida!" << endl;
        }
    } while(opcion != 6);

    return 0;
}

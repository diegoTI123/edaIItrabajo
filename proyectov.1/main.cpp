#include <iostream>
#include <string>
#include <vector> //arreglo automatico que crece automaticamente
#include <unordered_map> //contenedor tipo hash para buscar, insertar y borrar en tiempo constante
#include <fstream> //permite leer archivos y escribir
#include <sstream> //permite trabajar con strings con comas

using namespace std;

class Persona {
private:
    string nombre;
    int dni;
    int prioridad;
    string zona;

public:
    //constructor
    Persona(string n, int d, int p, string z) : nombre(n), dni(d), prioridad(p), zona(z) {}
        //estos son sus getters
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
        //estos son sus setters
        void setPrioridad(int p) {
            prioridad = p;
        }
        void setZona(string z) {
            zona = z;
        }
};

class TablaHashPersonas {
    private:
        pair<int, Persona*>* tabla; //almacena el valor del hash y el puntero de la persona
        int tam;
        int cantidad;
        const float factorCargaMaximo = 0.7; //es el umbral del 70%, si se pasa activa rehashing

        int funcionHash(int clave) {
            return abs(clave) % tam;
        }
        //rehashing
        void rehashing() {
            int tamViejo = tam;
            tam *= 2; //duplica el tamaño del hash antiguo
            pair<int, Persona*>* nuevaTabla = new pair<int, Persona*>[tam];

            for (int i = 0; i < tam; i++) {
                nuevaTabla[i] = make_pair(-1, nullptr);
            }
            //recorre tabla antigua, usa getdni para obtener clave y ponerlo en nueva posicion
            for (int i = 0; i < tamViejo; i++) {
                if (tabla[i].first != -1) {
                    int pos = funcionHash(tabla[i].second->getDni());
                    while (nuevaTabla[pos].first != -1) {
                        pos = (pos + 1) % tam;  //si hay colision se avanza hasta encontrar un hueco, usa lineal
                    }
                    nuevaTabla[pos] = tabla[i];
                }
            }

            delete[] tabla;
            tabla = nuevaTabla;
        }

    public:
        TablaHashPersonas(int t) : tam(t), cantidad(0) {
            tabla = new pair<int, Persona*>[tam];
            for (int i = 0; i < tam; i++) {
                tabla[i] = make_pair(-1, nullptr);
            }
        }

        ~TablaHashPersonas() {
            for (int i = 0; i < tam; i++) {
                if (tabla[i].second != nullptr) {
                    delete tabla[i].second;
                }
            }
            delete[] tabla;
        }
        //busca insertar personas
        void insertar(Persona* persona) {
            if (float(cantidad) / tam >= factorCargaMaximo) { //condicion si necesita rehashing
                rehashing();
            }

            int dni = persona->getDni();
            int pos = funcionHash(dni);
            int intentos = 0; //conteo para ver intentos y evitar ciclos infinitos

            //si tabla no esta vacia, busca si hay dni repetido, si hay devuelve error
            //sino entonces avanza a la siguiente posicion para buscar espacio
            while (tabla[pos].first != -1 && intentos < tam) {
                if (tabla[pos].second->getDni() == dni) {
                    cout << "Persona con DNI " << dni << " ya existe." << endl;
                    return;
                }
                pos = (pos + 1) % tam;
                intentos++;
            }
            //si hay posicion libre, se inserta (clave y puntero), aumenta cuenta total de insertados
            //si no hay espacio sale error que no se puedo insertar
            if (intentos < tam) {
                tabla[pos] = make_pair(funcionHash(dni), persona);
                cantidad++;
            } else {
                cout << "Error: No se pudo insertar a " << persona->getNombre() << endl;
            }
        }

        //busca devolver puntero a persona si esta en tabla
        Persona* buscar(int dni) {
            int pos = funcionHash(dni); //busca persona por dni, usa intentos para evitar loops
            int intentos = 0;
            //mientras la posicion este vacia, dni no esta en tabla, por lo que retorna null
            while (intentos < tam) {
                if (tabla[pos].first == -1) {
                    return nullptr;
                }
                if (tabla[pos].second->getDni() == dni) {  //si dni coincide, retorna puntero de persona
                    return tabla[pos].second;
                }
                pos = (pos + 1) % tam;  //si hay colision, busca siguiente pos
                intentos++;
            }

            return nullptr; //retorna null si no encuentra dni
        }
        //si encuentra dni es true
        bool validar(int dni) {
            return buscar(dni) != nullptr;
        }
        //si la pos esta vacia imprime vacio
        //sino, imprime sus datos
        void imprimir() {
            for (int i = 0; i < tam; i++) {
                cout << i << ": ";
                if (tabla[i].first == -1) {
                    cout << "Vacio" << endl;
                } else {
                    cout << tabla[i].second->getNombre() << " DNI: " << tabla[i].second->getDni()
                         << ", Prioridad: " << tabla[i].second->getPrioridad()
                         << ", Zona: " << tabla[i].second->getZona() << endl;
                }
            }
        }
    };

class MaxHeap {
    private:
        struct ElementoHeap {
            //identificador dni y valor de pos de heap
            int dni;
            int prioridad;
            //compara los prioridades para ver orden que tiene que ir
            bool operator<(const ElementoHeap& otro) const {
                return prioridad < otro.prioridad;
            }
        };
        //arreglo de heap, cap max, y tamaño del heap
        ElementoHeap* heap;
        int capacidad;
        int tam;
        //navegacion
        int padre(int i) { return (i - 1) / 2; }
        int izquierdo(int i) { return 2 * i + 1; }
        int derecho(int i) { return 2 * i + 2; }
        //si es prioridad, inserta elemento mas al inicio del heap
        void flotar(int i) {
            while (i > 0 && heap[padre(i)] < heap[i]) {
                swap(heap[i], heap[padre(i)]);
                i = padre(i);
            }
        }
        //si prioridad es baja, inserta elemento mas al final del heap
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
                swap(heap[i], heap[maxIdx]);
                hundir(maxIdx);
            }
        }

    public:
        //constructor
        MaxHeap(int capacidad) : capacidad(capacidad), tam(0) {
            heap = new ElementoHeap[capacidad];
        }
        //destructor
        ~MaxHeap() {
            delete[] heap;
        }
        //verifica espacio disponible, se añade al final del arreglo aplicando flotar si es mayor prioridad de su padre
        void insertar(int dni, int prioridad) {
            if (tam == capacidad) {
                cout << "Heap lleno, no se puede insertar." << endl;
                return;
            }

            tam++;
            heap[tam - 1] = {dni, prioridad};
            flotar(tam - 1);
        }
        //devuelve el primero en salir
        ElementoHeap extraerMaximo() {
            if (tam <= 0) {
                return {-1, -1};
            }
            //guarda raiz, reemplaza raiz con ultimo elemento del heap, reduce tamaño y llama a hundir
            //para que siga manteniendo orden el heap
            ElementoHeap raiz = heap[0];
            heap[0] = heap[tam - 1];
            tam--;
            hundir(0);

            return raiz;
        }
        //mantiene el heap sin necesidad de reconstruir todo desde 0
        void actualizarPrioridad(int dni, int nuevaPrioridad) {
            for (int i = 0; i < tam; i++) { //recorre heap
                if (heap[i].dni == dni) {
                    int viejaPrioridad = heap[i].prioridad; //guarda prioridad anterior
                    heap[i].prioridad = nuevaPrioridad; //y se actualiza la nueva
                    //si sube prioridad, se usa flotar, si baja, se usa hundir
                    if (nuevaPrioridad > viejaPrioridad) {
                        flotar(i);
                    } else {
                        hundir(i);
                    }
                    return;
                }
            }
            cout << "DNI no encontrado en el heap." << endl;
        }
        //muestra los primeros 5 de prioridad maxima
        void mostrarSiguientes5() {
            cout << "Proximos 5 accesos:" << endl;
            for (int i = 0; i < min(5, tam); i++) { //imprime los primeros 5 de prioridad
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

class RegistroAcceso {
    public:
        string zona;
        string hora;
        int dni;

        RegistroAcceso(string z, string h, int d) : zona(z), hora(h), dni(d) {}

        bool operator<(const RegistroAcceso& otro) const {
            if (zona != otro.zona) return zona < otro.zona;
            return hora < otro.hora;
        }

        bool operator>(const RegistroAcceso& otro) const {
            if (zona != otro.zona) return zona > otro.zona;
            return hora > otro.hora;
        }

        bool operator==(const RegistroAcceso& otro) const {
            return zona == otro.zona && hora == otro.hora && dni == otro.dni;
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

        void contarPorZona(Node* nodo, unordered_map<string, int>& conteo) {
            if (!nodo) return;

            conteo[nodo->dato.zona]++;
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
            unordered_map<string, int> conteo;
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

    public:
        SistemaControlAccesos(int tamTablaHash, int capacidadHeap)
            : tablaPersonas(tamTablaHash), colaAcceso(capacidadHeap) {}

        void cargarParticipantes(const string& archivo) {
            ifstream file(archivo);
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
            cout << "Participantes cargados: " << archivo << endl;
        }

        void cargarEvento(const string& archivo) {
            ifstream file(archivo);
            int dni;

            while (file >> dni) {
                Persona* p = tablaPersonas.buscar(dni);
                if (p) {
                    colaAcceso.insertar(dni, p->getPrioridad());
                }
            }
            cout << "Evento cargado: " << archivo << endl;
        }

        void procesarAcceso(const string& zona, const string& hora) {
            if (colaAcceso.vacio()) {
                cout << "No hay personas en la cola de acceso." << endl;
                return;
            }

            auto siguiente = colaAcceso.extraerMaximo();
            Persona* p = tablaPersonas.buscar(siguiente.dni);

            if (p) {
                RegistroAcceso registro(zona, hora, siguiente.dni);
                registrosAcceso.insertar(registro);
                cout << "Acceso concedido: " << p->getNombre() << "DNI: " << p->getDni()
                     << ", Prioridad: " << p->getPrioridad() << "en zona " << zona
                     << "a las " << hora << endl;
            }
        }

        void mostrarSiguientes5() {
            colaAcceso.mostrarSiguientes5();
        }

        void consultarAccesosPorHora(const string& inicio, const string& fin) {
            auto resultados = registrosAcceso.consultarFranjaHoraria(inicio, fin);

            cout << "Accesos entre " << inicio << " y " << fin << ":" << endl;
            for (const auto& registro : resultados) {
                Persona* p = tablaPersonas.buscar(registro.dni);
                if (p) {
                    cout << p->getNombre() << " DNI: " << p->getDni() << " - Zona: "
                         << registro.zona << ", Hora: " << registro.hora << endl;
                }
            }
        }

        void mostrarZonaMasConcurrida() {
            string zona = registrosAcceso.zonaConMasEntradas();
            cout << "Zona con mas entradas: " << zona << endl;
        }

        bool validarIdentidad(int dni) {
            return tablaPersonas.validar(dni);
        }

        void cargarRegistrosPrevios(const string& archivo) {
            ifstream file(archivo);
            if (!file.is_open()) {
                cerr << "Error al abrir " << archivo << endl;
                return;
            }

            string linea;
            int contador = 0;

            while (getline(file, linea)) {
                stringstream ss(linea);
                int dni;
                string zona, hora;
                char delim;

                ss >> dni >> delim;
                getline(ss, zona, ',');
                getline(ss, hora);

                if (tablaPersonas.validar(dni)) {
                    RegistroAcceso registro(zona, hora, dni);
                    registrosAcceso.insertar(registro);
                    contador++;
                } else {
                    cerr << " DNI no registrado: " << dni << endl;
                }
            }
            cout << contador << " registros previos cargados" << endl;
        }

    };

int main() {
    SistemaControlAccesos sistema(70000, 50000);

    // Cargar archivos
    sistema.cargarParticipantes("C:/Users/diego/Desktop/EXAMEN2/participantes.txt");
    sistema.cargarEvento("C:/Users/diego/Desktop/EXAMEN2/evento.txt");
    sistema.cargarRegistrosPrevios("C:/Users/diego/Desktop/EXAMEN2/arbol.txt");

    // Menú interactivo
    int opcion;
    do {
        cout << "\n=== SISTEMA DE ACCESOS ===" << endl;
        cout << "1. Procesar nuevo acceso" << endl;
        cout << "2. Mostrar siguientes 5 accesos" << endl;
        cout << "3. Consultar por franja horaria" << endl;
        cout << "4. Mostrar zona mas concurrida" << endl;
        cout << "5. Salir" << endl;
        cout << "Opcion: ";
        cin >> opcion;

        switch(opcion) {
            case 1: {
                string zona, hora;
                cout << "Ingrese zona (ej: Zona_A): ";
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
                cout << "Saliendo del sistema..." << endl;
                break;
            default:
                cout << "Opcion no valida!" << endl;
        }
    } while(opcion != 5);

    return 0;
}

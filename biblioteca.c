
/*
  Proyecto Integrador - Programación Estructurada (C)
  Sistema de Gestión de Biblioteca
  - Libros, Usuarios, Préstamos
  - Listas enlazadas (memoria dinámica)
  - Persistencia en archivos binarios
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_STR 100

typedef struct Libro {
    char isbn[20];
    char titulo[MAX_STR];
    char autor[MAX_STR];
    int anio;
    int copias;
    struct Libro* sig;
} Libro;

typedef struct Usuario {
    int id;
    char nombre[MAX_STR];
    char programa[MAX_STR];
    struct Usuario* sig;
} Usuario;

typedef struct Prestamo {
    int usuario_id;
    char isbn[20];
    char fecha[20]; // YYYY-MM-DD
    struct Prestamo* sig;
} Prestamo;

/* ====================== UTILIDADES ====================== */

static void limpiarNuevaLinea(char* s) {
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n') s[n - 1] = '\0';
}

static void leerLinea(const char* prompt, char* buf, size_t tam) {
    printf("%s", prompt);
    if (fgets(buf, (int)tam, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    limpiarNuevaLinea(buf);
}

static int leerEntero(const char* prompt, int* out) {
    char buf[64];
    leerLinea(prompt, buf, sizeof(buf));
    if (buf[0] == '\0') return 0;

    char* end = NULL;
    long val = strtol(buf, &end, 10);
    if (end == buf || *end != '\0') return 0;

    *out = (int)val;
    return 1;
}

static void fechaActual(char* out, size_t tam) {
    time_t t = time(NULL);
    struct tm* tm_info = localtime(&t);
    snprintf(out, tam, "%04d-%02d-%02d",
        tm_info->tm_year + 1900,
        tm_info->tm_mon + 1,
        tm_info->tm_mday);
}

/* ====================== BUSQUEDAS ====================== */

static Libro* buscarLibro(Libro* head, const char* isbn) {
    for (Libro* p = head; p != NULL; p = p->sig) {
        if (strcmp(p->isbn, isbn) == 0) return p;
    }
    return NULL;
}

static Usuario* buscarUsuario(Usuario* head, int id) {
    for (Usuario* p = head; p != NULL; p = p->sig) {
        if (p->id == id) return p;
    }
    return NULL;
}

static Prestamo* buscarPrestamo(Prestamo* head, int usuario_id, const char* isbn) {
    for (Prestamo* p = head; p != NULL; p = p->sig) {
        if (p->usuario_id == usuario_id && strcmp(p->isbn, isbn) == 0) return p;
    }
    return NULL;
}

/* ====================== CRUD LIBROS ====================== */

static int agregarLibro(Libro** head) {
    char isbn[20], titulo[MAX_STR], autor[MAX_STR];
    int anio, copias;

    leerLinea("ISBN: ", isbn, sizeof(isbn));
    if (isbn[0] == '\0') { printf("ISBN vacío.\n"); return 0; }
    if (buscarLibro(*head, isbn)) { printf("Error: ISBN ya existe.\n"); return 0; }

    leerLinea("Título: ", titulo, sizeof(titulo));
    leerLinea("Autor: ", autor, sizeof(autor));
    if (!leerEntero("Año: ", &anio)) { printf("Año inválido.\n"); return 0; }
    if (!leerEntero("Copias: ", &copias) || copias < 0) { printf("Copias inválidas.\n"); return 0; }

    Libro* nuevo = (Libro*)malloc(sizeof(Libro));
    if (!nuevo) { printf("Error: sin memoria.\n"); return 0; }

    strncpy(nuevo->isbn, isbn, sizeof(nuevo->isbn));
    nuevo->isbn[sizeof(nuevo->isbn) - 1] = '\0';
    strncpy(nuevo->titulo, titulo, sizeof(nuevo->titulo));
    nuevo->titulo[sizeof(nuevo->titulo) - 1] = '\0';
    strncpy(nuevo->autor, autor, sizeof(nuevo->autor));
    nuevo->autor[sizeof(nuevo->autor) - 1] = '\0';
    nuevo->anio = anio;
    nuevo->copias = copias;

    nuevo->sig = *head;
    *head = nuevo;

    printf("Libro agregado.\n");
    return 1;
}

static void listarLibros(Libro* head) {
    printf("\n--- LISTA DE LIBROS ---\n");
    if (!head) { printf("(vacío)\n"); return; }
    for (Libro* p = head; p != NULL; p = p->sig) {
        printf("ISBN: %s | %s | %s | Año: %d | Copias: %d\n",
            p->isbn, p->titulo, p->autor, p->anio, p->copias);
    }
}

static int eliminarLibro(Libro** head, const char* isbn) {
    Libro* prev = NULL, * cur = *head;
    while (cur) {
        if (strcmp(cur->isbn, isbn) == 0) {
            if (prev) prev->sig = cur->sig;
            else *head = cur->sig;
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->sig;
    }
    return 0;
}

/* ====================== CRUD USUARIOS ====================== */

static int agregarUsuario(Usuario** head) {
    int id;
    char nombre[MAX_STR], programa[MAX_STR];

    if (!leerEntero("ID usuario: ", &id) || id <= 0) { printf("ID inválido.\n"); return 0; }
    if (buscarUsuario(*head, id)) { printf("Error: ID ya existe.\n"); return 0; }

    leerLinea("Nombre: ", nombre, sizeof(nombre));
    leerLinea("Programa/Carrera: ", programa, sizeof(programa));
    if (nombre[0] == '\0') { printf("Nombre vacío.\n"); return 0; }

    Usuario* nuevo = (Usuario*)malloc(sizeof(Usuario));
    if (!nuevo) { printf("Error: sin memoria.\n"); return 0; }

    nuevo->id = id;
    strncpy(nuevo->nombre, nombre, sizeof(nuevo->nombre));
    nuevo->nombre[sizeof(nuevo->nombre) - 1] = '\0';
    strncpy(nuevo->programa, programa, sizeof(nuevo->programa));
    nuevo->programa[sizeof(nuevo->programa) - 1] = '\0';

    nuevo->sig = *head;
    *head = nuevo;

    printf("Usuario agregado.\n");
    return 1;
}

static void listarUsuarios(Usuario* head) {
    printf("\n--- LISTA DE USUARIOS ---\n");
    if (!head) { printf("(vacío)\n"); return; }
    for (Usuario* p = head; p != NULL; p = p->sig) {
        printf("ID: %d | %s | %s\n", p->id, p->nombre, p->programa);
    }
}

static int eliminarUsuario(Usuario** head, int id) {
    Usuario* prev = NULL, * cur = *head;
    while (cur) {
        if (cur->id == id) {
            if (prev) prev->sig = cur->sig;
            else *head = cur->sig;
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->sig;
    }
    return 0;
}

/* ====================== PRESTAMOS ====================== */

static int prestarLibro(Libro* libros, Usuario* usuarios, Prestamo** prestamos) {
    int uid;
    char isbn[20];

    if (!leerEntero("ID usuario: ", &uid)) { printf("ID inválido.\n"); return 0; }
    leerLinea("ISBN libro: ", isbn, sizeof(isbn));

    Usuario* u = buscarUsuario(usuarios, uid);
    if (!u) { printf("Usuario no existe.\n"); return 0; }

    Libro* l = buscarLibro(libros, isbn);
    if (!l) { printf("Libro no existe.\n"); return 0; }

    if (l->copias <= 0) { printf("No hay copias disponibles.\n"); return 0; }

    if (buscarPrestamo(*prestamos, uid, isbn)) {
        printf("Este usuario ya tiene ese libro en préstamo.\n");
        return 0;
    }

    Prestamo* nuevo = (Prestamo*)malloc(sizeof(Prestamo));
    if (!nuevo) { printf("Error: sin memoria.\n"); return 0; }

    nuevo->usuario_id = uid;
    strncpy(nuevo->isbn, isbn, sizeof(nuevo->isbn));
    nuevo->isbn[sizeof(nuevo->isbn) - 1] = '\0';
    fechaActual(nuevo->fecha, sizeof(nuevo->fecha));

    nuevo->sig = *prestamos;
    *prestamos = nuevo;

    l->copias--;
    printf("Préstamo creado. Fecha: %s\n", nuevo->fecha);
    return 1;
}

static int devolverLibro(Libro* libros, Prestamo** prestamos) {
    int uid;
    char isbn[20];

    if (!leerEntero("ID usuario: ", &uid)) { printf("ID inválido.\n"); return 0; }
    leerLinea("ISBN libro: ", isbn, sizeof(isbn));

    Prestamo* prev = NULL, * cur = *prestamos;
    while (cur) {
        if (cur->usuario_id == uid && strcmp(cur->isbn, isbn) == 0) {
            Libro* l = buscarLibro(libros, isbn);
            if (l) l->copias++;

            if (prev) prev->sig = cur->sig;
            else *prestamos = cur->sig;
            free(cur);

            printf("Devolución registrada.\n");
            return 1;
        }
        prev = cur;
        cur = cur->sig;
    }

    printf("Préstamo no encontrado.\n");
    return 0;
}

static void listarPrestamos(Prestamo* head) {
    printf("\n--- PRÉSTAMOS ACTIVOS ---\n");
    if (!head) { printf("(vacío)\n"); return; }
    for (Prestamo* p = head; p != NULL; p = p->sig) {
        printf("Usuario ID: %d | ISBN: %s | Fecha: %s\n", p->usuario_id, p->isbn, p->fecha);
    }
}

/* ====================== PERSISTENCIA ====================== */

typedef struct LibroRec {
    char isbn[20];
    char titulo[MAX_STR];
    char autor[MAX_STR];
    int anio;
    int copias;
} LibroRec;

typedef struct UsuarioRec {
    int id;
    char nombre[MAX_STR];
    char programa[MAX_STR];
} UsuarioRec;

typedef struct PrestamoRec {
    int usuario_id;
    char isbn[20];
    char fecha[20];
} PrestamoRec;

static void guardarDatos(Libro* libros, Usuario* usuarios, Prestamo* prestamos) {
    FILE* f;

    f = fopen("libros.dat", "wb");
    if (f) {
        for (Libro* p = libros; p; p = p->sig) {
            LibroRec r = { 0 };
            strncpy(r.isbn, p->isbn, sizeof(r.isbn) - 1);
            strncpy(r.titulo, p->titulo, sizeof(r.titulo) - 1);
            strncpy(r.autor, p->autor, sizeof(r.autor) - 1);
            r.anio = p->anio;
            r.copias = p->copias;
            fwrite(&r, sizeof(r), 1, f);
        }
        fclose(f);
    }

    f = fopen("usuarios.dat", "wb");
    if (f) {
        for (Usuario* p = usuarios; p; p = p->sig) {
            UsuarioRec r = { 0 };
            r.id = p->id;
            strncpy(r.nombre, p->nombre, sizeof(r.nombre) - 1);
            strncpy(r.programa, p->programa, sizeof(r.programa) - 1);
            fwrite(&r, sizeof(r), 1, f);
        }
        fclose(f);
    }

    f = fopen("prestamos.dat", "wb");
    if (f) {
        for (Prestamo* p = prestamos; p; p = p->sig) {
            PrestamoRec r = { 0 };
            r.usuario_id = p->usuario_id;
            strncpy(r.isbn, p->isbn, sizeof(r.isbn) - 1);
            strncpy(r.fecha, p->fecha, sizeof(r.fecha) - 1);
            fwrite(&r, sizeof(r), 1, f);
        }
        fclose(f);
    }

    printf("Datos guardados.\n");
}

static void liberarLibros(Libro** head) {
    while (*head) { Libro* tmp = *head; *head = (*head)->sig; free(tmp); }
}
static void liberarUsuarios(Usuario** head) {
    while (*head) { Usuario* tmp = *head; *head = (*head)->sig; free(tmp); }
}
static void liberarPrestamos(Prestamo** head) {
    while (*head) { Prestamo* tmp = *head; *head = (*head)->sig; free(tmp); }
}

static void cargarDatos(Libro** libros, Usuario** usuarios, Prestamo** prestamos) {
    FILE* f;

    liberarLibros(libros);
    liberarUsuarios(usuarios);
    liberarPrestamos(prestamos);

    f = fopen("libros.dat", "rb");
    if (f) {
        LibroRec r;
        while (fread(&r, sizeof(r), 1, f) == 1) {
            Libro* n = (Libro*)malloc(sizeof(Libro));
            if (!n) break;
            memset(n, 0, sizeof(*n));
            strncpy(n->isbn, r.isbn, sizeof(n->isbn) - 1);
            strncpy(n->titulo, r.titulo, sizeof(n->titulo) - 1);
            strncpy(n->autor, r.autor, sizeof(n->autor) - 1);
            n->anio = r.anio;
            n->copias = r.copias;
            n->sig = *libros;
            *libros = n;
        }
        fclose(f);
    }

    f = fopen("usuarios.dat", "rb");
    if (f) {
        UsuarioRec r;
        while (fread(&r, sizeof(r), 1, f) == 1) {
            Usuario* n = (Usuario*)malloc(sizeof(Usuario));
            if (!n) break;
            memset(n, 0, sizeof(*n));
            n->id = r.id;
            strncpy(n->nombre, r.nombre, sizeof(n->nombre) - 1);
            strncpy(n->programa, r.programa, sizeof(n->programa) - 1);
            n->sig = *usuarios;
            *usuarios = n;
        }
        fclose(f);
    }

    f = fopen("prestamos.dat", "rb");
    if (f) {
        PrestamoRec r;
        while (fread(&r, sizeof(r), 1, f) == 1) {
            Prestamo* n = (Prestamo*)malloc(sizeof(Prestamo));
            if (!n) break;
            memset(n, 0, sizeof(*n));
            n->usuario_id = r.usuario_id;
            strncpy(n->isbn, r.isbn, sizeof(n->isbn) - 1);
            strncpy(n->fecha, r.fecha, sizeof(n->fecha) - 1);
            n->sig = *prestamos;
            *prestamos = n;
        }
        fclose(f);
    }

    printf("Datos cargados.\n");
}

/* ====================== MEDICION RENDIMIENTO ====================== */

static void medirBusquedaLibro(Libro* libros) {
    char isbn[20];
    leerLinea("ISBN a buscar (medición): ", isbn, sizeof(isbn));

    clock_t ini = clock();
    Libro* l = buscarLibro(libros, isbn);
    clock_t fin = clock();

    double ms = (double)(fin - ini) * 1000.0 / CLOCKS_PER_SEC;
    if (l) printf("Encontrado: %s (%0.3f ms)\n", l->titulo, ms);
    else printf("No encontrado (%0.3f ms)\n", ms);
}

/* ====================== MENUS ====================== */

static void menuLibros(Libro** libros) {
    int op;
    do {
        printf("\n=== MENÚ LIBROS ===\n");
        printf("1) Agregar libro\n");
        printf("2) Listar libros\n");
        printf("3) Eliminar libro\n");
        printf("0) Volver\n");
        if (!leerEntero("Opción: ", &op)) op = -1;

        if (op == 1) agregarLibro(libros);
        else if (op == 2) listarLibros(*libros);
        else if (op == 3) {
            char isbn[20];
            leerLinea("ISBN a eliminar: ", isbn, sizeof(isbn));
            if (eliminarLibro(libros, isbn)) printf("Eliminado.\n");
            else printf("No encontrado.\n");
        }
    } while (op != 0);
}

static void menuUsuarios(Usuario** usuarios) {
    int op;
    do {
        printf("\n=== MENÚ USUARIOS ===\n");
        printf("1) Agregar usuario\n");
        printf("2) Listar usuarios\n");
        printf("3) Eliminar usuario\n");
        printf("0) Volver\n");
        if (!leerEntero("Opción: ", &op)) op = -1;

        if (op == 1) agregarUsuario(usuarios);
        else if (op == 2) listarUsuarios(*usuarios);
        else if (op == 3) {
            int id;
            if (!leerEntero("ID a eliminar: ", &id)) { printf("ID inválido.\n"); continue; }
            if (eliminarUsuario(usuarios, id)) printf("Eliminado.\n");
            else printf("No encontrado.\n");
        }
    } while (op != 0);
}

static void menuPrestamos(Libro* libros, Usuario* usuarios, Prestamo** prestamos) {
    int op;
    do {
        printf("\n=== MENÚ PRÉSTAMOS ===\n");
        printf("1) Prestar libro\n");
        printf("2) Devolver libro\n");
        printf("3) Listar préstamos\n");
        printf("0) Volver\n");
        if (!leerEntero("Opción: ", &op)) op = -1;

        if (op == 1) prestarLibro(libros, usuarios, prestamos);
        else if (op == 2) devolverLibro(libros, prestamos);
        else if (op == 3) listarPrestamos(*prestamos);
    } while (op != 0);
}

int main(void) {
    Libro* libros = NULL;
    Usuario* usuarios = NULL;
    Prestamo* prestamos = NULL;

    int op;
    do {
        printf("\n===== SISTEMA BIBLIOTECA =====\n");
        printf("1) Gestion de libros\n");
        printf("2) Gestion de usuarios\n");
        printf("3) Prestamos y devoluciones\n");
        printf("4) Guardar datos\n");
        printf("5) Cargar datos\n");
        printf("6) Medir rendimiento (busqueda libro)\n");
        printf("0) Salir\n");

        if (!leerEntero("Opcion: ", &op)) op = -1;

        if (op == 1) menuLibros(&libros);
        else if (op == 2) menuUsuarios(&usuarios);
        else if (op == 3) menuPrestamos(libros, usuarios, &prestamos);
        else if (op == 4) guardarDatos(libros, usuarios, prestamos);
        else if (op == 5) cargarDatos(&libros, &usuarios, &prestamos);
        else if (op == 6) medirBusquedaLibro(libros);

    } while (op != 0);

    guardarDatos(libros, usuarios, prestamos);
    liberarLibros(&libros);
    liberarUsuarios(&usuarios);
    liberarPrestamos(&prestamos);

    printf("Fin.\n");
    return 0;
}

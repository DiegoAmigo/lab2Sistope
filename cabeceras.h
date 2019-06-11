#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Struct para listar
struct Nodo{
	struct Nodo * siguiente;
	char * visibilidad;
};
typedef struct Nodo Nodo;

struct Monitor{
    Nodo * buffer;
    int contadorBuffer;
    int full;
    int empty;
    pthread_mutex_t mutex;
    pthread_cond_t notFull;
    pthread_cond_t notEmpty;
};
typedef struct Monitor Monitor;

struct MonitorResultados{
	Resultado ** arregloResultados;
	int full;
	pthread_mutex_t mutex;
	pthread_cond_t notFull;
};
typedef struct MonitorResultados MonitorResultados;

struct Resultado{
	float mediaReal;
	float mediaImaginaria;
	float potencia;
	float ruidoTotal;
	int hebra;
};
typedef struct Resultado Resultado;
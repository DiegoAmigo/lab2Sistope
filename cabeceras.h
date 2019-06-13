#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

//Struct para listar
struct Nodo{
	struct Nodo * siguiente;
	char * visibilidad;
};
typedef struct Nodo Nodo;

//Estructura para guardar los resultados de cada hebra
struct Resultado{
	float mediaReal;
	float mediaImaginaria;
	float potencia;
	float ruidoTotal;
};
typedef struct Resultado Resultado;

struct Monitor{
    char ** buffer;//Se guarda las visibilidades en un arreglo de strings
    Resultado * resultado;//estrucutra de resultado para el monitor
    int contadorBuffer;//Contador con la cantidad de visibilidades en el buffer
    int tamanioMaximo;//Cantidad maxima de visibilidades en el buffer
    int full;//entero que funciona como booleano para indicar si el buffer está lleno
    int notFull;//entero que funciona como booleano para indicar si el buffer está vacío
    int finished;//entero que funciona como booleano para indicar que ya no quedan más visibilidades para procesar
    int written;//entero que funciona como booleano para indicar que ya se han escritos los resultados
    int flag;//entero que funciona como booleano para indicar a la hebra si hace print de lo que procesó
    int numeroHebra;//Numero de la hebra
    pthread_mutex_t mutex;//mutex que provee la exclusión mútua para las secciones críticas
    pthread_cond_t NotFull;//Variable de conodición para que la hebra no entre a la sc si el buffer no está lleno
    pthread_cond_t Full;//Variable de conodición para que el padre no entre a la sc si el buffer está lleno
   	pthread_cond_t NotWritten;//Variable de condición para cuando los resultados no están escritos
};
typedef struct Monitor Monitor;

extern Monitor ** monitor;

void *trabajarVisibilidades(void *i);
void recibirArgumentos(int argc, char *argv[]);
void enviarVisibilidades(Nodo * inicial, int anchoDiscos, int cantDiscos, char * nombreSalida);
Nodo * leerArchivo(char * direccion);
int direccionarVisibilidad(char * visibilidad, int ancho, int ndiscos);
void salidaArchivo(char *nombreArchivo, int cantDiscos);
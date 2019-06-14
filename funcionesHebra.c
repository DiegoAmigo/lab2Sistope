#include "cabeceras.h"

void * trabajarVisibilidades(void *i){
    int numeroHebra = *(int*)i;
    float sumaReal = 0, sumaImaginaria= 0, sumaRuido= 0, sumaPotencia = 0;
    //se bloquea el monitor correspondiente a la hebra para exclusión mutua
    int maxBuf = monitor[numeroHebra]->tamanioMaximo;
    char * buffer = calloc(256, sizeof(char));
    char *pieza;
    char *pieza2;
    int iterador;
    int contadorVisibilidades = 0;
    while(!monitor[numeroHebra]->finished){
    	//se cierra mutex
    	pthread_mutex_lock(&monitor[numeroHebra]->mutex);
        //mientras el buffer no esté lleno esperar
        while(monitor[numeroHebra]->notFull){
        	pthread_cond_wait(&(monitor[numeroHebra]->NotFull), &(monitor[numeroHebra]->mutex));
        }
        iterador = 0;
        //seccion critica
        //ciclo en cual se va descontando las visibilidades del buffer y descontando el contadorBuffer 
        while (monitor[numeroHebra]->contadorBuffer>0){
            sumaReal = sumaReal + monitor[numeroHebra]->buffer[iterador]->real;
            sumaImaginaria = sumaImaginaria + monitor[numeroHebra]->buffer[iterador]->imaginario;
            sumaRuido = sumaRuido + monitor[numeroHebra]->buffer[iterador]->ruido;
            sumaPotencia = sumaPotencia + sqrtf(powf(monitor[numeroHebra]->buffer[iterador]->real,2)+powf(monitor[numeroHebra]->buffer[iterador]->imaginario,2));
            iterador++;
            monitor[numeroHebra]->contadorBuffer--;
            contadorVisibilidades++;
        }
        //señaliza que está vacio el buffer
        monitor[numeroHebra]->notFull=1;
        monitor[numeroHebra]->full=0;
        //Se libera el mutex y la variable de condición
		pthread_cond_signal(&(monitor[numeroHebra]->Full));
        pthread_mutex_unlock(&(monitor[numeroHebra]->mutex));
    }
    monitor[numeroHebra]->resultado->mediaImaginaria = sumaImaginaria/contadorVisibilidades;
    monitor[numeroHebra]->resultado->mediaReal = sumaReal/contadorVisibilidades;
    monitor[numeroHebra]->resultado->ruidoTotal=sumaRuido;
    monitor[numeroHebra]->resultado->potencia=sumaPotencia;
    if(monitor[numeroHebra]->flag){
    	printf("Soy la hebra %d, procese %d visibilidades\n",numeroHebra,contadorVisibilidades);
    	fflush(stdout);
    }
}



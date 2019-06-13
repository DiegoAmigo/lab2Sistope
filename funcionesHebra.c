#include "cabeceras.h"

void * trabajarVisibilidades(void *i){
    int numeroHebra = *(int*)i;
    float sumaReal = 0, auxReal = 0, sumaImaginaria= 0, auxImg = 0, sumaRuido= 0, auxRuido = 0, sumaPotencia = 0;
    //se bloquea el monitor correspondiente a la hebra para exclusión mutua
    int maxBuf = monitor[numeroHebra]->tamanioMaximo;
    char * buffer = calloc(256, sizeof(char));
    char *pieza;
    char *pieza2;
    int iterador;
    int contadorVisibilidades = 0;
    while(1){
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
        	buffer = strdup(monitor[numeroHebra]->buffer[iterador]);
            pieza = strtok(buffer, ",");
            pieza = strtok(NULL, ",");
            pieza = strtok(NULL, ",");
    	    auxReal = strtof(pieza, &pieza2);
            pieza = strtok(NULL, ",");
    	    auxImg = strtof(pieza, NULL);
    	    pieza = strtok(NULL, ",");
    	    auxRuido = strtof(pieza, NULL);
            sumaReal = sumaReal + auxReal;
            sumaImaginaria = sumaImaginaria + auxImg;
            sumaRuido = sumaRuido + auxRuido;
            sumaPotencia = sumaPotencia + sqrtf(powf(auxReal,2)+powf(auxImg,2));
            iterador++;
            monitor[numeroHebra]->contadorBuffer--;
            contadorVisibilidades++;
        }
        //señaliza que está vacio el buffer
        monitor[numeroHebra]->notFull=1;
        monitor[numeroHebra]->full=0;
        //Se libera el mutex y la variable de condición
        pthread_mutex_unlock(&(monitor[numeroHebra]->mutex));
		if(monitor[numeroHebra]->finished){
			break;
		}
		else{
			pthread_cond_signal(&(monitor[numeroHebra]->Full));
		}
    }
    //Se cierra mutex para escribir los resultados
    pthread_mutex_lock(&monitor[numeroHebra]->mutex);
    //sección crítica para escribir los resultados
    monitor[numeroHebra]->resultado->mediaImaginaria = sumaImaginaria/contadorVisibilidades;
    monitor[numeroHebra]->resultado->mediaReal = sumaReal/contadorVisibilidades;
    monitor[numeroHebra]->resultado->ruidoTotal=sumaRuido;
    monitor[numeroHebra]->resultado->potencia=sumaPotencia;
    monitor[numeroHebra]->written = 1;
    pthread_cond_signal(&(monitor[numeroHebra]->NotWritten));
    pthread_mutex_unlock(&(monitor[numeroHebra]->mutex));
    if(monitor[numeroHebra]->flag){
    	printf("Soy la hebra %d, procese %d visibilidades\n",numeroHebra,contadorVisibilidades);
    	fflush(stdout);
    }
}



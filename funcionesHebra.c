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
    	printf("%d cerro\n",numeroHebra);
        //mientras el buffer no esté lleno esperar
        while(monitor[numeroHebra]->notFull){
        	printf("%d me quede pegado\n",numeroHebra);
        	pthread_cond_wait(&(monitor[numeroHebra]->NotFull), &(monitor[numeroHebra]->mutex));
        }
        printf("pase %d\n",numeroHebra);
        fflush(stdout);
        iterador = 0;
        //seccion critica
        //ciclo en cual se va descontando las visibilidades del buffer y descontando el contadorBuffer 
        printf("%d por procesar por %d\n", monitor[numeroHebra]->contadorBuffer,numeroHebra);
        fflush(stdout);
        while (monitor[numeroHebra]->contadorBuffer>0){
            strcpy(buffer, monitor[numeroHebra]->buffer[iterador]);
            printf("%s por %d\n",buffer, numeroHebra);
            fflush(stdout);
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
            printf("pico %d\n",monitor[numeroHebra]->contadorBuffer);
        }
        printf("termine de procesar este ciclo %d\n",numeroHebra);
        fflush(stdout);
        //señaliza que está vacio el buffer
        monitor[numeroHebra]->notFull=1;
        monitor[numeroHebra]->full=0;
        //Se libera el mutex y la variable de condición
        pthread_mutex_unlock(&(monitor[numeroHebra]->mutex));
		if(monitor[numeroHebra]->finished){
			printf("termine %d\n",numeroHebra);
			break;
		}
		else{
			pthread_cond_signal(&(monitor[numeroHebra]->Full));
		}
    }
    printf("empece a escribir datos por %d",numeroHebra);
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
    }
}



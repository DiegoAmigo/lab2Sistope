#include "cabeceras.h"





void obtenerVisibilidades(void *i){
    int numeroHebra = (int)i;
    float sumaReal = 0, auxReal = 0, sumaImaginaria= 0, auxImg = 0, sumaRuido= 0, auxRuido = 0, sumaPotencia = 0;
    //se bloquea el monitor correspondiente a la hebra para exclusión mutua
    pthread_mutex_lock(&monitor[numeroHebra]->mutex);
    int maxBuf = monitor[numeroHebra]->tamanioMaximo;
    char * buffer = calloc(256, sizeof(char));
    char *pieza;
    char *pieza2;
    while(true){
        int i=0;
        //mientras el buffer no este lleno esperar
        while(!monitor[numeroHebra]->full){
        pthread_cond_wait(&(monitor[numeroHebra]->notFull), &(monitor[numeroHebra]->mutex));
        }
        //ciclo que se va descontando el contadorBuffer 
        while (monitor[numeroHebra]->contadorBuffer>0){
            strcpy(buffer, monitor[numeroHebra]->buffer[i]);
            //strcpy(buffer,monitor[numeroHebra]->buffer[i]->visibilidad);
            pieza = strtok(buffer, ',');
            pieza = strtok(NULL, ',');
            pieza = strtok(NULL, ',');
    	    auxReal = strtof(pieza, &pieza2);
            pieza = strtok(NULL, ",");
    	    auxImg = strtof(pieza, &pieza2);
    	    pieza = strtok(NULL, ",");
    	    auxRuido = strtof(pieza, &pieza2);
            sumaReal = sumaReal + auxReal;
            sumaImaginaria = sumaImaginaria + auxImg;
            sumaRuido = sumaRuido + auxRuido;
            sumaPotencia = sumaPotencia + sqrtf(powf(auxReal,2)+powf(auxImg,2));
            i++;
            monitor[numeroHebra]->contadorBuffer--;
        }
        //señaliza que está vacio (o se pone en el papi que no creo)
        monitor[numeroHebra]->empty=1;
        monitor[numeroHebra]->full=0;
        //se almacenan los resultados en el arreglo de resultados
        monitorResultados->arregloResultados[numeroHebra]->mediaImaginaria = sumaImaginaria/maxBuf;
        monitorResultados->arregloResultados[numeroHebra]->mediaReal = sumaReal/maxBuf;
        monitorResultados->arregloResultados[numeroHebra]->ruidoTotal=sumaRuido;
        monitorResultados->arregloResultados[numeroHebra]->potencia=sumaPotencia;
    }
    
}



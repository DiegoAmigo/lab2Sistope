#include "cabeceras.h"





void obtenerVisibilidades(void *i){
    int numeroHebra = (int)i;
    pthread_mutex_lock(monitor[numeroHebra]->mutex);
    char * buffer = calloc(256, sizeof(char));
    while(true){
        int i=0;
        while(!monitor[numeroHebra]->full){
        pthread_cond_wait(&(monitor[numeroHebra]->notFull), &(monitor[numeroHebra]->mutex));
        }
        while (monitor[numeroHebra]->contadorBuffer>0){
            strcpy(buffer,monitor[numeroHebra]->buffer[i]->visibilidad);
            i++;
            monitor[numeroHebra]->contadorBuffer--;
        }
    }
    


}


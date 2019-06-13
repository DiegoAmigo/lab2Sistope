#include "cabeceras.h"

//Función por la cual se inicia el proceso de ejecución y principalmente se leen los argumentos de entrada.
void recibirArgumentos(int argc, char *argv[]){
	int flags, opt;
	Nodo *inicio=NULL;
	printf("Hola");
	fflush(stdout);
	//Se le asigna memoria a path de archivos
	char *nombreArchivo= calloc(100,sizeof(char));
	char *nombreSalida=calloc(100,sizeof(char));
	int cantDiscos;
	int anchoDiscos;
	int tamanioBuffer;
	if(argc <11){//si se ingresa un numero de argumentos menor a 8, se finaliza la ejecucion del programa
		printf("Se ingreso un numero incorrecto de argumentos\n");
		   exit(EXIT_FAILURE);
		}
	flags = 0;
	//Haciendo uso de getopt se guardan los datos de entrada
	while((opt = getopt(argc, argv, "i:o:n:d:s:b")) != -1) { //Los argumentos deban ir acompañados por algun valor, se deben acompañar de un ":" como se puede ver en h:, en el caso de -m esto no es necesario porque no se acompaña de ningun valor
	   switch(opt) {
	   case 'i':
		   strcpy(nombreArchivo,optarg);
		   break;
	   case 'o': 
	    	strcpy(nombreSalida,optarg);
		   break;
		case 'n':
			sscanf(optarg, "%i", &cantDiscos);
			break;
		case 'd':
			sscanf(optarg, "%i", &anchoDiscos);
			break;
		case 's':
			sscanf(optarg, "%i", &tamanioBuffer);
			break;
		case 'b':
			flags=1;
			break;
	   }
	}
	monitor = (Monitor **)calloc(cantDiscos,sizeof(Monitor *));
	int i;
	int j;
	//Se inicializan los n monitores para controlar las n hebras
	for(i = 0; i<cantDiscos; i++){
		//Se le da memoria a cada monitor
		monitor[i] = (Monitor *)malloc(sizeof(Monitor));
		//Se inicializan los atributos del monitor
		monitor[i]->contadorBuffer = 0;
		monitor[i]->buffer = (char **)calloc(tamanioBuffer,sizeof(char *));
		for(j = 0; j<tamanioBuffer; j++){
			monitor[i]->buffer[j] = (char *)calloc(256,sizeof(char));
		}
		monitor[i]->tamanioMaximo = tamanioBuffer;
		monitor[i]->full = 0;
		monitor[i]->notFull = 1;
		monitor[i]->finished = 0;
		monitor[i]->written = 0;
		monitor[i]->flag = flags;
		monitor[i]->numeroHebra = i;
		monitor[i]->resultado = (Resultado *)malloc(sizeof(Resultado));
		pthread_mutex_init(&(monitor[i]->mutex),NULL);
		pthread_cond_init(&(monitor[i]->NotFull),NULL);
		pthread_cond_init(&(monitor[i]->Full),NULL);
		pthread_cond_init(&(monitor[i]->NotWritten),NULL);
	}
	//Se inicializa el monitor para controlar la escritura de resultados
	pthread_t * hebras = (pthread_t *)calloc(cantDiscos, sizeof(pthread_t));
	for(i = 0; i<cantDiscos;i++){
		pthread_create(&hebras[i],NULL,trabajarVisibilidades,(void *)&monitor[i]->numeroHebra);
	}
	//Se llama a función leer archivo
	inicio=leerArchivo(nombreArchivo);
	enviarVisibilidades(inicio, anchoDiscos, cantDiscos, nombreSalida);
	salidaArchivo(nombreSalida, cantDiscos);
}

void enviarVisibilidades(Nodo * inicial, int anchoDiscos, int cantDiscos, char * nombreSalida){
	Nodo * actual = inicial;
	int numeroHebra;
	int contador = 0;
	while(actual != NULL){
		numeroHebra = direccionarVisibilidad(actual->visibilidad, anchoDiscos, cantDiscos);
		//Se cierra mutex
		pthread_mutex_lock(&monitor[numeroHebra]->mutex);

		//Se verifica que el buffer de la hebra a la cual será direccionada la visibilidad no esté llena
		while(monitor[numeroHebra]->full){
        	pthread_cond_wait(&(monitor[numeroHebra]->Full), &(monitor[numeroHebra]->mutex));
        }
        //Sección crítica
        strcpy(monitor[numeroHebra]->buffer[monitor[numeroHebra]->contadorBuffer],actual->visibilidad);
        pthread_mutex_unlock(&(monitor[numeroHebra]->mutex));
        monitor[numeroHebra]->contadorBuffer++;
        fflush(stdout);
        printf("%d\n",contador);
        if(monitor[numeroHebra]->contadorBuffer == monitor[numeroHebra]->tamanioMaximo){
        	monitor[numeroHebra]->full = 1;
        	monitor[numeroHebra]->notFull = 0;
        	pthread_cond_signal(&(monitor[numeroHebra]->NotFull));
        }
        contador++;
        actual = actual->siguiente;
	}
	int i;
	//Se libera la variable de condición que impide a las hebras entrar a la sc si no está lleno el buffer, esto debido que en este punto ya no quedan visibilidades que enviar y puede
	//que los buffer contengan datos que no han sido leídos por las hebras
	for(i = 0; i<cantDiscos; i++){
		//Además se setea la variable finished como verdadera, para que las hebras dejen de leer visibilidades y empiecen a escribir los resultados
		monitor[i]->finished = 1;
		monitor[i]->full = 1;
        monitor[i]->notFull = 0;
		pthread_cond_signal(&(monitor[i]->NotFull));
	}
	salidaArchivo(nombreSalida, cantDiscos);
	//se hace free de los monitores
	int j;
	for(i = 0; i<cantDiscos;i++){
		for(j = 0; j<monitor[i]->tamanioMaximo; j++){
			free(monitor[i]->buffer[j]);
		}
		free(monitor[i]->buffer);
		free(monitor[i]->resultado);
		free(monitor[i]);
	}
	free(monitor);
	//free a la lista
	Nodo * aux;
	while(inicial != NULL){
		aux = inicial;
		inicial = inicial->siguiente;
		free(aux->visibilidad);
		free(aux);
	}
	exit(0);
}


//Función que lee el archivo con los datos del espacio de Fourier
Nodo * leerArchivo(char * direccion){
	Nodo * inicial= NULL;
	Nodo * aux;	
	FILE* fp;
	char buffer[255];
	fp = fopen(direccion, "r");
	char * pend;
	//Se lee linea a linea
	while(fgets(buffer, 255, (FILE*) fp)) {
		//se asigna memoria al nuevo nodo
		aux = (Nodo *)malloc(sizeof(Nodo));
		aux->siguiente = NULL;
		//se asigna memoria para la linea (visibilidad) que contendrá el nodo
		aux->visibilidad = (char *)calloc(256,sizeof(char));
		if(buffer[strlen(buffer)-1]=='\n'){
			buffer[strlen(buffer)-1] = '\0';
		}
		strcpy(aux->visibilidad, buffer);
		//el nuevo nodo se agrega a la lista (se agrega como si fuera una pila para optimizar)
    	if(inicial == NULL){
    		inicial = aux;
    	}
    	else{
    		aux->siguiente = inicial;
    		inicial = aux;
    	}
	}
	fclose(fp);
	int contador = 0;
	aux = inicial;
	while(aux != NULL){
		contador++;
		aux = aux->siguiente;
	}
	return inicial;
}

//Función para calcular la hebra al cual será envíado una visibilidad, se retorna el indice del monitor correspondiente a dicho hijo
int direccionarVisibilidad(char * visibilidad, int ancho, int ndiscos){
	char * token;
	strcpy(token,visibilidad);
	char * pend;
	token = strtok(token,",");
	//Se obtiene la coordenada U de la visibilidad
	float coordenadaU = strtof(token,&pend);
	token = strtok(NULL,",");
	//Se obtiene la coordenada V de la visibilidad
	float coordenadaV = strtof(token,NULL);
	//Se aplica la formula de la distancia, raíz de la suma de potencias de ambas coordenadas
	float distancia = sqrtf(powf(coordenadaU,2)+powf(coordenadaV,2));
	//EL indice final, se calcula dividiendo la distancia calculada por el ancho de disco.
	int indice = (int)distancia/ancho;
	//Los indices que superan el valor del último intervalo de distancias son asignados al último disco
	if(indice>=ndiscos-1){
		return ndiscos-1;
	}
	else{
		return indice;
	}
}

//Función para escribir el archivo de salida.
void salidaArchivo(char *nombreArchivo, int cantDiscos){
	FILE *salida=fopen(nombreArchivo, "w");
	int i;
	for(i=0;i<cantDiscos;i++){
		//Se cierra mutex de la hebra i
		pthread_mutex_lock(&monitor[i]->mutex);
		//mientras los resultados no esten escritos esperar
        while(!monitor[i]->written){
        	pthread_cond_wait(&(monitor[i]->NotWritten), &(monitor[i]->mutex));
        }
		fprintf(salida, "%s %i\n%s %f\n", "Disco", i+1, "Media Real: ", monitor[i]->resultado->mediaReal);
		fprintf(salida, "%s %f\n", "Media Imaginaria: ", monitor[i]->resultado->mediaImaginaria);
		fprintf(salida, "%s %f\n", "Potencia: ", monitor[i]->resultado->potencia);
		fprintf(salida, "%s %f\n", "Ruido Total: ", monitor[i]->resultado->ruidoTotal);
		//Se libera mutex de la hebra i
		pthread_mutex_unlock(&monitor[i]->mutex);
	}
	fclose(salida);
}
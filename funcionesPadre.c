#include "cabeceras.h"

//Función por la cual se inicia el proceso de ejecución y principalmente se leen los argumentos de entrada.
void recibirArgumentos(int argc, char *argv[]){
	int flags, opt;
	Nodo *inicio=NULL;
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
			monitor[i]->buffer[i] = (char *)calloc(256,sizeof(char));
		}
		monitor[i]->tamanioMaximo = tamanioBuffer;
		monitor[i]->full = 0;
		monitor[i]->empty = 1;
		pthread_mutex_init(&(monitor[i]->mutex),NULL);
		pthread_cond_init(&(monitor[i]->notFull),NULL);
		pthread_cond_init(&(monitor[i]->notEmpty),NULL);
	}
	//Se inicializa el monitor para controlar la escritura de resultados
	pthread_t * hebras = (pthread_t *)calloc(cantDiscos, sizeof(pthread_t));
	for(i = 0; i<cantDiscos;i++){
		pthread_create(&hebras[i],NULL,obtenerVisibilidades,(void *)i);
	}
	//Se llama a función leer archivo
	inicio=leerArchivo(nombreArchivo);
	int i = 0;
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
	return inicial;
}

//Función para calcular el hijo al cual será envíado una visibilidad, se retorna el indice del pipe correspondiente a dicho hijo
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
void salidaArchivo(char *nombreArchivo, Resultado **resultado){
	FILE *salida=fopen(nombreArchivo, "w");
	for(int i=0;i<cantDiscos;i++){
		fprintf(salida, "%s %i\n%s %f\n", "Disco", resultado[i]->hebra+1, "Media Real: ", resultado[i]->mediaReal);
		fprintf(salida, "%s %f\n", "Media Imaginaria: ", resultado[i]->mediaImaginaria);
		fprintf(salida, "%s %f\n", "Potencia: ", resultado[i]->potencia);
		fprintf(salida, "%s %f\n", "Ruido Total: ", resultado[i]->ruidoTotal);
	}
	fclose(salida);
}
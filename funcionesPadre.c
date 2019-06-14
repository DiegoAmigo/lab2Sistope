#include "cabeceras.h"

pthread_t * hebras;

/*	Entradas: argumentos que se reciben al momento de ejecutar el código en la consola, nombre de archivo con las visibilidades, nombre de archivo de salida, cantidad de discos, ancho de cada disco, tamaño del buffer y una bandera
	Funcionamiento: está función inicia el proceso de ejecución leyendo los argumentos que se ingresan, se comprueban el numero de argumentos ingresados, inicializa los monitores acorde a la cantidad de discos que se ingresaron
					inicia la lectura de archivos, inicia el calculo de visibilidades pasandole una cantidad fija de visibilidades acorde a lo ingresado como parametro, llama a la funcion para crear y escribir el archivo de salida
					y libera la memoria ocupada en monitores
	Salidas: ninguna debido a que es void.
 */

//Función por la cual se inicia el proceso de ejecución y principalmente se leen los argumentos de entrada.
void recibirArgumentos(int argc, char *argv[]){
	int flags, opt;
	Nodo *inicio=NULL;
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
		monitor[i]->buffer = (Visibilidad **)calloc(tamanioBuffer,sizeof(Visibilidad *));
		monitor[i]->tamanioMaximo = tamanioBuffer;
		monitor[i]->full = 0;
		monitor[i]->notFull = 1;
		monitor[i]->finished = 0;
		monitor[i]->flag = flags;
		monitor[i]->numeroHebra = i;
		monitor[i]->resultado = (Resultado *)malloc(sizeof(Resultado));
		pthread_mutex_init(&(monitor[i]->mutex),NULL);
		pthread_cond_init(&(monitor[i]->NotFull),NULL);
		pthread_cond_init(&(monitor[i]->Full),NULL);
	}
	//Se inicializa el monitor para controlar la escritura de resultados
	hebras = (pthread_t *)calloc(cantDiscos, sizeof(pthread_t));
	for(i = 0; i<cantDiscos;i++){
		pthread_create(&hebras[i],NULL,trabajarVisibilidades,(void *)&monitor[i]->numeroHebra);
	}
	//Se llama a función leer archivo
	inicio=leerArchivo(nombreArchivo);
	enviarVisibilidades(inicio, anchoDiscos, cantDiscos, nombreSalida);
	salidaArchivo(nombreSalida, cantDiscos);
	//se hace free de los monitores
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
	while(inicio != NULL){
		aux = inicio;
		inicio = inicio->siguiente;
		free(aux->visibilidad);
		free(aux);
	}
}

/*	Entradas: El nodo inicial que sería la estructura para el buffer y listar las visibilidades, el ancho del disco, la cantidad de discos (hebras) y el nombre de el archivo de salida
	Funcionamiento: obteniendo el numero de hebra, se llena el buffer de la estructura monitor de la hebra hasta que no queden visibilidades que enviar, esto se hace verificando
					que el buffer de la hebra no esté llena.
	Salidas: ninguna en especial debido al retorno void.

 */


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
        monitor[numeroHebra]->buffer[monitor[numeroHebra]->contadorBuffer] = actual->visibilidad;
        monitor[numeroHebra]->contadorBuffer++;
        if(monitor[numeroHebra]->contadorBuffer == monitor[numeroHebra]->tamanioMaximo){
        	monitor[numeroHebra]->full = 1;
        	monitor[numeroHebra]->notFull = 0;
        	pthread_cond_signal(&(monitor[numeroHebra]->NotFull));
        }
        pthread_mutex_unlock(&(monitor[numeroHebra]->mutex));
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
}


/* Entradas: la dirección o nombre del archivo a leer
	Funcionamiento: lee el archivo que contiene los datos del espacio de fourier y guarda los contenidos en una lista que contendrá todas las visibilidades leidas del archivo,
					esta estructura visibilidades contiene por separado cada dato de la linea (coordenadas u y v, parte real, parte imaginaria y ruido).
	Salida: un nodo que es la lista con toda la información leida del archivo de entrada.

 */

//Función que lee el archivo con los datos del espacio de Fourier
Nodo * leerArchivo(char * direccion){
	Nodo * inicial= NULL;
	Nodo * aux;	
	FILE* fp;
	char buffer[255];
	fp = fopen(direccion, "r");
	char * pend;
	char * pieza;
	char * pieza2;
	//Se lee linea a linea
	while(fgets(buffer, 255, (FILE*) fp)) {
		//se asigna memoria al nuevo nodo
		aux = (Nodo *)malloc(sizeof(Nodo));
		aux->siguiente = NULL;
		//se asigna memoria para la linea (visibilidad) que contendrá el nodo
		aux->visibilidad = (Visibilidad *)malloc(sizeof(Visibilidad));
		pieza = strtok(buffer, ",");
		aux->visibilidad->U = strtof(pieza, &pieza2);
        pieza = strtok(NULL, ",");
        aux->visibilidad->V = strtof(pieza, NULL);
        pieza = strtok(NULL, ",");
   	    aux->visibilidad->real = strtof(pieza, NULL);
   	    pieza = strtok(NULL, ",");
   	    aux->visibilidad->imaginario = strtof(pieza, NULL);
        pieza = strtok(NULL, ",");
        aux->visibilidad->ruido = strtof(pieza,NULL);
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

/* Entrada: la estructura visibilidad dodne están los datos de cada linea, el ancho de los discos y el numero de discos
	Funcionamiento: calcula a que hebra le será enviada la visibilidad obtenida del archivo de entrada
	Salida: retorna el indice del monitor correspondiente a dicha hebra que se selecciona (numero del monitor es el mismo al numero de la hebra)

 */
//Función para calcular la hebra al cual será envíado una visibilidad, se retorna el indice del monitor correspondiente a dicho hijo
int direccionarVisibilidad(Visibilidad * visibilidad, int ancho, int ndiscos){
	//Se aplica la formula de la distancia, raíz de la suma de potencias de ambas coordenadas
	float distancia = sqrtf(powf(visibilidad->U,2)+powf(visibilidad->V,2));
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

/*Entrada: nombre de archivo de salida y la cantidad de discos
	Funcionamiento: se lee la estructura global que contiene el resultado final de todas las operaciones de las hebras y la escribe en el archivo de salida
	Salidas: ninguna en especifico debido al tipo void.

 */



//Función para escribir el archivo de salida.
void salidaArchivo(char *nombreArchivo, int cantDiscos){
	FILE *salida=fopen(nombreArchivo, "w");
	int i;
	for(i=0;i<cantDiscos;i++){
		//Se esperan los resultados 
		pthread_join(hebras[i], NULL);
		fprintf(salida, "%s %i\n%s %f\n", "Disco", i+1, "Media Real: ", monitor[i]->resultado->mediaReal);
		fprintf(salida, "%s %f\n", "Media Imaginaria: ", monitor[i]->resultado->mediaImaginaria);
		fprintf(salida, "%s %f\n", "Potencia: ", monitor[i]->resultado->potencia);
		fprintf(salida, "%s %f\n", "Ruido Total: ", monitor[i]->resultado->ruidoTotal);
		//Se libera mutex de la hebra i
		pthread_mutex_unlock(&monitor[i]->mutex);
	}
	fclose(salida);
}
#include "snakePiLib.h"

//------------------------------------------------------
// MENSAJES POR PANTALLA
//------------------------------------------------------
	void MensajeBienvenida(){ //Mensaje que se muestra al iniciar el juego
		piLock (STD_IO_BUFFER_KEY);
		printf("\n");
		printf("A - IZQUIERDA\n");
		printf("E - ARRIBA\n");
		printf("D - DERECHA\n");
		printf("C - ABAJO\n");
		printf("6 - PAUSAR JUEGO\n");
		printf("7 - REANUDAR JUEGO\n");
		printf("\n");
		printf("PULSE F PARA COMENZAR EL JUEGO...\n");
		printf("\n");
		fflush(stdout);
		piUnlock (STD_IO_BUFFER_KEY);
	}

void MensajeGameOver(fsm_t* this){ //Mensaje que se muestra cuando el jugador pierde una partida

	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	int puntos = p_snakePi->serpiente.score; //Se guarda en puntos la puntuación obtenida por el jugador
	int pausas = p_snakePi->serpiente.pause; //Se guarda en pausas las veces que el jugador ha pausado el juego durante la partida

	piLock (STD_IO_BUFFER_KEY);
	printf("\n");
	printf("\nGAME OVER :(");
	printf("\n");
	printf("\nSU PUNTUACIÓN HA SIDO: %i", puntos);
	printf("\nHA PAUSADO %i VECES EL JUEGO", pausas);
	printf("\n");
	printf("\nPULSE LA TECLA F PARA RESETEAR EL JUEGO\n");
	fflush(stdout);
	piUnlock (STD_IO_BUFFER_KEY);

}
	void MensajeReinicio(){ //Mensaje que se muestra cuando se reincia el juego

		piLock (STD_IO_BUFFER_KEY);
		printf("\n");
		printf("\nELIGA EL NIVEL DE DIFICULTAD\n"); //Menú para elegir el nivel de dificultad
		printf("\n");
		printf("1 - DIFICULTAD FACIL\n");
		printf("2 - DIFICULTAD MEDIA\n");
		printf("3 - DIFICULTAD DIFICIL\n");
		printf("4 - DIFICULTAD INCREMENTAL\n");
		printf("\n");
		fflush(stdout);
		piUnlock (STD_IO_BUFFER_KEY);
	}

//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------

void InicializaManzana(tipo_manzana *p_manzana) {
	// Aleatorizamos la posicion inicial de la manzana
	p_manzana->x = rand() % NUM_COLUMNAS_DISPLAY;
	p_manzana->y = rand() % NUM_FILAS_DISPLAY;
}

void InicializaSerpiente(tipo_serpiente *p_serpiente) {
	// Nos aseguramos de que la serpiente comienza sin p_cola
	LiberaMemoriaCola(p_serpiente);

	// Inicializamos la posicion inicial de la cabeza al comienzo del juego
	p_serpiente->p_cola->x=3;
	p_serpiente->p_cola->y=3;
	p_serpiente->direccion = ARRIBA;
	p_serpiente->score = 0;
	p_serpiente->pause = 0;
	//p_serpiente->velocidad_incremental = 0;
}

void InicializaSnakePi(tipo_snakePi *p_snakePi) {
	// Modelamos la serpiente como una p_cola de segmentos
	// Inicialmente la serpiente consta de un unico segmento: la cabeza
	// Actualizamos la p_cola para que incluya a la cabeza
	p_snakePi->serpiente.p_cola = &(p_snakePi->serpiente.cabeza);
	p_snakePi->serpiente.p_cola->p_next = NULL;

	ResetSnakePi(p_snakePi);

	ActualizaPantallaSnakePi(p_snakePi);

	piLock (STD_IO_BUFFER_KEY);

	printf("\n");
	printf("\nCOMIENZA EL JUEGO!!!\n");
	fflush(stdout);

	PintaPantallaPorTerminal((tipo_pantalla*)(p_snakePi->p_pantalla));

	piUnlock (STD_IO_BUFFER_KEY);
}

void ResetSnakePi(tipo_snakePi *p_snakePi) {
	InicializaSerpiente(&(p_snakePi->serpiente));
	InicializaManzana(&(p_snakePi->manzana));
}

//------------------------------------------------------
// PROCEDIMIENTOS PARA LA GESTION DEL JUEGO
//------------------------------------------------------

void ActualizaColaSerpiente(tipo_serpiente *p_serpiente) {
	tipo_segmento *seg_i;

	// Recorremos los diferentes segmentos de que consta la serpiente
	// desde el comienzo de la cola hacia el final haciendo que cada segmento pase
	// a ocupar la posicion del que le precedia en la cola
	for(seg_i = p_serpiente->p_cola; seg_i->p_next; seg_i=seg_i->p_next) {
		seg_i->x = seg_i->p_next->x;
		seg_i->y = seg_i->p_next->y;
	}
}

int ActualizaLongitudSerpiente(tipo_serpiente *p_serpiente) {
	tipo_segmento *nueva_cola;

	nueva_cola = malloc(sizeof(tipo_segmento));

	if (!nueva_cola) {
		printf("[ERROR!!!][PROBLEMAS DE MEMORIA!!!]\n");
		return 0;
	}

	nueva_cola->x = p_serpiente->p_cola->x;
	nueva_cola->y = p_serpiente->p_cola->y;
	nueva_cola->p_next = p_serpiente->p_cola;

	p_serpiente->p_cola = nueva_cola;

	return 1;
}


	/*
	 * Metodo que actualiza la snakePi
	 * Comprueba si se ha comido una manzana, en caso de que se la coma añade una nueva
	 * Actualiza la longitud de la serpiente
	 * Y se cambia la dirección de movimiento de la serpiente
	 */
	int ActualizaSnakePi(tipo_snakePi *p_snakePi) {

		ActualizaColaSerpiente(&(p_snakePi->serpiente)); //Actualizamos la cola de la serpiente

		if (CompruebaColision(&(p_snakePi->serpiente), &(p_snakePi->manzana), 1)) {

			p_snakePi->serpiente.score ++; //Se incrementa la puntuación si conseguimos una manzana

			//Si nos encontramos en el modo de velocidad incremental y el numero de manzanas conseguidas es multiplo de 5
			if((((p_snakePi->serpiente.score) % 5)==0)&&((p_snakePi->serpiente.velocidad_incremental)==1)){

				//Se reduce a la mitad el timeout de refesco del snakePi
				p_snakePi->serpiente.timeout_dificultad = p_snakePi->serpiente.timeout_dificultad / 2;

			}

			// Colision con manzana, nos comemos la manzana y la serpiente crece
			if(!ActualizaLongitudSerpiente(&(p_snakePi->serpiente)))
				return 0;

			// Añadimos una nueva manzana asegurandonos de que no aparezca en una posicion ya ocupada por la serpiente
			while (CompruebaColision(&(p_snakePi->serpiente), &(p_snakePi->manzana), 1)) {
				InicializaManzana(&(p_snakePi->manzana));//Se añade una nueva manzana
			}
		}

		switch (p_snakePi->serpiente.direccion) {
			case ARRIBA:
				p_snakePi->serpiente.cabeza.y--;
				break;
			case DERECHA:
				p_snakePi->serpiente.cabeza.x++;
				break;
			case ABAJO:
				p_snakePi->serpiente.cabeza.y++;
				break;
			case IZQUIERDA:
				p_snakePi->serpiente.cabeza.x--;
				break;
			case NONE:
				break;
		}

		return 1;
	}

void CambiarDireccionSerpiente(tipo_serpiente *serpiente, enum t_direccion direccion) {
	switch (direccion) {
		case ARRIBA:
			// No puedo cambiar de sentido! Me como!
			if (serpiente->direccion != ABAJO)
				serpiente->direccion = ARRIBA;
			break;
		case DERECHA:
			// No puedo cambiar de sentido! Me como!
			if (serpiente->direccion != IZQUIERDA)
				serpiente->direccion = DERECHA;
			break;
		case ABAJO:
			// No puedo cambiar de sentido! Me como!
			if (serpiente->direccion != ARRIBA)
				serpiente->direccion = ABAJO;
			break;
		case IZQUIERDA:
			// No puedo cambiar de sentido! Me como!
			if (serpiente->direccion != DERECHA)
				serpiente->direccion = IZQUIERDA;
			break;
		default:
			printf("[ERROR!!!!][Direccion NO VALIDA!!!!][%d]", direccion);
			break;
	}
}

int CompruebaColision(tipo_serpiente *serpiente, tipo_manzana *manzana, int comprueba_manzana) {
	tipo_segmento *seg_i;

	if (comprueba_manzana) {
		// Para todos los elementos de la p_cola...
		for (seg_i = serpiente->p_cola; seg_i; seg_i=seg_i->p_next) {
			// ...compruebo si alguno ha "colisionado" con la manzana
			if (seg_i->x == manzana->x && seg_i->y == manzana->y)

				return 1;

			}

		return 0;
	}
	else {
		// Compruebo si la cabeza de la serpiente colisiona con cualquier otro segmento de la cola...
		for(seg_i = serpiente->p_cola; seg_i->p_next; seg_i=seg_i->p_next) {
			if (serpiente->cabeza.x == seg_i->x && serpiente->cabeza.y == seg_i->y)
				return 1;
		}

		// Compruebo si la cabeza de la serpiente rebasa los limites del area de juego...
		if (serpiente->cabeza.x < 0 || serpiente->cabeza.x >= NUM_COLUMNAS_DISPLAY ||
			serpiente->cabeza.y < 0 || serpiente->cabeza.y >= NUM_FILAS_DISPLAY) {
			return 1;
		}

		return 0;
	}
}

void LiberaMemoriaCola(tipo_serpiente *p_serpiente) {
	tipo_segmento *seg_i;
	tipo_segmento *next_tail;

	seg_i = p_serpiente->p_cola;

	while (seg_i->p_next) {
		next_tail=seg_i->p_next;
		free(seg_i);
		seg_i=next_tail;
	}

	p_serpiente->p_cola=seg_i;
	p_serpiente->p_cola->p_next = NULL;
}

//------------------------------------------------------
// PROCEDIMIENTOS PARA LA VISUALIZACION DEL JUEGO
//------------------------------------------------------

void PintaManzana(tipo_snakePi *p_snakePi) {
	p_snakePi->p_pantalla->matriz[p_snakePi->manzana.x][p_snakePi->manzana.y] = 1;
}

void PintaSerpiente(tipo_snakePi *p_snakePi) {
	tipo_segmento *seg_i;

	for(seg_i = p_snakePi->serpiente.p_cola; seg_i->p_next; seg_i=seg_i->p_next) {
		p_snakePi->p_pantalla->matriz[seg_i->x][seg_i->y] = 1;
	}

	p_snakePi->p_pantalla->matriz[seg_i->x][seg_i->y] = 1;
}

void ActualizaPantallaSnakePi(tipo_snakePi *p_snakePi) {

	ReseteaPantallaSnakePi((tipo_pantalla*)(p_snakePi->p_pantalla));

	PintaSerpiente(p_snakePi);

	PintaManzana(p_snakePi);

}

void ReseteaPantallaSnakePi(tipo_pantalla *p_pantalla) {
	int i;
	int j;
	for(i=0; i<NUM_FILAS_DISPLAY; i++){
		for(j=0; j<NUM_COLUMNAS_DISPLAY; j++){
			p_pantalla->matriz[j][i]=0;
		}
	}
}

//------------------------------------------------------
// FUNCIONES DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaBotonPulsado (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_BOTON),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}
	/*
	 * Metodo de comprobación el flag de dificultad fácil
	 */
	int CompruebaDificultadFacil(fsm_t* this) {

		int result = 0;

		piLock(SYSTEM_FLAGS_KEY);
		result = (flags & FLAG_FACIL),
		piUnlock (SYSTEM_FLAGS_KEY);

		return result;
	}

	/*
	 * Metodo de comprobación el flag de dificultad media
	 */
	int CompruebaDificultadMedia(fsm_t* this) {

		int result = 0;

		piLock(SYSTEM_FLAGS_KEY);
		result = (flags & FLAG_MEDIO),
		piUnlock (SYSTEM_FLAGS_KEY);

		return result;
	}

/*
 * Metodo de comprobación el flag de dificultad dificil
 */
int CompruebaDificultadDificil(fsm_t* this) {

	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_DIFICIL),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}

/*
 * Metodo de comprobación el flag de modo con velocidad incremental
 */
int CompruebaDificultadIncremental(fsm_t* this) {

	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_INCREMENTAL),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}

/*
 * Metodo de comprobación el flag de pausar juego
 */
int CompruebaPausa (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_PAUSA),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}


/*
 * Metodo de comprobación el flag de reanudar el juego
 */
int CompruebaReanudar (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_REANUDAR),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaMovimientoArriba(fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_MOV_ARRIBA),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaMovimientoAbajo(fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_MOV_ABAJO),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaMovimientoIzquierda(fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_MOV_IZQUIERDA),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaMovimientoDerecha(fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_MOV_DERECHA),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaTimeoutActualizacionJuego (fsm_t* this) {
	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_TIMER_JUEGO),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}

int CompruebaFinalJuego(fsm_t* this) {

	int result = 0;

	piLock(SYSTEM_FLAGS_KEY);
	result = (flags & FLAG_FIN_JUEGO),
	piUnlock (SYSTEM_FLAGS_KEY);

	return result;
}

//------------------------------------------------------
// FUNCIONES DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

/*
 * Inicializa el juego
 * Se desactiva el flag de boton
 * Se crea el temporizador
 */
void InicializaJuego(fsm_t* this) {
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock (SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_BOTON);
	piUnlock (SYSTEM_FLAGS_KEY);

	InicializaSnakePi(p_snakePi);

	//Temporizador con el tiempo de refresco establecido en el modo de dificultad

	tmr_startms(p_snakePi->tmr_refresco_snake, p_snakePi->serpiente.timeout_dificultad);
	CaritaContenta(this); //Se muestra la pantalla inicial
	pseudoWiringPiEnableDisplay(1); //Se habilita el display
}

/*
 * Metodo para pausar el juego
 */
void PausaJuego (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock (SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_PAUSA); //Se desactiva el flag de pausa
	piUnlock (SYSTEM_FLAGS_KEY);

	p_snakePi->serpiente.pause ++;

	tmr_startms(p_snakePi->tmr_refresco_snake, 0); //Se desactiva el temporizador, inicializandolo a 0

}

/*
* Metodo para reanudar el juego
*/
void ReanudarJuego (fsm_t* this) {
tipo_snakePi* p_snakePi;
p_snakePi = (tipo_snakePi*)(this->user_data);

piLock (SYSTEM_FLAGS_KEY);
flags &= (~ FLAG_REANUDAR);
piUnlock (SYSTEM_FLAGS_KEY);

//Se vuelve a iniciar el temporizador de refresco del snakePi
tmr_startms(p_snakePi->tmr_refresco_snake, p_snakePi->serpiente.timeout_dificultad);

}

/*
 * Se encarga de establecer la dificultad facil al juego
 * Su función principal es establecer un tiempo de refresco alto
 */
void ActualizaDificultadFacil (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_FACIL); //Desactiva el flag de dificultad facil
	piUnlock (SYSTEM_FLAGS_KEY);

	//Establece el tiempo de refresco del juego igual a TIMEOUT_FACIL (2000)
	p_snakePi->serpiente.timeout_dificultad = TIMEOUT_FACIL;

	piLock (STD_IO_BUFFER_KEY);
	printf("\n");
	printf("\nMODO: DIFICULTAD FACIL");  //Se muestra un mensaje con el nivel de dificultad elegido
	printf("\n");
	fflush(stdout);
	piUnlock (STD_IO_BUFFER_KEY);

	MensajeBienvenida(); //Muestra un mensaje de bienvenida
}

/*
 * Se encarga de establecer la dificultad media al juego
 * Su función principal es establecer un tiempo de refresco medio
 */
void ActualizaDificultadMedia (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_MEDIO);  //Desactiva el flag de dificultad media
	piUnlock (SYSTEM_FLAGS_KEY);

	//Establece el tiempo de refresco del juego igual a TIMEOUT_MEDIO (1000)
	p_snakePi->serpiente.timeout_dificultad = TIMEOUT_MEDIO;

	piLock (STD_IO_BUFFER_KEY);
	printf("\n");
	printf("\nMODO: DIFICULTAD MEDIO"); //Se muestra un mensaje con el nivel de dificultad elegido
	printf("\n");
	fflush(stdout);
	piUnlock (STD_IO_BUFFER_KEY);

	MensajeBienvenida();//Muestra un mensaje de bienvenida
}

/*
 * Se encarga de establecer la dificultad dificil al juego
 * Su función principal es establecer un tiempo de refresco bajo
 */
void ActualizaDificultadDificil (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_DIFICIL); //Se desactiva el flag de dificultad dificil
	piUnlock (SYSTEM_FLAGS_KEY);

	//Establece el tiempo de refresco del juego igual a TIMEOUT_DIFICIL (500)
	p_snakePi->serpiente.timeout_dificultad = TIMEOUT_DIFICIL;

	piLock (STD_IO_BUFFER_KEY);
	printf("\n");
	printf("\nMODO: DIFICULTAD DIFICIL"); //Se muestra un mensaje con el nivel de dificultad elegido
	printf("\n");
	fflush(stdout);
	piUnlock (STD_IO_BUFFER_KEY);

	MensajeBienvenida();//Muestra un mensaje de bienvenida
}

/*
 * Se encarga de establecer la dificultad de velocidad incremental al juego
 * Su función principal es establecer un tiempo de refresco incialmente alto
 * Cada 5 manzanas conseguidas, se reduce el tiempo de refresco a la mitad
 */
void ActualizaDificultadIncremental (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock(SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_INCREMENTAL); //Se desactiva el flag de velocidad incremental
	piUnlock (SYSTEM_FLAGS_KEY);

	//Establece el tiempo de refresco del juego igual a TIMEOUT_INCREMENTAL (1400)
	p_snakePi->serpiente.timeout_dificultad = TIMEOUT_INCREMENTAL;
	//Variable para saber en ActualizaJuego() si el modo elegido es el de dificultad incremental
	p_snakePi->serpiente.velocidad_incremental=1;

	piLock (STD_IO_BUFFER_KEY);
	printf("\n");
	printf("\nMODO: DIFICULTAD INCREMENTAL"); //Se muestra un mensaje con el nivel de dificultad elegido
	printf("\nCADA 5 MANZANAS COMIDAS AUMENTARÁ LA VELOCIDAD DE LA SERPIENTE");
	printf("\n");
	fflush(stdout);
	piUnlock (STD_IO_BUFFER_KEY);

	MensajeBienvenida(); //Muestra un mensaje de bienvenida
}

void MueveSerpienteIzquierda (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock (SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_MOV_IZQUIERDA);
	piUnlock (SYSTEM_FLAGS_KEY);

	CambiarDireccionSerpiente(&(p_snakePi->serpiente), IZQUIERDA);
	ActualizaSnakePi(p_snakePi);

	if(CompruebaColision(&(p_snakePi->serpiente),&(p_snakePi->manzana),0)){
		piLock (SYSTEM_FLAGS_KEY);
		flags |= ( FLAG_FIN_JUEGO);
		piUnlock (SYSTEM_FLAGS_KEY);
	}
	else {

		piLock (MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);
		piUnlock (MATRIX_KEY);

		piLock (STD_IO_BUFFER_KEY);
		PintaPantallaPorTerminal((p_snakePi->p_pantalla));
		piUnlock (STD_IO_BUFFER_KEY);

		tmr_startms(p_snakePi->tmr_refresco_snake, p_snakePi->serpiente.timeout_dificultad);

	}
}

void MueveSerpienteDerecha (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock (SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_MOV_DERECHA);
	piUnlock (SYSTEM_FLAGS_KEY);

	CambiarDireccionSerpiente(&(p_snakePi->serpiente), DERECHA);
	ActualizaSnakePi(p_snakePi);

	if(CompruebaColision(&(p_snakePi->serpiente),&(p_snakePi->manzana),0)){
		piLock (SYSTEM_FLAGS_KEY);
		flags |= ( FLAG_FIN_JUEGO);
		piUnlock (SYSTEM_FLAGS_KEY);
	}
	else{
		piLock (MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);
		piUnlock (MATRIX_KEY);

		piLock (STD_IO_BUFFER_KEY);
		PintaPantallaPorTerminal(p_snakePi->p_pantalla);
		piUnlock (STD_IO_BUFFER_KEY);

		tmr_startms(p_snakePi->tmr_refresco_snake, p_snakePi->serpiente.timeout_dificultad);

	}
}

void MueveSerpienteArriba (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock (SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_MOV_ARRIBA);
	piUnlock (SYSTEM_FLAGS_KEY);

	CambiarDireccionSerpiente(&(p_snakePi->serpiente), ARRIBA);
	ActualizaSnakePi(p_snakePi);

	if(CompruebaColision(&(p_snakePi->serpiente),&(p_snakePi->manzana),0)){
		piLock (SYSTEM_FLAGS_KEY);
		flags |= ( FLAG_FIN_JUEGO);
		piUnlock (SYSTEM_FLAGS_KEY);
	}
	else{
		piLock (MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);
		piUnlock (MATRIX_KEY);

		piLock (STD_IO_BUFFER_KEY);
		PintaPantallaPorTerminal(p_snakePi->p_pantalla);
		piUnlock (STD_IO_BUFFER_KEY);

		tmr_startms(p_snakePi->tmr_refresco_snake, p_snakePi->serpiente.timeout_dificultad);

	}
}

void MueveSerpienteAbajo (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock (SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_MOV_ABAJO);
	piUnlock (SYSTEM_FLAGS_KEY);

	CambiarDireccionSerpiente(&(p_snakePi->serpiente), ABAJO);
	ActualizaSnakePi(p_snakePi);

	if(CompruebaColision(&(p_snakePi->serpiente),&(p_snakePi->manzana),0)){
		piLock (SYSTEM_FLAGS_KEY);
		flags |= ( FLAG_FIN_JUEGO);
		piUnlock (SYSTEM_FLAGS_KEY);
	}
	else{
		piLock (MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);
		piUnlock (MATRIX_KEY);

		piLock (STD_IO_BUFFER_KEY);
		PintaPantallaPorTerminal(p_snakePi->p_pantalla);
		piUnlock (STD_IO_BUFFER_KEY);

		tmr_startms(p_snakePi->tmr_refresco_snake, p_snakePi->serpiente.timeout_dificultad);

	}
}

void ActualizarJuego (fsm_t* this) {
	tipo_snakePi* p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	piLock (SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_TIMER_JUEGO);
	piUnlock (SYSTEM_FLAGS_KEY);

	CambiarDireccionSerpiente(&(p_snakePi->serpiente), p_snakePi->serpiente.direccion);
	ActualizaSnakePi(p_snakePi);

	if(CompruebaColision(&(p_snakePi->serpiente),&(p_snakePi->manzana),0)){
		piLock (SYSTEM_FLAGS_KEY);
		flags |= ( FLAG_FIN_JUEGO);
		piUnlock (SYSTEM_FLAGS_KEY);
	}
	else{
		piLock (MATRIX_KEY);
		ActualizaPantallaSnakePi(p_snakePi);
		piUnlock (MATRIX_KEY);

		piLock (STD_IO_BUFFER_KEY);
		PintaPantallaPorTerminal(p_snakePi->p_pantalla);
		piUnlock (STD_IO_BUFFER_KEY);

		tmr_startms(p_snakePi->tmr_refresco_snake, p_snakePi->serpiente.timeout_dificultad);

	}

}

/*
 * Su función es terminar el juego
 * Desactiva el display, para poner la pantalla final
 * Se activa, para poder mostrarla en el terminal
 * Muestra mensaje de Game Over
 */
void FinalJuego (fsm_t* this) {

	piLock (SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_FIN_JUEGO); //Desactiva el flag de fin de juego
	piUnlock (SYSTEM_FLAGS_KEY);

	pseudoWiringPiEnableDisplay(0); //Deshabilita el display

	CaritaTriste(this); //Muestra la pantalla final (carita triste)

	pseudoWiringPiEnableDisplay(1); //Habilita el display

	MensajeGameOver(this); //Muestra mensaje de Game Over
}

/*Su función es resetear el juego
 * Desactiva el flag
 * Deshabilita el display
 */
void ReseteaJuego (fsm_t* this) {

	piLock (SYSTEM_FLAGS_KEY);
	flags &= (~ FLAG_BOTON); //Desactiva el flag del boton
	piUnlock (SYSTEM_FLAGS_KEY);

	pseudoWiringPiEnableDisplay(0); //Deshabilita el display

	MensajeReinicio(); //Muestra mensaje de Game Over

}

void timer_isr (union sigval value) {
	piLock (SYSTEM_FLAGS_KEY);
	flags |= FLAG_TIMER_JUEGO;
	piUnlock (SYSTEM_FLAGS_KEY);

}

//------------------------------------------------------
// MEJORA CARITA TRISTE Y FELIZ
//------------------------------------------------------

/*
 * Pantalla inicial
 * Se muestra al comenzar el juego
 * Contiene una cara sonriente
 */
void CaritaContenta (fsm_t* this) {
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	int i;
	int j;

	piLock (MATRIX_KEY); //Se bloquea el MUTEX

	for(i=0; i<NUM_FILAS_DISPLAY; i++){ //Se recorren las filas
		for(j=0; j<NUM_COLUMNAS_DISPLAY; j++){ //Se recorren todas las columnas
			//Se recorre la pantalla inicial copiandola a la pantalla del display
			p_snakePi->p_pantalla->matriz[j][i]=pantalla_inicial.matriz[j][i];
		}
	}

	piUnlock (MATRIX_KEY); //Se desbloquea el MUTEX

}

/*
 * Pantalla final
 * Se muestra al finalizar el juego
 * Contiene una cara triste
 */
void CaritaTriste (fsm_t* this) {
	tipo_snakePi *p_snakePi;
	p_snakePi = (tipo_snakePi*)(this->user_data);

	int i;
	int j;

	piLock (MATRIX_KEY);//Se bloquea el MUTEX

	for(i=0; i<NUM_FILAS_DISPLAY; i++){//Se recorren las filas
		for(j=0; j<NUM_COLUMNAS_DISPLAY; j++){ //Se recorren todas las columnas
			//Se recorre la pantalla final copiandola a la pantalla del display
			p_snakePi->p_pantalla->matriz[j][i]=pantalla_final.matriz[j][i];
		}
	}

	piUnlock (MATRIX_KEY);//Se desbloquea el MUTEX

}

#include "teclado_TL04.h"

char tecladoTL04[4][4] = {
	{'1', '2', '3', 'C'},
	{'4', '5', '6', 'D'},
	{'7', '8', '9', 'E'},
	{'A', '0', 'B', 'F'}
};

// Maquina de estados: lista de transiciones
// {EstadoOrigen, CondicionDeDisparo, EstadoFinal, AccionesSiTransicion }
fsm_trans_t fsm_trans_excitacion_columnas[] = {
	{ TECLADO_ESPERA_COLUMNA, CompruebaTimeoutColumna, TECLADO_ESPERA_COLUMNA, TecladoExcitaColumna },
	{-1, NULL, -1, NULL },
};

fsm_trans_t fsm_trans_deteccion_pulsaciones[] = {
	{ TECLADO_ESPERA_TECLA, CompruebaTeclaPulsada, TECLADO_ESPERA_TECLA, ProcesaTeclaPulsada},
	{-1, NULL, -1, NULL },
};



//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------

void InicializaTeclado(TipoTeclado *p_teclado) {

	p_teclado->columna_actual = COLUMNA_1;

	//INICIALIZACION  DE LA EXCITACION DE COLUMNAS
	pinMode (teclado.columnas[0], OUTPUT);
	digitalWrite (teclado.columnas[0], HIGH);

	for(int i=1;i<NUM_COLUMNAS_TECLADO;i++) {
		pinMode (teclado.columnas[i], OUTPUT);
		digitalWrite (teclado.columnas[i], LOW);
	}

	//INICIALIZACION  DE LA EXCITACION DE FILAS
	for(int j=0;j<NUM_FILAS_TECLADO;j++) {
		pinMode (teclado.filas[j], INPUT);
		pullUpDnControl (teclado.filas[j], PUD_DOWN);
		wiringPiISR(teclado.filas[j], INT_EDGE_RISING, teclado.rutinas_ISR[j]);
	}

	teclado.tmr_duracion_columna = tmr_new(timer_duracion_columna_isr);
	tmr_startms(teclado.tmr_duracion_columna, TIMEOUT_COLUMNA_TECLADO);

}

//------------------------------------------------------
// OTROS PROCEDIMIENTOS PROPIOS DE LA LIBRERIA
//------------------------------------------------------

void ActualizaExcitacionTecladoGPIO (int columna) {

	switch(columna){
		case COLUMNA_1:
				digitalWrite (GPIO_KEYBOARD_COL_1, HIGH);
				digitalWrite (GPIO_KEYBOARD_COL_2, LOW);
				digitalWrite (GPIO_KEYBOARD_COL_3, LOW);
				digitalWrite (GPIO_KEYBOARD_COL_4, LOW);
				break;

		case COLUMNA_2:
				digitalWrite (GPIO_KEYBOARD_COL_1, LOW);
				digitalWrite (GPIO_KEYBOARD_COL_2, HIGH);
				digitalWrite (GPIO_KEYBOARD_COL_3, LOW);
				digitalWrite (GPIO_KEYBOARD_COL_4, LOW);
				break;

		case COLUMNA_3:
				digitalWrite (GPIO_KEYBOARD_COL_1, LOW);
				digitalWrite (GPIO_KEYBOARD_COL_2, LOW);
				digitalWrite (GPIO_KEYBOARD_COL_3, HIGH);
				digitalWrite (GPIO_KEYBOARD_COL_4, LOW);
				break;

		case COLUMNA_4:
				digitalWrite (GPIO_KEYBOARD_COL_1, LOW);
				digitalWrite (GPIO_KEYBOARD_COL_2, LOW);
				digitalWrite (GPIO_KEYBOARD_COL_3, LOW);
				digitalWrite (GPIO_KEYBOARD_COL_4, HIGH);
				break;
		default:
			break;
	}
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaTimeoutColumna (fsm_t* this) {
	int result = 0;
	TipoTeclado *p_teclado;
	p_teclado = (TipoTeclado*)(this->user_data);

	piLock (KEYBOARD_KEY);
	result = (p_teclado->flags & FLAG_TIMEOUT_COLUMNA_TECLADO);
	piUnlock (KEYBOARD_KEY);

	return result;
}

int CompruebaTeclaPulsada (fsm_t* this) {
	int result = 0;
	TipoTeclado *p_teclado;
	p_teclado = (TipoTeclado*)(this->user_data);

	piLock (KEYBOARD_KEY);
	result = (p_teclado->flags & FLAG_TECLA_PULSADA);
	piUnlock (KEYBOARD_KEY);

	return result;
}

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LAS MAQUINAS DE ESTADOS
//------------------------------------------------------

void TecladoExcitaColumna (fsm_t* this) {
	TipoTeclado *p_teclado;
	p_teclado = (TipoTeclado*)(this->user_data);

	p_teclado->columna_actual++;
	if(p_teclado->columna_actual >= NUM_COLUMNAS_TECLADO){
		p_teclado->columna_actual = 0;
	}

	ActualizaExcitacionTecladoGPIO(p_teclado->columna_actual);

	teclado.flags &= (~FLAG_TIMEOUT_COLUMNA_TECLADO);
	tmr_startms(teclado.tmr_duracion_columna, TIMEOUT_COLUMNA_TECLADO);

}

void ProcesaTeclaPulsada (fsm_t* this) {
	TipoTeclado *p_teclado;
	p_teclado = (TipoTeclado*)(this->user_data);

	piLock (STD_IO_BUFFER_KEY);
	teclado.flags &= (~FLAG_TECLA_PULSADA);

	switch(p_teclado->teclaPulsada.col){
		case COLUMNA_1:
			if (p_teclado->teclaPulsada.row == FILA_1){
				//caso tecla "1"
				//printf("\nKeypress 1!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_FACIL;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_2){
				//caso tecla "4"
				//printf("\nKeypress 4!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_INCREMENTAL;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_3){
				//caso tecla "7"
				//printf("\nKeypress 7!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_REANUDAR;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_4){
				//caso tecla "A"
				//printf("\nKeypress A!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_MOV_IZQUIERDA;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			break;

		case COLUMNA_2:
			if (p_teclado->teclaPulsada.row == FILA_1){
				//caso tecla "2"
				//printf("\nKeypress 2!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_MEDIO;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_2){
				//caso tecla "5"
				//printf("\nKeypress 5!!!\n");
				//fflush(stdout);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_3){
				//caso tecla "8"
				//printf("\nKeypress 8!!!\n");
				//fflush(stdout);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_4){
				//caso tecla "0"
				//printf("\nKeypress 0!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_FIN_JUEGO;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			break;

		case COLUMNA_3:
			if (p_teclado->teclaPulsada.row == FILA_1){
				//caso tecla "3"
				//printf("\nKeypress 3!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_DIFICIL;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_2){
				//caso tecla "6"
				//printf("\nKeypress 6!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_PAUSA;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_3){
				//caso tecla "9"
				//printf("\nKeypress 9!!!\n");
				//fflush(stdout);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_4){
				//caso tecla "B"
				//printf("\nKeypress B!!!\n");
				//fflush(stdout);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			break;

		case COLUMNA_4:
			if (p_teclado->teclaPulsada.row == FILA_1){
				//caso tecla "C"
				//printf("\nKeypress C!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_MOV_ABAJO;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_2){
				//caso tecla "D"
				//printf("\nKeypress D!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_MOV_DERECHA;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_3){
				//caso tecla "E"
				//printf("\nKeypress E!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_MOV_ARRIBA;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			else if(p_teclado->teclaPulsada.row == FILA_4){
				//caso tecla "F"
				//printf("\nKeypress F!!!\n");
				//fflush(stdout);
				piLock(SYSTEM_FLAGS_KEY);
				flags |= FLAG_BOTON;
				piUnlock(SYSTEM_FLAGS_KEY);
				p_teclado->teclaPulsada.row = -1;
				p_teclado->teclaPulsada.col = -1;
			}
			break;

		default:
			break;
	}
	piUnlock(STD_IO_BUFFER_KEY);
}


//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------

void teclado_fila_1_isr (void) {

	/*
 	if (millis () < teclado.debounceTime[FILA_1]){
		teclado.debounceTime[FILA_1] = millis () + DEBOUNCE_TIME ;
		return;
	}
	*/

	piLock (KEYBOARD_KEY);

	teclado.teclaPulsada.row = FILA_1;
	teclado.teclaPulsada.col = teclado.columna_actual;

	teclado.flags |= FLAG_TECLA_PULSADA;

	piUnlock (KEYBOARD_KEY);

//	teclado.debounceTime[FILA_1] = millis () + DEBOUNCE_TIME;

}

void teclado_fila_2_isr (void) {

	/*
    if (millis () < teclado.debounceTime[FILA_2]){
		teclado.debounceTime[FILA_2] = millis () + DEBOUNCE_TIME ;
		return;
	}
	*/

	piLock (KEYBOARD_KEY);

	teclado.teclaPulsada.row = FILA_2;
	teclado.teclaPulsada.col = teclado.columna_actual;

	teclado.flags |= FLAG_TECLA_PULSADA;
	piUnlock (KEYBOARD_KEY);

//	teclado.debounceTime[FILA_2] = millis () + DEBOUNCE_TIME;

}

void teclado_fila_3_isr (void) {

    /*
    if (millis () < teclado.debounceTime[FILA_3]){
		teclado.debounceTime[FILA_3] = millis () + DEBOUNCE_TIME ;
		return;
	}
	*/
	piLock (KEYBOARD_KEY);

	teclado.teclaPulsada.row = FILA_3;
	teclado.teclaPulsada.col = teclado.columna_actual;

	teclado.flags |= FLAG_TECLA_PULSADA;
	piUnlock (KEYBOARD_KEY);

//	teclado.debounceTime[FILA_3] = millis () + DEBOUNCE_TIME;

}

void teclado_fila_4_isr (void) {

	/*
	if (millis () < teclado.debounceTime[FILA_4]){
		teclado.debounceTime[FILA_4] = millis () + DEBOUNCE_TIME ;
		return;
	}
	*/
	piLock (KEYBOARD_KEY);

	teclado.teclaPulsada.row = FILA_4;
	teclado.teclaPulsada.col = teclado.columna_actual;

	teclado.flags |= FLAG_TECLA_PULSADA;

	piUnlock (KEYBOARD_KEY);

//	teclado.debounceTime[FILA_4] = millis () + DEBOUNCE_TIME;
}

void timer_duracion_columna_isr (union sigval value) {
	piLock (KEYBOARD_KEY);
	teclado.flags |= FLAG_TIMEOUT_COLUMNA_TECLADO;
	piUnlock (KEYBOARD_KEY);
}

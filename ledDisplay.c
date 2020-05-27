
#include "ledDisplay.h"

	/*
	 * Cara sonriente
	 * Se muestra al inicio del juego
	 */
	tipo_pantalla pantalla_inicial = {
		.matriz = {
		{0,0,0,0,0,0,0},
		{0,1,1,0,1,0,0},
		{0,1,1,0,0,1,0},
		{0,0,0,0,0,1,0},
		{0,0,0,0,0,1,0},
		{0,1,1,0,0,1,0},
		{0,1,1,0,1,0,0},
		{0,0,0,0,0,0,0},
		}
	};

	/*
	 * Cara triste
	 * Se muestra al finalizar el juego
	 */
	tipo_pantalla pantalla_final = {
		.matriz = {
		{0,0,0,0,0,0,0},
		{0,0,1,0,0,1,0},
		{0,1,1,0,1,0,0},
		{0,0,0,0,1,0,0},
		{0,0,0,0,1,0,0},
		{0,1,1,0,1,0,0},
		{0,0,1,0,0,1,0},
		{0,0,0,0,0,0,0},
		}
	};

// Maquina de estados: lista de transiciones
// {EstadoOrigen, CondicionDeDisparo, EstadoFinal, AccionesSiTransicion }
fsm_trans_t fsm_trans_excitacion_display[] = {
	{ DISPLAY_ESPERA_COLUMNA, CompruebaTimeoutColumnaDisplay, DISPLAY_ESPERA_COLUMNA, ActualizaExcitacionDisplay },
	{-1, NULL, -1, NULL },
};

//------------------------------------------------------
// PROCEDIMIENTOS DE INICIALIZACION DE LOS OBJETOS ESPECIFICOS
//------------------------------------------------------

void InicializaLedDisplay (TipoLedDisplay *led_display) {

	led_display->columna_actual = 0;

	//INICIALIZACION  DE LA EXCITACION DE COLUMNAS
	piLock (MATRIX_KEY);

	for(int i=0;i<NUM_COLUMNAS_DISPLAY;i++) {
		pinMode (led_display->columnas[i], OUTPUT);
		digitalWrite (led_display->columnas[i], LOW);
	}

	//INICIALIZACION  DE LA EXCITACION DE FILAS
	for(int j=0;j<NUM_FILAS_DISPLAY;j++) {
		pinMode (led_display->filas[j], OUTPUT);
		digitalWrite (led_display->filas[j], HIGH);
	}

	piUnlock (MATRIX_KEY);

	led_display->tmr_refresco_display = tmr_new(timer_refresco_display_isr);

	tmr_startms((tmr_t*)(led_display->tmr_refresco_display), TIMEOUT_COLUMNA_DISPLAY);

}

//------------------------------------------------------
// OTROS PROCEDIMIENTOS PROPIOS DE LA LIBRERIA
//------------------------------------------------------

void ApagaFilas (TipoLedDisplay *led_display){

	for(int i=0; i<NUM_FILAS_DISPLAY; i++){
		digitalWrite(led_display->filas[i], HIGH);
	}
}

void ExcitaColumnas(int columna) {

	switch(columna) {
		case 0:
				digitalWrite (GPIO_LED_DISPLAY_COL_1, LOW);
				digitalWrite (GPIO_LED_DISPLAY_COL_2, LOW);
				digitalWrite (GPIO_LED_DISPLAY_COL_3, LOW);
				break;

		case 1:
				digitalWrite (GPIO_LED_DISPLAY_COL_1, HIGH);
				digitalWrite (GPIO_LED_DISPLAY_COL_2, LOW);
				digitalWrite (GPIO_LED_DISPLAY_COL_3, LOW);
				break;

		case 2:
				digitalWrite (GPIO_LED_DISPLAY_COL_1, LOW);
				digitalWrite (GPIO_LED_DISPLAY_COL_2, HIGH);
				digitalWrite (GPIO_LED_DISPLAY_COL_3, LOW);
				break;
		case 3:
				digitalWrite (GPIO_LED_DISPLAY_COL_1, HIGH);
				digitalWrite (GPIO_LED_DISPLAY_COL_2, HIGH);
				digitalWrite (GPIO_LED_DISPLAY_COL_3, LOW);
				break;
		case 4:
				digitalWrite (GPIO_LED_DISPLAY_COL_1, LOW);
				digitalWrite (GPIO_LED_DISPLAY_COL_2, LOW);
				digitalWrite (GPIO_LED_DISPLAY_COL_3, HIGH);
				break;
		case 5:
				digitalWrite (GPIO_LED_DISPLAY_COL_1, HIGH);
				digitalWrite (GPIO_LED_DISPLAY_COL_2, LOW);
				digitalWrite (GPIO_LED_DISPLAY_COL_3, HIGH);
				break;
		case 6:
				digitalWrite (GPIO_LED_DISPLAY_COL_1, LOW);
				digitalWrite (GPIO_LED_DISPLAY_COL_2, HIGH);
				digitalWrite (GPIO_LED_DISPLAY_COL_3, HIGH);
				break;
		case 7:
				digitalWrite (GPIO_LED_DISPLAY_COL_1, HIGH);
				digitalWrite (GPIO_LED_DISPLAY_COL_2, HIGH);
				digitalWrite (GPIO_LED_DISPLAY_COL_3, HIGH);
				break;
		default:
		break;
	}
}

void ActualizaLedDisplay (TipoLedDisplay *led_display) {

	ApagaFilas(led_display);

	led_display->columna_actual = (led_display->columna_actual + 1) % NUM_COLUMNAS_DISPLAY ;

	ExcitaColumnas(led_display->columna_actual);

	for(int i=0; i<NUM_FILAS_DISPLAY; i++){
		if(led_display->pantalla.matriz[led_display->columna_actual][i]==1){
			digitalWrite(led_display->filas[i],LOW);
		}
	}
}

void PintaPantallaPorTerminal (tipo_pantalla *p_pantalla) {

	#ifdef __SIN_PSEUDOWIRINGPI__
		printf("\n[PANTALLA]\n");
		fflush(stdout);
		int i=0;
		int j=0;
		for(j=0;j<NUM_FILAS_DISPLAY;j++) {
			for(i=0;i<NUM_COLUMNAS_DISPLAY;i++) {
				printf("%d", p_pantalla->matriz[i][j]);
				fflush(stdout);
			}
			printf("\n");
			fflush(stdout);
		}
		fflush(stdout);
	#endif
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

int CompruebaTimeoutColumnaDisplay (fsm_t* this) {
	int result = 0;

	TipoLedDisplay *p_ledDisplay;
	p_ledDisplay = (TipoLedDisplay*)(this->user_data);

	piLock (MATRIX_KEY);
	result = (p_ledDisplay->flags & FLAG_TIMEOUT_COLUMNA_DISPLAY);
	piUnlock (MATRIX_KEY);

	return result;
}

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

void ActualizaExcitacionDisplay (fsm_t* this) {

	TipoLedDisplay *p_ledDisplay;
	p_ledDisplay = (TipoLedDisplay*)(this->user_data);

	piLock (MATRIX_KEY);
	p_ledDisplay->flags &= (~FLAG_TIMEOUT_COLUMNA_DISPLAY);
	ActualizaLedDisplay(p_ledDisplay);
	piUnlock (MATRIX_KEY);

	tmr_startms(led_display.tmr_refresco_display, TIMEOUT_COLUMNA_DISPLAY);
}

//------------------------------------------------------
// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES
//------------------------------------------------------

void timer_refresco_display_isr (union sigval value) {

	piLock (MATRIX_KEY);
	led_display.flags |= FLAG_TIMEOUT_COLUMNA_DISPLAY;
	piUnlock (MATRIX_KEY);

}


#include "snakePi.h"

int flags = 0;

// Declaracion del objeto teclado
TipoTeclado teclado = {
	.columnas = {GPIO_KEYBOARD_COL_1, GPIO_KEYBOARD_COL_2, GPIO_KEYBOARD_COL_3, GPIO_KEYBOARD_COL_4},
	.filas = {GPIO_KEYBOARD_ROW_1, GPIO_KEYBOARD_ROW_2, GPIO_KEYBOARD_ROW_3, GPIO_KEYBOARD_ROW_4},
	.debounceTime = {DEBOUNCE_TIME, DEBOUNCE_TIME, DEBOUNCE_TIME, DEBOUNCE_TIME},
	.rutinas_ISR = {teclado_fila_1_isr, teclado_fila_2_isr, teclado_fila_3_isr, teclado_fila_4_isr},
	.columna_actual = 0,
	.teclaPulsada.col = -1,
	.teclaPulsada.row = -1,
	.flags = 0
};

// Declaracion del objeto display
TipoLedDisplay led_display = {
	.columnas = {GPIO_LED_DISPLAY_COL_1, GPIO_LED_DISPLAY_COL_2, GPIO_LED_DISPLAY_COL_3},
	.filas = {GPIO_LED_DISPLAY_ROW_1, GPIO_LED_DISPLAY_ROW_2, GPIO_LED_DISPLAY_ROW_3, GPIO_LED_DISPLAY_ROW_4, GPIO_LED_DISPLAY_ROW_5, GPIO_LED_DISPLAY_ROW_6, GPIO_LED_DISPLAY_ROW_7},
	.columna_actual = 0,
	.flags = 0
};

//------------------------------------------------------
// FUNCIONES DE CONFIGURACION/INICIALIZACION
//------------------------------------------------------

// int ConfiguracionSistema (TipoSistema *p_sistema): procedimiento de configuracion
// e inicializacion del sistema.
// Realizará, entra otras, todas las operaciones necesarias para:
// configurar el uso de posibles librerías (e.g. Wiring Pi),
// configurar las interrupciones externas asociadas a los pines GPIO,
// configurar las interrupciones periódicas y sus correspondientes temporizadores,
// la inicializacion de los diferentes elementos de los que consta nuestro sistema,
// crear, si fuese necesario, los threads adicionales que pueda requerir el sistema
// como el thread de exploración del teclado del PC
int ConfiguraInicializaSistema (TipoSistema *p_sistema) {
	int result = 0;
	if(wiringPiSetupGpio()<0){
		printf("Error");
		return -1;
	}

	p_sistema->snakePi.tmr_refresco_snake = tmr_new(timer_isr);
	p_sistema->snakePi.p_pantalla = &(led_display.pantalla);

	InicializaTeclado(&teclado);
	InicializaLedDisplay(&led_display);
	//led_display.tmr_refresco_display = tmr_new(timer_refresco_display_isr);



// Lanzamos thread para exploracion del teclado convencional del PC
//	result = piThreadCreate (thread_explora_teclado_PC);
//
//	if (result != 0) {
//		printf ("Thread didn't start!!!\n");
//		return -1;
//	}

	return result;
}

//------------------------------------------------------
// FUNCIONES LIGADAS A THREADS ADICIONALES
//------------------------------------------------------

/*// PI_THREAD (thread_explora_teclado_PC) {
	int teclaPulsada;

	while(1) {
		delay(10); // Wiring Pi function: pauses program execution for at least 10 ms

		piLock (STD_IO_BUFFER_KEY);

		if(kbhit()) {
			teclaPulsada = kbread();

			switch(teclaPulsada) {

				case 's':
					printf("Tecla S pulsada!\n");
					fflush(stdout);
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_TIMER_JUEGO;
					piUnlock (SYSTEM_FLAGS_KEY);
					break;

				case 'a':
					printf("Tecla A pulsada!\n");
					fflush(stdout);
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_IZQUIERDA;
					piUnlock (SYSTEM_FLAGS_KEY);
					break;

				case 'd':
					printf("Tecla D pulsada!\n");
					fflush(stdout);
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_DERECHA;
					piUnlock (SYSTEM_FLAGS_KEY);
					break;

				case 'x':
					printf("Tecla X pulsada!\n");
					fflush(stdout);
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_ABAJO;
					piUnlock (SYSTEM_FLAGS_KEY);
					break;

				case 'w':
					printf("Tecla W pulsada!\n");
					fflush(stdout);
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_MOV_ARRIBA;
					piUnlock (SYSTEM_FLAGS_KEY);
					break;

				case 'q':
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_FIN_JUEGO;
					piUnlock (SYSTEM_FLAGS_KEY);
					//exit(0);
					break;

				case 'b':
					printf("Boton pulsado!\n");
					fflush(stdout);
					piLock (SYSTEM_FLAGS_KEY);
					flags |= FLAG_BOTON;
					piUnlock (SYSTEM_FLAGS_KEY);
					break;

				default:
					printf("INVALID KEY!!!\n");
					break;
			}
		}

		piUnlock (STD_IO_BUFFER_KEY);
	}
}*/

// wait until next_activation (absolute time)
void delay_until (unsigned int next) {
	unsigned int now = millis();
	if (next > now) {
		delay (next - now);
	}
}


int main () {
	TipoSistema sistema;
	unsigned int next;

	/*Declaración de los estados y transiciones de snakePi*/
	fsm_trans_t snakePi[] = {
		{ WAIT_DIFFICULTY, CompruebaDificultadFacil, WAIT_START, ActualizaDificultadFacil },
		{ WAIT_DIFFICULTY, CompruebaDificultadMedia, WAIT_START, ActualizaDificultadMedia },
		{ WAIT_DIFFICULTY, CompruebaDificultadDificil, WAIT_START, ActualizaDificultadDificil },
		{ WAIT_DIFFICULTY, CompruebaDificultadIncremental, WAIT_START, ActualizaDificultadIncremental },
		{ WAIT_START, CompruebaBotonPulsado, WAIT_PUSH, InicializaJuego },
		{ WAIT_PUSH, CompruebaTimeoutActualizacionJuego, WAIT_PUSH, ActualizarJuego },
		{ WAIT_PUSH, CompruebaPausa, WAIT_PAUSE, PausaJuego },
		{ WAIT_PAUSE, CompruebaReanudar, WAIT_PUSH, ReanudarJuego },
		{ WAIT_PUSH, CompruebaMovimientoArriba, WAIT_PUSH, MueveSerpienteArriba },
		{ WAIT_PUSH, CompruebaMovimientoAbajo, WAIT_PUSH, MueveSerpienteAbajo },
		{ WAIT_PUSH, CompruebaMovimientoIzquierda, WAIT_PUSH, MueveSerpienteIzquierda },
		{ WAIT_PUSH, CompruebaMovimientoDerecha, WAIT_PUSH, MueveSerpienteDerecha },
		{ WAIT_PUSH, CompruebaFinalJuego, WAIT_END, FinalJuego },
		{ WAIT_END, CompruebaBotonPulsado, WAIT_DIFFICULTY, ReseteaJuego },
		{-1, NULL, -1, NULL },
	};


	// Configuracion e inicializacion del sistema
	ConfiguraInicializaSistema (&sistema);

	//Declaración de las maquinas de estados
	fsm_t* teclado_fsm = fsm_new(TECLADO_ESPERA_COLUMNA, fsm_trans_excitacion_columnas, &(teclado));
	fsm_t* tecla_fsm = fsm_new(TECLADO_ESPERA_TECLA, fsm_trans_deteccion_pulsaciones, &(teclado));
	fsm_t* snakePi_fsm = fsm_new (WAIT_DIFFICULTY, snakePi, &(sistema.snakePi));
	fsm_t* display_fsm = fsm_new(DISPLAY_ESPERA_COLUMNA, fsm_trans_excitacion_display, &(led_display));

	next = millis();

	printf("\n");
	printf("\n");
	printf("             MT02\n");
	printf("    ANDRES CASTILLEJO AMESCUA\n");
	printf("JAVIER LOPEZ INIESTA DIAZ DEL CAMPO \n");
	printf("\n");
	printf("ELIGA EL NIVEL DE DIFICULTAD\n");
	printf("\n");
	printf("1 - DIFICULTAD FACIL\n");
	printf("2 - DIFICULTAD MEDIA\n");
	printf("3 - DIFICULTAD DIFICIL\n");
	printf("4 - DIFICULTAD INCREMENTAL\n");
	printf("\n");

	while (1) {
		//Definicion de la función que permite liberar la memoria utilizada por la maquina de estado
		fsm_fire (snakePi_fsm);
		fsm_fire (teclado_fsm);
		fsm_fire (tecla_fsm);
		fsm_fire (display_fsm);

		/*CLK_MS permite una frecuencia de actualizacion del estado del sistema altaç
		para que los tiempos de respuesta sean bajos y no haya conflictos*/

		next += CLK_MS;
		delay_until (next); //Espero
	}

	//Se destruye el timer y las maquinas de estados
	tmr_destroy(sistema.snakePi.tmr_refresco_snake);
	fsm_destroy (snakePi_fsm);

	return 0;
}

#include <REG51F.H>

#define FreqClock 12000000   // Frequ�ncia do clock do sistema: 12 MHz
#define FreqTimer0_emHz 100  // Frequ�ncia desejada para o Timer 0 (100 Hz)
#define TH1_Inicial 204      // Valor inicial do Timer 1 para configuracao do baud rate

/*
 * C�lculo do valor de TH1 para o Timer 1 em Modo 2:
 * Formula:  BaudRate = 2^(SMOD) * FreqClock / (12 * (256 - TH1))
 * Por exemplo, para um baud rate de 1200 bps e SMOD = 1:
 *     1200 = 2 / 32 * 12000000 / (12 * (256 - TH1))
 */

unsigned char caracter;  // Vari�vel global para armazenar o caractere recebido

// ---------- Configura��o do Temporizador 1 ----------
/*
 * Inicializa o Timer 1 no Modo 2 (8 bits com recarga autom�tica)
 * Utilizado para gerar o baud rate da comunica��o serial.
 */
 
void timer1_inicializa () {
	TR1 = 0;                      // Desliga o Timer 1 temporariamente
	TMOD = (TMOD & 0x0F) | 0x20;  // Configura Timer 1 no Modo 2 (8 bits, recarga autom�tica)
	PCON |= 0x80;                 // Define SMOD = 1 para dobrar a taxa de baud rate
	TH1 = TH1_Inicial;            // Carrega o valor inicial do Timer 1
	TR1 = 1;                      // Habilita o Timer 1
}

// ---------- Fun��o: Configura��o da Interface Serial ----------
/*
 * Configura a interface serial no Modo 1 (8 bits, start e stop bit).
 * Habilita a recep��o e as interrup��es relacionadas � interface serial.
 */

void serial1_inicializa () {
	SM0 = 0;  // Configura Modo 1 (8 bits, start e stop bit)
	SM1 = 1;  // Configura Modo 1  
	SM2 = 0;  // Desativa o modo de multiprocessamento
	REN = 1;  // Habilita a recep��o serial
	ES = 1;   // Habilita interrup��o serial
}

// ---------- Tratamento da Interrup��o Serial ----------
/*
 * Rotina de interrup��o da interface serial.
 * - Se um byte foi transmitido (TI = 1): zera o flag TI.
 * - Se um byte foi recebido (RI = 1): armazena o byte e reenvia incrementado.
 */

void serial_int () interrupt 4 using 2 {
	if(TI) {  // Se um byte foi transmitido
		TI = 0;  // Zera o flag de transmiss�o
	}
	if(RI) {  // Se um byte foi recebido
		RI = 0;  // Zera o flag de recep��o
		caracter = SBUF;  // L� o byte recebido
		SBUF = caracter + 1;  // Envia o caractere incrementado de volta
	}
}

// ---------- Fun��o Principal ----------
void main()
{
	timer1_inicializa();  // Configura o Timer 1
	serial1_inicializa(); // Configura a interface serial

	EA = 1;  // Habilita todas as interrup��es globais (Enable All)

	while (1) {
		// Loop infinito
		// A l�gica de recep��o e envio � tratada pela interrup��o serial
	}
}
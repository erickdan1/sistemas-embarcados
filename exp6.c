#include <REG517A.H>
#include <string.h>

#define Freqtimer0_Hz 100
#define FREQ_CLOCK 12000000 // Frequ�ncia do clock do sistema: 12 MHz
#define BUFFER_SIZE 16      // Tamanho dos buffers circulares

// Buffers circulares para transmiss�o e recep��o de dados
unsigned char TxBuffer[BUFFER_SIZE]; // Buffer de transmiss�o
unsigned char RxBuffer[BUFFER_SIZE]; // Buffer de recep��o

// Ponteiros para manipular os buffers circulares
unsigned char TxIn = 0, TxOut = 0;   
unsigned char RxIn = 0, RxOut = 0;   

// Flags e vari�veis auxiliares
unsigned char txOcupado = 0;         // Indica se a transmiss�o est� em progresso
unsigned char c;                     // Vari�vel auxiliar para armazenamento de caracteres
unsigned char nextRxIn;              // Pr�xima posi��o de entrada no buffer Rx

unsigned char buffer[BUFFER_SIZE];

// ---------- Buffer Circular Recep��o ----------

// Verifica se o buffer de recep��o est� vazio
char RxBufferVazio() {
	return (RxIn == RxOut); 
}

// Retira um caractere do buffer de recep��o
char ReceiveChar() {
	char c = RxBuffer[RxOut]; // L� o caractere da posi��o atual do buffer
	RxOut++;
	RxOut %= BUFFER_SIZE; // Atualiza o ponteiro de sa�da de forma circular
	return c;
}

// L� uma string do buffer de recep��o at� encontrar o delimitador '$'
void ReceiveString(char *s) {
	char c;
	unsigned char it = 0;

	RxIn = RxOut = 0;

	while(1){
		if(RxIn > RxOut){
			c = ReceiveChar();
			if (c == '$') { // Se encontrar o caractere delimitador
				break;
			}
			*s++ = c; // Armazena o caractere na string de destino
		}
	}
}


// ---------- Buffer Circular Transmiss�o ----------

// Insere um caractere no buffer de transmiss�o e inicia a transmiss�o se necess�rio
void SendChar(char c) {
	TxBuffer[TxIn] = c; // Armazena o caractere no buffer de transmiss�o
	TxIn++;
	TxIn %= BUFFER_SIZE; // Atualiza o ponteiro de entrada de forma circular
	if (!txOcupado) { // Se a transmiss�o n�o estiver em andamento
		txOcupado = 1; // Marca que a transmiss�o come�ou
		TI0 = 1; // Dispara a transmiss�o manualmente
	}
}

// Envia uma string pelo buffer de transmiss�o at� encontrar '$'
void SendString(char *s) {
	unsigned char len = strlen(s), i=0;
	for(i=0; i<len;i++){
			SendChar(s[i]);
	}
}

// ---------- Configura��o da Interface Serial ----------

// Configura a interface UART para comunica��o serial
void serial1_inicializa() {
	PCON = PCON | 0x80;
	ADCON0 = ADCON0 | 0X80;
	S0CON = 0x50;
	// Configura��o do gerador de baudrate
	TH1 = 0xFD;  // Valor para 9600 bps com clock de 12 MHz (ajustar conforme necess�rio)
	TMOD |= 0x20;  // Timer 1 no Modo 2 (Auto-reload)
	TR1 = 1;       // Habilita o Timer 1
	REN0 = 1;       // Habilita a recep��o serial
	ES0 = 1;       // Habilita a interrup��o de UART
}

// ---------- Tratamento da Interrup��o Serial ----------

// Rotina de interrup��o da interface serial
void serial_int() interrupt 4 using 2 {
	if (TI0) { // Se a transmiss�o foi conclu�da
		TI0 = 0; // Limpa o flag de transmiss�o
		if (RxBufferVazio()) { // Se o buffer de transmiss�o estiver vazio
			txOcupado = 0; // Marca que n�o h� transmiss�o pendente
		} else {
			S0BUF = TxBuffer[TxOut]; // Envia o pr�ximo caractere do buffer Tx
			TxOut++; // Atualiza o ponteiro de sa�da do buffer
			TxOut %= BUFFER_SIZE; // Mant�m dentro dos limites do buffer
		}
	}

	if (RI0) { // Se um caractere foi recebido
		RI0 = 0; // Limpa o flag de recep��o
		nextRxIn = (RxIn + 1) % BUFFER_SIZE; // Calcula a pr�xima posi��o do buffer Rx
		if (nextRxIn != RxOut) { // Se houver espa�o no buffer
			RxBuffer[RxIn] = S0BUF; // Armazena o caractere no buffer Rx
			RxIn = nextRxIn; // Atualiza o ponteiro de entrada do buffer
		}
	}
}

// ---------- Fun��o Principal ----------
void main() {
	serial1_inicializa(); // Configura a interface serial
	EAL = 1;              // Habilita todas as interrup��es globais

	while(1) {
		ReceiveString(buffer); // L� uma string recebida no buffer Rx
		SendString(buffer);    // Reenvia a string pelo buffer Tx
	}
}
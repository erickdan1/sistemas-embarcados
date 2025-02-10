#include <REG51F.H>
#include <string.h>

#define FREQ_CLOCK 12000000 // Frequência do clock do sistema: 12 MHz
#define BUFFER_SIZE 16      // Tamanho dos buffers circulares

// Buffers circulares para transmissão e recepção de dados
unsigned char TxBuffer[BUFFER_SIZE]; // Buffer de transmissão
unsigned char RxBuffer[BUFFER_SIZE]; // Buffer de recepção

// Ponteiros para manipular os buffers circulares
unsigned char TxIn = 0, TxOut = 0;   
unsigned char RxIn = 0, RxOut = 0;   

// Flags e variáveis auxiliares
unsigned char txOcupado = 0;         // Indica se a transmissão está em progresso
unsigned char c;                     // Variável auxiliar para armazenamento de caracteres
unsigned char nextRxIn;              // Próxima posição de entrada no buffer Rx

unsigned char buffer[BUFFER_SIZE];

// ---------- Buffer Circular Recepção ----------

// Verifica se o buffer de recepção está vazio
char RxBufferVazio() {
	return (RxIn == RxOut); 
}

// Retira um caractere do buffer de recepção
char ReceiveChar() {
	char c = RxBuffer[RxOut]; // Lê o caractere da posição atual do buffer
	RxOut++;
	RxOut %= BUFFER_SIZE; // Atualiza o ponteiro de saída de forma circular
	return c;
}

// Lê uma string do buffer de recepção até encontrar o delimitador '$'
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


// ---------- Buffer Circular Transmissão ----------

// Insere um caractere no buffer de transmissão e inicia a transmissão se necessário
void SendChar(char c) {
	TxBuffer[TxIn] = c; // Armazena o caractere no buffer de transmissão
	TxIn++;
	TxIn %= BUFFER_SIZE; // Atualiza o ponteiro de entrada de forma circular
	if (!txOcupado) { // Se a transmissão não estiver em andamento
		txOcupado = 1; // Marca que a transmissão começou
		TI = 1; // Dispara a transmissão manualmente
	}
}

// Envia uma string pelo buffer de transmissão até encontrar '$'
void SendString(char *s) {
	unsigned char len = strlen(s), i=0;
	for(i=0; i<len;i++){
			SendChar(s[i]);
	}
}

// ---------- Configuração da Interface Serial ----------

// Configura a interface UART para comunicação serial
void serial1_inicializa() {
	SCON = 0x50;  // Configura UART no Modo 1: 8 bits de dados, start e stop bit
	TMOD |= 0x20; // Configura o Timer 1 no Modo 2 (auto-reload)
	TH1 = 0xFD;   // Configura baud rate para 9600 bps com clock de 12 MHz
	TR1 = 1;      // Inicia o Timer 1
	REN = 1;      // Habilita recepção serial
	ES = 1;       // Habilita interrupção serial
	EA = 1;       // Habilita todas as interrupções globais
}

// ---------- Tratamento da Interrupção Serial ----------

// Rotina de interrupção da interface serial
void serial_int() interrupt 4 using 2 {
	if (TI) { // Se a transmissão foi concluída
		TI = 0; // Limpa o flag de transmissão
		if (RxBufferVazio()) { // Se o buffer de transmissão estiver vazio
			txOcupado = 0; // Marca que não há transmissão pendente
		} else {
			SBUF = TxBuffer[TxOut]; // Envia o próximo caractere do buffer Tx
			TxOut++; // Atualiza o ponteiro de saída do buffer
			TxOut %= BUFFER_SIZE; // Mantém dentro dos limites do buffer
		}
	}

	if (RI) { // Se um caractere foi recebido
		RI = 0; // Limpa o flag de recepção
		nextRxIn = (RxIn + 1) % BUFFER_SIZE; // Calcula a próxima posição do buffer Rx
		if (nextRxIn != RxOut) { // Se houver espaço no buffer
			RxBuffer[RxIn] = SBUF; // Armazena o caractere no buffer Rx
			RxIn = nextRxIn; // Atualiza o ponteiro de entrada do buffer
		}
	}
}

// ---------- Função Principal ----------
void main() {
	serial1_inicializa(); // Configura a interface serial

	while(1) {
		ReceiveString(buffer); // Lê uma string recebida no buffer Rx
		SendString(buffer);    // Reenvia a string pelo buffer Tx
	}
}
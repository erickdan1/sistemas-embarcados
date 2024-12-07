#include <REG51F.H>

#define FreqClock 12000000
#define FreqTimer0_emHz 100
#define CORRECAO 9
#define TH0_Inicial ((65536-(FreqClock/(12*FreqTimer0_emHz))+CORRECAO)>>8)
#define TL0_Inicial ((65536-(FreqClock/(12*FreqTimer0_emHz))+CORRECAO)&0xFF)

unsigned char contador_controle_baixo = 0;
unsigned char contador_controle_alto = 0;

// Calcular o valor de correção..
// Debugar e encontrar valor que o timer deixa de contar - timer0_int()

// ---------- Configurações do Temporizador ----------

void timer0_inicializa () {
	TR0 = 0;                    // Desliga Timer0
	TMOD = (TMOD & 0xF0)|0x01;  // Timer 0 programado no Modo 1 (16 bits)
	TH0 = TH0_Inicial;          // Programa valor de contagem do Timer0
	TL0 = TL0_Inicial;
	ET0 = 1;                    // Habilita interrupcao do timer 0
	TR0 = 1;                    // Habilita contagem do timer 0
}

void timer0_int (void) interrupt 1 using 2 {
	TR0 = 0;                       // Desliga Timer0
	TL0 += TL0_Inicial;            // Faz recarga da contagem do Timer0
	TH0 += TH0_Inicial + (unsigned char) CY;
	TR0 = 1;                       // Habilita contagem do timer 0
	contador_controle_baixo += 1;  // Contagem de interrupções do bit de controle baixo
	contador_controle_alto += 1;   // Contagem de interrupções do bit de controle alto
}

// ---------------------------------------------------

// Palavra reseravada do ambeinte do 8051 - 'bit'
// Define os bits de controle da porta P2
sbit controle_baixo = P2^0;  // Define 'controle_baixo' como o bit 0 da porta P2
sbit controle_alto = P2^1;   // Define 'controle_alto' como o bit 1 da porta P2

// Definição dos estados para a máquina de estados que controla o bit baixo (controle_baixo)
typedef enum {
    ESPERAR_B,  // Estado aguardando 1 segundo após detectar a transição no controle_baixo
    BIT0_1,     // Estado inicial, monitorando a transição de controle_baixo para 0
    BIT0_0      // Estado monitorando a transição de controle_baixo para 1
} EstadoCB;

// Definição dos estados para a máquina de estados que controla o bit alto (controle_alto)
typedef enum {
    ESPERAR_A,  // Estado aguardando 1 segundo após detectar a transição no controle_alto
    BIT1_1,     // Estado inicial, monitorando a transição de controle_alto para 0
    BIT1_0      // Estado monitorando a transição de controle_alto para 1
} EstadoCA;

// Função que controla o comportamento da máquina de estados associada ao controle_baixo
void fun_controle_baixo () {
		// Define o estado inicial da máquina de estados para controle_baixo
		static EstadoCB estado_atual = BIT0_1;
	
		// Variável auxiliar para evitar a leitura direta de P1 (porta de saída)
		unsigned char p1_aux = 0xFF;
	
		// Variável usada para realizar a contagem de 1 segundo via software
		// static unsigned int contador = 0;
	
		// Implementação da lógica da máquina de estados
		switch (estado_atual) {
		case BIT0_1: if (!controle_baixo) {  // Aguarda a transição do controle_baixo de 1 para 0
			estado_atual = BIT0_0;
		}
		break;
		case ESPERAR_B: if (contador_controle_baixo >= 100) {  // Se contador execeder 100 interrupções (10ms * 100 = 1s)
			// Após 1 segundo, zera os 4 bits menos significativos de P1
			p1_aux = p1_aux & 0xF0;
			P1 = p1_aux;
			estado_atual = BIT0_1;
		}
		break;
		case BIT0_0: if (controle_baixo) {  // Aguarda a transição do controle_baixo de 0 para 1
			// Atualiza os 4 bits menos significativos de P1 com os de P0, mantendo os outros bits inalterados
			p1_aux = (P0 & 0x0F) | (p1_aux & 0xF0);
			P1 = p1_aux;
			
			// Reinicia o contador e muda para o estado ESPERAR_B
			contador_controle_baixo = 0;
			estado_atual = ESPERAR_B;
		}
		break;
	}
}

// Função que controla o comportamento da máquina de estados associada ao controle_alto
void fun_controle_alto () {
		// Define o estado inicial da máquina de estados para controle_alto
		static EstadoCA estado_atual = BIT1_1;
	
		// Variável auxiliar para evitar a leitura direta de P1 (porta de saída)
		unsigned char p1_aux = 0xFF;
	
		// Variável usada para realizar a contagem de 1 segundo via software
		// static unsigned int contador = 0;
	
		// Implementação da lógica da máquina de estados
		switch (estado_atual) {
		case BIT1_1: if (!controle_alto) {  // Aguarda a transição do controle_alto de 1 para 0
			estado_atual = BIT1_0;
		}
		break;
		case ESPERAR_A: if (contador_controle_alto >= 100) { // Se contador execeder 100 interrupções (10ms * 100 = 1s)
			// Após 1 segundo, zera os 4 bits menos significativos de P1
			p1_aux = p1_aux & 0x0F;
			P1 = p1_aux;
			estado_atual = BIT1_1;
		}
		break;
		case BIT1_0: if (controle_alto) {  // Aguarda a transição do controle_alto de 0 para 1
			// Atualiza os 4 bits mais significativos de P1 com os de P0, mantendo os outros bits inalterados
			p1_aux = (P0 & 0xF0) | (p1_aux & 0x0F);
			P1 = p1_aux;
			
			// Reinicia o contador e muda para o estado ESPERAR_A
			contador_controle_alto = 0;
			estado_atual = ESPERAR_A;
		}
		break;
	}
}


void main()
{
	timer0_inicializa();
	EA = 1;  // Habilita o tratamento de interrupções (Enable All)
	while(1) {
		// Chama as funções que controlam os bits de controle (baixo e alto)
		fun_controle_baixo();
		fun_controle_alto();
	}
}
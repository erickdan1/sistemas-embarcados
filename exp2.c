#include <REG51F.H>

// palavra reseravada do ambeinte do 8051 - 'bit'
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
		static unsigned int contador = 0;
	
		// Implementação da lógica da máquina de estados
		switch (estado_atual) {
		case BIT0_1: if (!controle_baixo) {  // Aguarda a transição do controle_baixo de 1 para 0
			estado_atual = BIT0_0;
		}
		break;
		case ESPERAR_B: if (contador < 34910) {  // Contador incrementa até atingir aproximadamente 1 segundo (ajustado para ~1.006 segundos)
			contador += 1;
		} else {
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
			contador = 0;
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
		static unsigned int contador = 0;
	
		// Implementação da lógica da máquina de estados
		switch (estado_atual) {
		case BIT1_1: if (!controle_alto) {  // Aguarda a transição do controle_alto de 1 para 0
			estado_atual = BIT1_0;
		}
		break;
		case ESPERAR_A: if (contador < 34910) {  // Contador incrementa até atingir aproximadamente 1 segundo (ajustado para ~1.006 segundos)
			contador += 1;
		} else {
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
			contador = 0;
			estado_atual = ESPERAR_A;
		}
		break;
	}
}


void main()
{
	while(1) {
		// Chama as funções que controlam os bits de controle (baixo e alto)
		fun_controle_baixo();
		fun_controle_alto();
	}
}
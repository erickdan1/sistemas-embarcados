#include <reg51.h>

#define FrClk 12000000
#define FreqTimer0_emHz 100
#define VALOR_TH0 ((65536 - (FrClk / (12 * FreqTimer0_emHz))+2) >>8)
#define VALOR_TL0 ((65536 - (FrClk / (12 * FreqTimer0_emHz))+2) & 0xFF)


// Declarada como volatile para garantir que cada acesso seja feito diretamente à memória, evitando otimizações indesejadas.
volatile unsigned int contador;
// 'temp' é usada para armazenar temporariamente o valor de 'contador' de forma atômica.
unsigned int temp;

void timer0_inicializa() 
  {
  TR0 = 0;              // Desliga Timer0
  TMOD = (TMOD & 0xF0) | 0x01;  // Timer 0 programado como timer de 16 bits
  TH0 = VALOR_TH0;  // Programa contagem do Timer0
  TL0 = VALOR_TL0;
  ET0 = 1;      // Habilita interrupcao do timer 0
  TR0 = 1;      // Habilita contagem do timer 0
  }

void timer0_int (void) interrupt 1 using 2
  {
  TR0 = 0;              // Desliga Timer0
  TL0 += VALOR_TL0;
  TH0 += VALOR_TH0 + (unsigned char) CY;   // Programa contagem do Timer0
  TR0 = 1;      // Habilita contagem do timer 0

  if (contador != 0)
    contador--;
  }

void main()
	{
  timer0_inicializa();
	EA=1; // Habilita o tratamento de interrupções

	while (1)
      {
      contador = 299;
			
			 /*
			 * Problema: O microcontrolador 8051 acessa variáveis de 16 bits em dois passos (byte baixo e byte alto).
			 * Se uma interrupção ocorrer entre a leitura desses dois bytes, a leitura pode ficar "rasgada" (inconsistente).
			 * 
			 * Solução: Desabilitar as interrupções (EA = 0) durante a leitura de 'contador' para garantir que ambos os bytes sejam lidos
			 * sem interferência da ISR. Após a leitura, as interrupções são reabilitadas (EA = 1).
			 *
			 * A variável 'temp' armazena o valor lido de 'contador' de forma atômica, sem ser interrompido.
			 */
				
			// Aguarda o contador chegar a 0, lendo-o de forma atômica:
			do {
				EA = 0;           // Desabilita interrupções
				temp = contador;  // Lê o valor completo (16 bits) de 'contador'
				EA = 1;           // Habilita interrupções
			} while(temp != 0);	// Fica em loop enquanto "contador != 0"

      if (temp == 0) // Testa se contador é igual a zero.
				P0 = 0;
      else
        P0 = 0xFF;	// ESTA INSTRUÇÃO NUNCA DEVERIA SER EXECUTADA!
      }
	}
	


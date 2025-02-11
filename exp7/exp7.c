#include <reg51.h>

#define FrClk 12000000
#define FreqTimer0_emHz 100
#define VALOR_TH0 ((65536 - (FrClk / (12 * FreqTimer0_emHz))+2) >>8)
#define VALOR_TL0 ((65536 - (FrClk / (12 * FreqTimer0_emHz))+2) & 0xFF)


// Declarada como volatile para garantir que cada acesso seja feito diretamente � mem�ria, evitando otimiza��es indesejadas.
volatile unsigned int contador;
// 'temp' � usada para armazenar temporariamente o valor de 'contador' de forma at�mica.
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
	EA=1; // Habilita o tratamento de interrup��es

	while (1)
      {
      contador = 299;
			
			 /*
			 * Problema: O microcontrolador 8051 acessa vari�veis de 16 bits em dois passos (byte baixo e byte alto).
			 * Se uma interrup��o ocorrer entre a leitura desses dois bytes, a leitura pode ficar "rasgada" (inconsistente).
			 * 
			 * Solu��o: Desabilitar as interrup��es (EA = 0) durante a leitura de 'contador' para garantir que ambos os bytes sejam lidos
			 * sem interfer�ncia da ISR. Ap�s a leitura, as interrup��es s�o reabilitadas (EA = 1).
			 *
			 * A vari�vel 'temp' armazena o valor lido de 'contador' de forma at�mica, sem ser interrompido.
			 */
				
			// Aguarda o contador chegar a 0, lendo-o de forma at�mica:
			do {
				EA = 0;           // Desabilita interrup��es
				temp = contador;  // L� o valor completo (16 bits) de 'contador'
				EA = 1;           // Habilita interrup��es
			} while(temp != 0);	// Fica em loop enquanto "contador != 0"

      if (temp == 0) // Testa se contador � igual a zero.
				P0 = 0;
      else
        P0 = 0xFF;	// ESTA INSTRU��O NUNCA DEVERIA SER EXECUTADA!
      }
	}
	


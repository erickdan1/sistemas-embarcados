#include <REG51F.H>

// palavra reseravada do ambeinte do 8051 - 'bit'
sbit controle_baixo = P2^0;  // Nomeando o 'bit' 0 da porta P2
sbit controle_alto = P2^1;   // // Nomeando o 'bit' 1 da porta P2

void main()
{
	// evitar que porta P1, utilizada como saída, seja lida
	unsigned char p1_aux = 0xFF;

	while(1) {
		// se bit0 de p2 for igual a 1
		if(controle_baixo)
		{
			// atribui a P1 os 4 bits menos significativos de P0, e mantém os 4 mais significativos de P1
			p1_aux = (P0 & 0x0F) | (p1_aux & 0xF0);
		}
		else
		{
			// zera os 4 bits menos significativos de P1
			p1_aux = p1_aux & 0xF0;
		}
		
		// se bit1 de p2 for igual a 1
		if(controle_alto)
		{
			// atribui a P1 os 4 bits mais significativos de P0, e mantém os 4 menos significativos de P1
			p1_aux = (P0 & 0xF0) | (p1_aux & 0x0F);
		}
		else
		{
			// zera os 4 bits mais significativos de P1
			p1_aux = p1_aux & 0x0F;
		}
		
		P1 = p1_aux;
	}
}
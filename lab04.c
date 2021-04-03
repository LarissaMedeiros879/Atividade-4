#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char *p_portd; /*Ponteiro dos registradores port d */
unsigned char *p_ddrd; /*Ponteiro dos registradores port d */
unsigned char *p_ddrc; /*Ponteiro dos registradores port c */
unsigned char *p_pinc; /*Ponteiro dos registradores port c */
unsigned char *p_cicr; /*Ponteiro do registrador PCICR */
unsigned char *p_cmsk1; /*Ponteiro do registrador PCMSK1 */

unsigned int pulsos; /* Conta a quantidade de pulsos por borda de descida */
unsigned int frpm; /* Estima a velocidade do motor */
unsigned int velocidade; /* Atribui um valor entre 0 e 9 para velocidade estimada frpm a depender do intervalo em que ela se encaixa */

unsigned char display[10] = { /* Vetor com os valores a serem exibidos no display a cada atualizacao da velocidade */
/* O display acende conforme cada segmento e setado em 1, formando os numeros correspondentes em hexadecimal. Os bits 1 a 7 correspondem, respectivamente, aos segmentos de a ate g, formando os numeros no display. Abaixo, tem-se o correspondente em hexadecimal da relacao de segmentos acesos que formam um numero. */
/* g f e d c b a dp - x7 x6 x5 x4 x3 x2 x1 x0*/
0x7F, /* 0 - 01111111 */
0xD, /* 1 - 00001101 */
0xB7, /* 2 - 10110111 */
0x9F, /* 3 - 10011111 */
0xCD, /* 4 - 11001101 */
0xDB, /* 5 - 11011011*/
0xFB, /* 6 - 11111011*/
0xF, /* 7 - 00001111*/
0xFF, /* 8 - 11111111 */
0xDF, /* 9 - 11011111*/
};

/* Rotina de interrupcao do PCINT8*/
ISR(PCINT1_vect) {
  /* Se a rotina do tipo Pin Change esta sendo executada e porque houve uma mudanca de nivel logico. Se o nivel atual e 0, e porque o anterior era 1, logo trata-se de uma borda de descida. */
  if ((*p_pinc & 0x01) == 0x00) {
    
    /* Um pulso corresponde a uma borda de subida seguida de uma de descida. Portanto, a cada borda de descida eu tenho um pulso, contabilizado na variavel a seguir. Toda vez que ocorre uma mudanca de nivel logico, a rotina ira ser executada e, nas bordas de descida, e contabilizada a quantidade de pulsos que ocorreram. */
	pulsos = pulsos + 1;

}

}
void setup() {
  /* Desabilita o atendimento a interrupcoes */
  cli();
  
  /* Atribui aos ponteiros os enderecos do registrador Port D */
  p_portd = (unsigned char *) 0x2B;
  p_ddrd = (unsigned char *) 0x2A;  
  /* Define as portas PDx como saida */
  *p_ddrd |= 0xFF;
  /* Inicia o display desligado */
  *p_portd |= 0x00;

  /* Atribui aos ponteiros os enderecos do registrador Port C */
  p_ddrc = (unsigned char *) 0x27;
  p_pinc = (unsigned char *) 0x26;
  /* Define PC0 como um regisrador de entrada */
  *p_ddrc &= 0xFE;
  
  /* Atribui ao ponteiro o endereco de PCICR */
  p_cicr = (unsigned char *) 0x68;
  /* Interrupcao gerada pelo pino PCINT 18, entre 16 e 23 */
  *p_cicr |= 0x02;

  /* Atribui ao ponteiro o endereco de PCMSK1 */
  p_cmsk1 = (unsigned char *) 0x6C;
  /* Habilita o disparo da interrupcao no pino PCINT8 */
  *p_cmsk1 |= 0x01; 

}

/* Funcao que relaciona o valor mostrado no display a velocidade do motor */
void mostrador() {

/* Os "ifs" pegam apenas a casa da centena da velocidade frpm do motor e atribuem a variavel velocidade o valor da casa da centela, o qual sera exibido no display */
  if (frpm < 100) {
    velocidade = 0;
  }
  else if ((100 <= frpm) && (200 > frpm)) {
	velocidade = 1;
  }
  else if ((200 <= frpm) && (300 > frpm)) {
    velocidade = 2;
  }
  else if ((300 <= frpm) && (400 > frpm)) {
    velocidade = 3;
  }
  else if ((400 <= frpm) && (500 > frpm)) {
    velocidade = 4;
  }    
  else if ((500 <= frpm) && (600 > frpm)) {
  	velocidade = 5;
  }    
  else if ((600 <= frpm) && (700 > frpm)) {
    velocidade = 6;
  }   
  else if ((700 <= frpm) && (800 > frpm)) {
    velocidade = 7;
  }     
  else if ((800 <= frpm) && (900 > frpm)) {
    velocidade = 8;
  }
  else {
    velocidade = 9;
  }
  
  /* Apago o que estava sendo exibido anteriormente no display */
  *p_portd &= 0;
  /* Ligo os segmentos do display correspondentes ao valor obtido para a velocidade */
  *p_portd |= display[velocidade];
  /* Zero a variavel pulso para que uma nova contagem seja iniciada apos o fim do intervalo considerado. */
  pulsos = 0;
}



int main(void){
  setup();
  
  while (1) {
    /* Habilita a interrupcao apenas antes do delay para que a contagem de pulsos nao se inicie muito antes do delay. Ajuda a melhorar a precisao. */
    sei();
    /* Ao inserir o delay, as instrucoes da main param de ocorrer, porem as instrucoes da interrupcao nao. Desse modo, os pulsos durante todo esse periodo sao contados, sendo possivel estimar a velocidade do do motor. */
    _delay_ms(100);
    /* Desabilita a interrupcao depois do delay para que nao haja contagem de pulsos enquanto as outras tarefas sao executadas. Ajuda a melhorar a precisao. */
    cli();
    /* frpm = (numero de pulsos) / (numero de pulsos por volta) * (intervalo) */
    /* Foi informado no enunciado que o numero de pulsos por volta era entre 48 e 49. Com 48, nos limites entre conjuntos de velocidade, o display comeca a oscilar para valores menores em relacao ao 49. Assim, optou-se por usar 49. */
    /* A quantidade de pulsos por intervalo e mapeada na interrupcao. Sao mapeados todos os pulsos que acontecem durante o delay. Logo o intervalo é de 100ms ou 1/600 minutos. */
    frpm = (600*pulsos)/49;
	/* A partir do calculo de frpm, e chamada a funcao que define o valor a ser exibido no display. */
    mostrador();
    /* A funcao mostrador zera a contagem dos pulsos. Assim, a proxima interrupcao comeca a contar os pulsos a partir do zero, durante todo o periodo de delay. Apos esse periodo, a funcao mostrador sera chamada, zerando novamente a contagem de pulsos. */
  }
  
  	return 0;
}
// NANO PORT COM 4 (USB ESQ)
//--------------------------
// Port direct pin 3 -> PORTD[2] 00000$00
// !!! ARDUINO IDE 1.18.6 !!! 
// May fail on loading to board in newer IDE versions
// Older IDE versions not tested

// VERSAO COM INTERRUPCAO DO TIMER1 POR COMPARACAO


#define START_STATE 0 // Estado de inicializacao, tempo para iniciar o receptor
#define START_BIT_STATE 1 // Estado de envio do Start Bit
#define DATA_BITS_STATE 2 // Estado de envio dos Bits do char
#define STOP_BIT_STATE 3 // Estado de envio do Stop Bit

#define STRING_SIZE 100

#define FREQUENCY 1500
#define OCR1A_FROM_FREQUENCY 16000000/FREQUENCY // NÃO MEXER

char string[STRING_SIZE+1] =  "UaSGzuVmckQSbaSWIvVlvpkymWEzNtj8HwH61HYYCFKKRm2eMr6WoscaAWYzqVa0lDS"\
"w0b6DzqLbKO9E4IKPbkY8xZHlfLgfsF8e";

int stringLengthSent; //Qtd de char enviados

int state; // Estado atual do sistema
boolean stateChangeLocker; // Habilita execução do código de um estado

int dataByteIndex; // indice do bit no byte de dado
// @TODO - Não depender disso (Provavelmente dará trabalho)
long sytemStartCounter = 0; // Valor alto entre envio de frases completas;
                            // É necessário para que o buffer do receptor seja capaz de 
                            // "empurrar" valor contido para fora e tenha o índice resetado
                            // no receptor;
                            // Deve ser uma valor maior que OCR1A e suficiente para o buffer,
                            // do receptor printar no console Serial, pois assim o período em Idle
                            // do sistema fará um auto ajuste (após algumas transmissões) na saída
                            // da frase no buffer do receptor caso o sistema seja inciado no meio 
                            // do envio de uma transmissão de dados.

int finishedTransmissions;  // Quantidade de tranmissões realizadas
                        
void setup() {
    // Habilita o Serial
    Serial.begin(115200);
    // Desliga o LED integrado
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);


    DDRD = DDRD | B00000100; // inputs e outputs; pin 3 Output
    PORTD = B00000100; // valor alto ou baixo; pin 3 Alto

    stringLengthSent = 0;

    state = START_STATE;
    stateChangeLocker = 1;
    dataByteIndex = 0;

    // Intervalo entre transmissoes em segundos
    // Com base em OCR1A
    // Calcular segundosDesejados / (1 / (OCR1A / 16000000 / Prescaler) = x 
    // 'x' é o valor a ser usado em sytemStartCounter
    // AQUI CONFIGURA APENAS O INTERVALO ANTES DA PRIMEIRA TRANSMISSAO
    // COLOCAR FREQUENCY * y para y segundos
    sytemStartCounter = (long) FREQUENCY * 5;

    // Desabilita interrupção global
    cli();

    // Inicializa TIMER1
    TCCR1A = 0;
    TCCR1B = 0;
    // Valor inicial do contador do TIMER1
    TCNT1 = 0;

    // Configura TIMER 1 para comparacao com CTC
    // Modo CTC faz comparação direta com contagem ascendente [0->65536]
    // ?? Mais simples de implementar o calculo (em relacao a OverFlow) ?? (creio que sim)
    // ?? desepenho igual ao overflow ?? (a princípio sim)
    TCCR1A = B00000000;    // [7:6] -> Comparacao no Canal A: NORMAL [00];
                            // [5:4] -> Comparacao no Canal B: NORMAL [00];
                            // [3:2] -> RESERVADOS;
                            // [1:0] -> Waveform Generation Mode:
                            //          00 pois com TCCR1B [xxxx1xxx] forma CTC (CLEAR TIME ON COMPARE)
                            //          reset o contador TCNT1 quando a flag de comparacao
                            //          é acionada.
                        
    TCCR1B = B00001001;    // [7]   -> Filtro de INPUT (?? delay de 4 ciclos ??);
                            // [6]   -> Borda de captura do INPUT (0:descida; 1:subida);
                            // [5]   -> RESERVADO;
                            // [4:3] -> Waveform Generation Mode;
                            // [2:0] -> Prescaler (setado em 256).

    TIMSK1 = 0b00000010;   // [7:6] -> RESERVADOS;
                            // [5]   -> Habilita interrupcao por INPUT;
                            // [4:3] -> RESERVADOS;
                            // [2]   -> Habilita interrupcao por COMPARACAO no Canal B;
                            // [1]   -> Habilita interrupcao por COMPARACAO no Canal A;
                            // [0]   -> Habilita interrupcao por OVERFLOW do CONTADOR1;
    
    // Valor a ser comparado no Canal A
    // Com prescaler de 8
    // 16000000 Hz -> Periodo de 62.5ns
    // EXEMPLO: 16000000 Hz/625 (contar até 625) = 3200 Hz -> Periodo de 312.5u
    // FreqDesejada = (16000000/PRESCALER)/y, atribuir y à OCR1A
    OCR1A = OCR1A_FROM_FREQUENCY;  //usar valor par

    // Habilita interrupção global
    sei();
}

void loop() {
    if(stateChangeLocker==1){
        
        // Matém o LED aceso por 5s.
        if(state == START_STATE && stateChangeLocker==1) {
            PORTD = B00000100;
            stateChangeLocker=0;
            //Serial.println("*");
        }
    
        // Faz o envio do Start Bit (nível baixo).
        if(state == START_BIT_STATE && stateChangeLocker==1) {
            PORTD = B00000000;
            stateChangeLocker=0;
            if(stringLengthSent == 0) {
                Serial.println("SENDING...");
            }
            //Serial.print("$0$ ");
        }
    
        // Faz o envio dos Bits do Char;
        // Ao enviar 8 bits (baudDataBitIndex==8), vai para o Stop Bit
        // e incrementa para o próximo caracter da string (stringLengthSent++).
        if(state == DATA_BITS_STATE && stateChangeLocker==1) {
            // Bits de dados (LSB para MSB)
            if((string[stringLengthSent] & (0x01 << dataByteIndex)) != 0) {
                PORTD = B00000100;
                //Serial.print("1");
            } else {
                PORTD = B00000000;
                //Serial.print("0");
            }
                dataByteIndex++;
                
            if(dataByteIndex >= 8) {
                stringLengthSent++;
            }
            stateChangeLocker=0;
        }
    
        // Faz o envio do Stop Bit (nível alto) e volta para o Start Bit;
        // Se a qtd de char enviados for maior ou igual ao tamanho da frase
        // Volta para estado de inicialização do sistema.
        if(state == STOP_BIT_STATE && stateChangeLocker==1) {
            PORTD = B00000100;
            if(stringLengthSent == STRING_SIZE) {
                Serial.println("SENT ");
            }
            //Serial.print(" #1#");
            //Serial.println(string[stringLengthSent-1]);
            stateChangeLocker=0;
        }
    }
}

// Interrupção por COMPARACAO do TIMER/CONTADOR1 (CANAL A)
ISR(TIMER1_COMPA_vect) {
    if(state == START_STATE) {
        sytemStartCounter--;
        if(sytemStartCounter <= 0L) {
            state=START_BIT_STATE;
            stateChangeLocker = 1;
        }
    } else if(state == START_BIT_STATE) {
        state=DATA_BITS_STATE;
        stateChangeLocker = 1;
    } else if(state == DATA_BITS_STATE) {
        if(dataByteIndex >= 8) {
            dataByteIndex = 0;
            state=STOP_BIT_STATE;
        }
        stateChangeLocker = 1;
    } else if(state == STOP_BIT_STATE) {
        if(stringLengthSent >= STRING_SIZE) {
            stringLengthSent = 0;
            state = START_STATE;
            // DELAY ENTRE TRNAMISSÕES EM SEGUNDOS
            // USER FREQUENCY / 2 para 0.5 Segundos
            sytemStartCounter = (long) FREQUENCY * 0.5;
        } else {
            state=START_BIT_STATE;
        }
        stateChangeLocker = 1;
    }
}

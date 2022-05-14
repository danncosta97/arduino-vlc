// MEGA PORT COM 3 or COM 5 (USB DIR)
//--------------------------
// Port direct D2 -> PORTE[4] 000$0000
// !!! ARDUINO IDE 1.18.6 !!! 
// May fail on loading to board in newer IDE versions
// Older IDE versions not tested

// VERSAO COM INTERRUPCAO DO TIMER1 POR COMPARACAO E
// COM INTERRUPCAO POR BORDA DE DESCIDA EM D2

#define IDLE_STATE 0 // Estado de aguardo do proximo Start Bit
#define START_BIT_STATE 1 // Estado de envio do Start Bit
#define DATA_BITS_STATE 2 // Estado de envio dos Bits do char
#define STOP_BIT_STATE 3 // Estado de envio do Stop Bit

#define STRING_SIZE 100

#define FREQUENCY 4000
#define OCR1A_FROM_FREQUENCY 16000000/FREQUENCY // NÃO MEXER
#define OCR1A_FROM_FREQUENCY_HALF OCR1A_FROM_FREQUENCY/2    // NÃO MEXER

#define TRANSMISSION_LIMIT 10   //Quantidade máxima de transmissões

// Delay inicial para pegar o meio do bit (ponto ótimo)
boolean initialdelay;

// Alloca 1300 espaços para char (buffer de recebimento) para evitar multiplos Serial.print
// Vlor máximo devido a memória do emissor
char *stringReceivedBuffer = (char*) calloc(1301, sizeof(char));

char charBits; // O char ASCII de fato (8 bits -> 1 byte)
int bitReceived; // Valor do bit atual
int bytesReceivedAmount; // Quantidade de bits recebido até o momento

int state; // Estado atual do sistema
boolean stateChangeLocker = 0; // Habilita execução do código de um estado

int dataByteIndex; // indice do bit no byte de dado

int startStarted = 1;

boolean idlePrintEnable = 1;

int finishedTransmissions; // Quantidade de tranmissões realizadas

void setup() {
    // Habilita o Serial
    Serial.begin(115200);
    // Desliga o LED integrado
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // Fim de string
    stringReceivedBuffer[STRING_SIZE] = '\0';

    // DIGITAL PIN 7~2.TX.RX
    // E7~E2.TX.RX
    DDRE = DDRE | B00000000; // inputs e outputs; DIGITAL D2 (PORTA E4) input

    charBits = 0;
    bitReceived = 0;
    bytesReceivedAmount = 0;

    state = IDLE_STATE;

    stateChangeLocker = 0;
    dataByteIndex = 0;
    finishedTransmissions = 0;
    
    // Desabilita interrupção global
    cli();

    // ##### Configura TIMER 1 #####
    // Inicializa TIMER1
    TCCR1A = 0;
    TCCR1B = 0;
    // Valor inicial do contador do TIMER1
    TCNT1 = 0;

    // Configura TIMER 1 para comparacao com CTC
    // Modo CTC faz comparação direta com contagem ascendente [0->65536]
    TCCR1A |= B00000000;    // [7:6] -> Comparacao no Canal A: NORMAL [00];
                            // [5:4] -> Comparacao no Canal B: NORMAL [00];
                            // [3:2] -> RESERVADOS;
                            // [1:0] -> Waveform Generation Mode:
                            //          00 pois com TCCR1B [xxxx1xxx] forma CTC (CLEAR TIME ON COMPARE)
                            //          reset o contador TCNT1 quando a flag de comparacao
                            //          é acionada.
                        
    TCCR1B |= B00001001;    // [7]   -> Filtro de INPUT
                            // [6]   -> Borda de captura do INPUT (0:descida; 1:subida);
                            // [5]   -> RESERVADO;
                            // [4:3] -> Waveform Generation Mode;
                            // [2:0] -> Prescaler (setado em 1).
                            
    // Interrupção por timer será habilitada durante a interrupção externa (FALLING EDGE)
    TIMSK1 |= B00000010;    // [7:6] -> RESERVADOS;
                            // [5]   -> Habilita interrupcao por INPUT;
                            // [4:3] -> RESERVADOS;
                            // [2]   -> Habilita interrupcao por COMPARACAO no Canal B;
                            // [1]   -> Habilita interrupcao por COMPARACAO no Canal A;
                            // [0]   -> Habilita interrupcao por OVERFLOW do CONTADOR1;

    // Valor a ser comparado no Canal A
    OCR1A = OCR1A_FROM_FREQUENCY;   //usar valor par de preferencia
    
    //##### Configura interrupção externa (LOW, Any, Falling Edge, Rising Edge) #####
    //[00] LOW LEVELS
    //[01] ANY EDGE
    //[10] FALLING EDGE
    //[11] RISING EDGE
    EICRA = B00000000;  //[7:6] INT3 - PIN18
                        //[5:4] INT2 - PIN19
                        //[3:2] INT1 - PIN20
                        //[1:0] INT0 - PIN21
    EICRB = B00000010;  //[7:6] INT7 - NA
                        //[5:4] INT6 - NA
                        //[3:2] INT5 - PIN3
                        //[1:0] INT4 - PIN2 (D2) - Sendo utilizado no momento
    // Ativa Ext Interrupt para INT7~0
    EIMSK = B00010000;

    // Habilita interrupção global
    sei();
}

void loop() {
    if(stateChangeLocker == 1){
        //Serial.print("STATE[");
        //Serial.print(state);
        //Serial.print("] -- ");
        
        if(state == IDLE_STATE) {
            //Serial.print(">>");
            if (idlePrintEnable == 1){
                //Serial.println("IDLE");
                idlePrintEnable = 0;
            }
            stateChangeLocker = 0;
        }
        
        if(state == START_BIT_STATE) {
            // será sempre habilitada ao final da recepção de um pacote
            cli();
            ///Serial.println(state);
            // Habilita interrupções externas
            EIMSK = B00000000;
            sei();
            //Serial.print("$");
            //Serial.print(PINE>>4 & B1);
            //Serial.print("$ ");
            //Serial.print(state);
            stateChangeLocker=0;
            //if(startStarted==1) {
            //    bytesReceivedAmount=0;
                //Serial.println("");
                //Serial.print("START TRANSMISSION");
                //Serial.println(""); 
            //    startStarted = 0;
            //}
        }
    
        if(state == DATA_BITS_STATE) {
            //Serial.print(PINE>>4 & B1);
            //Serial.print(state);
            bitReceived = PINE>>4 & B1;
            charBits = charBits | (bitReceived << dataByteIndex);
            dataByteIndex++;
            stateChangeLocker=0;
        }
    
        if(state == STOP_BIT_STATE) {
            //Serial.print("AAAA");
            //Serial.print(" #");
            //Serial.print(PINE>>4 & B1);
            //Serial.print("#");

            if(PINE>>4 & B1 == 0) { // Não é um StopBit
                stringReceivedBuffer[bytesReceivedAmount] = '?';
            } else if ((charBits > 47 && charBits < 58) ||
                       (charBits > 64 && charBits < 91) ||
                       (charBits > 96 && charBits < 123)) { // Temos um alfanumerico
                stringReceivedBuffer[bytesReceivedAmount] = charBits;
            } else {    // Temos um falso positivo StopBit
                stringReceivedBuffer[bytesReceivedAmount] = '?';
            }
            bytesReceivedAmount++;
    
            if(bytesReceivedAmount >= STRING_SIZE){
                bytesReceivedAmount = 0;
                Serial.println(stringReceivedBuffer);
                //startStarted = 1; // usar só no debug
                idlePrintEnable = 1;
                finishedTransmissions++;
            }
                
            //Serial.print(charBits);
            charBits = 0;
            stateChangeLocker=0;
            cli();
            ///Serial.println(state);
            // Habilita interrupções externas
            EIMSK = B00010000;
            sei();

            // Esse estado só ocorre entre transmissões completas (ou no início do sistema);
            // Ja que em ISR(TIMER1_COMPA_vect) só entrará em IDLE_STATE
            // quando nao houver um START_BIT (VALOR BAIXO) após STOP_BIT,
            // pois o START_BIT forçará ISR(INT4_vect) antes de fechar o periodo para
            // próxima chamada de ISR(TIMER1_COMPA_vect), onde há o reestabelecimento
            // do timers para ler corretamente o próximo bit;
            // Setar o IDLE_STATE aqui ao invés da próxima interrupção por tempo
            // faz com que ocorra uma auto ajuste com da saída da mensagem (após algumas)
            // tranmissões, caso o receptor seja iniciado no meio de uma transmissão.
            state = IDLE_STATE;

            // Ta bem complexo pensar o porque setar state em IDLE_STATE aqui força
            // esse auto ajuste
        }
        
        if(finishedTransmissions == TRANSMISSION_LIMIT) {
            Serial.print("FINISHED");
            // !!! SOMENTE PARA LIBERAR TODO O ULTIMO BUFFER NO SERIAL !!!
            // NÃO IMPACTA PORQUE A ÚLTIMA TRNAMISSÃO JÁ FOI FINALIZADA
            delay(1000);
            exit(0);
        }
    }
}

// Interrupção externa (FALLING EDGE) em D2
ISR(INT4_vect) {      

    state = START_BIT_STATE;

    // Metade do tempo definido em OCR1A - delay inicial ponto amostragem otimo
    TCNT1 = OCR1A_FROM_FREQUENCY_HALF;
    
    // Com base, em TCNT1, para amostragem no ponto ótimo
    initialdelay = 1;
    
    stateChangeLocker = 0;
}

// Interrupção por COMPARACAO do TIMER/CONTADOR1 (CANAL A)
ISR(TIMER1_COMPA_vect) {
    if(state == IDLE_STATE) {
        stateChangeLocker = 1;
    } else if(state == START_BIT_STATE) {
        stateChangeLocker = 1;
        if(initialdelay == 1) {
            initialdelay = 0;
        } else if (initialdelay == 0) {
            state = DATA_BITS_STATE;
        }
    }  else if(state == DATA_BITS_STATE) {
        stateChangeLocker = 1;
        if(dataByteIndex >= 8) {
            dataByteIndex = 0;
            state = STOP_BIT_STATE;
        }
    }
}

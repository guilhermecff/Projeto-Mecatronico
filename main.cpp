#include "mbed.h"
#include "TextLCD.h"



Ticker toggle;
// PARA X
DigitalOut StepDriverXY(D11);
DigitalOut direcaoX(D12);
DigitalOut enableX(D13);
InterruptIn fdcX1(D10); // xmax
//PARA Y
DigitalOut direcaoY(D7);
DigitalOut enableY(D8);
InterruptIn fdcY1(D5); // ymax
// PARA Z
DigitalOut direcaoZ(PA_11);
DigitalOut enableZ(PA_12);
InterruptIn fdcZ1(D4); // zmax

DigitalIn BotaoZcima(PC_11);
DigitalIn BotaoZbaixo(PC_10);

DigitalIn saveButton(PC_12); 
DigitalIn voltarButton(PD_2); 


AnalogIn EixoXJoyStick(A0);
AnalogIn EixoYJoyStick(A1);


I2C i2c_lcd(PB_9,PB_8);
TextLCD_I2C lcd(&i2c_lcd,0x4e, TextLCD::LCD20x4);



Serial pc(USBTX, USBRX); // declara o objeto pc para comunicação serial


int y, ymax, ymin;
int i = 0;
bool referenciadoX=false;
bool referenciadoY=false;
bool referenciadoZ=false;
int posX=0;
int posY=0;
int posZ=0;
int joyX;
int joyY;
int selecionar = 0;
int mililitros = 1;
int sinalJog = 1;
int lcdref = 0;


int savedPositions[9][4]; // Array to store up to 9 positions [x, y, z]



int savedCount = 0; // This will keep track of how many positions have been saved
int numSaved = 0;



void flip() {
    StepDriverXY = !StepDriverXY;
}

void lcd_show(int state) {
    switch(state) {
        case 0:
        lcd.cls();
        lcd.locate(0,0);
        lcd.printf("Ola \n");
        lcd.printf("Pressione o \n");
        lcd.printf("botao amarelo \n");
        lcd.printf("para referenciar");
        break;
        
        case 1: 
        lcd.cls();
        lcd.printf("Referenciamento \n");
        lcd.printf("sendo \n");
        lcd.printf("executado... \n");
        break;

        case 2:
        lcd.cls();
        lcd.printf("Referenciamento \n");
        lcd.printf("concluido \n");
        lcd.printf("Deseja selecionar a posicao de pega? \n");
        break;

        case 3:
        lcd.cls();
        lcd.printf("Botao de emergencia  \n");
        lcd.printf("Travamento   \n");
        lcd.printf("Automatico   \n");
        break;

        case 4:
        lcd.cls();
        lcd.printf("Coleta Principal\n");
        lcd.printf(" X:%4d\n", posX);
        lcd.printf(" Y:%4d\n", posY);
        lcd.printf(" Z:%4d\n", posZ);
        // lcd.printf("Pressione 'CNFRM'");
        break;

        case 5:
        lcd.cls();
        lcd.printf("Coleta Outros\n");
        lcd.printf(" X:%4d\n ", posX);
        lcd.printf(" Y:%4d\n ", posY);
        lcd.printf(" Z:%4d\n", posZ);
        lcd.printf(" mililitros:%4d\n", mililitros);
        // lcd.printf("Pressione 'CNFRM'");
        break;

        case 6:
        lcd.cls();
        lcd.printf("Quantos\n");
        lcd.printf("mililitros: %3d\n", mililitros);
        lcd.printf("Aperte Amarelo para confirmar");
        break;

    }
}



void refEixoX(){
    enableX=1;
    direcaoX=0;
    while(!referenciadoX){
        if(fdcX1==0){
            referenciadoX=true;
            break;
        }
        enableX=0;
    }
    if (referenciadoX){
        direcaoX=!direcaoX;
        wait(0.2);
        enableX=1;
    }

}
void refEixoY(){
    enableY=1;
    direcaoY=0;
    while(!referenciadoY){
        if(fdcY1==0){
            referenciadoY=true;
            break;
        }
        enableY=0;
    }
    if (referenciadoY){
        direcaoY=!direcaoY;
        wait(0.2);
        enableY=1;
    }

}

void refEixoZ(){
    
    enableZ=1;
    direcaoZ=0;
    while(!referenciadoZ){
        if(fdcZ1==0){
            referenciadoZ=true;
            break;
        }
        enableZ=0;
    }
    if (referenciadoZ){
        direcaoZ=!direcaoZ;
        wait(0.2);
        enableZ=1;
    }

}

void jog(){
    while (sinalJog == 1){
        joyX = EixoXJoyStick.read() * 1000;
        joyY = EixoYJoyStick.read() * 1000;
        
        // Logic for moving X
        if (joyX < 430 && fdcX1 == 1) {
            direcaoX = 1;
            enableX = 0;
            if (StepDriverXY == 1) {
                posX += 1;
            }
        } else if (joyX > 550 && fdcX1 == 1) {
            direcaoX = 0;
            enableX = 0;
            if (StepDriverXY == 1) {
                posX -= 1;
            }
        } else {
            enableX = 1;
        }

        // Logic for moving Y
        if (joyY < 430 && fdcY1 == 1) {
            direcaoY = 1;
            enableY = 0;
            if (StepDriverXY == 1) {
                posY += 1;
            }
        } else if (joyY > 550 && fdcY1 == 1) {
            direcaoY = 0;
            enableY = 0;
            if (StepDriverXY == 1) {
                posY -= 1;
            }
        } else {
            enableY = 1;
        }

        // Logic for moving Z
        if (BotaoZcima == 0 && fdcZ1 == 1) {
            direcaoZ = 0;
            enableZ = 0;
            posZ += 1;
        } else if (BotaoZbaixo == 0 && fdcZ1 == 1) {
            direcaoZ = 1;
            enableZ = 0;
            posZ -= 1;

        } else {
            enableZ = 1;
        }
        if (saveButton == 0){
            sinalJog = 2;
            wait(0.5);
        }
        printf("\r X=%4d \n",posX);
        printf("\r Y=%4d ",posY);
        printf("\r Z=%4d ",posZ);

    }
    while (sinalJog == 2) {
    enableX = 1;
    enableY = 1;
    enableZ = 1;
    if (savedCount == 0) {
        lcd_show(4);
        savedPositions[savedCount][0] = posX;
        savedPositions[savedCount][1] = posY;
        savedPositions[savedCount][2] = posZ;
        for (int a = 0; a < 3; a++) {
            pc.printf("\rsavedPositions[%i][%i]:%i\n", savedCount, a, savedPositions[savedCount][a]);
        }
        savedCount++;
        sinalJog = 1;
    } else if (savedCount > 0 && savedCount <= 9) {
        selecionar = 0;
        while (selecionar == 0) {
            lcd_show(6);
            if (BotaoZcima == 0) {
                mililitros += 1;
                // Add a small delay to avoid rapid increment
                wait(0.2);
            }
            if (BotaoZbaixo == 0) {
                mililitros -= 1;
                // Add a small delay to avoid rapid decrement
                wait(0.2);
            }
            printf("\r mililitros=%4d \n ",mililitros);
            if (saveButton == 0) {
                selecionar = 1;
            }
        }
        lcd_show(5);
        savedPositions[savedCount][0] = posX;
        savedPositions[savedCount][1] = posY;
        savedPositions[savedCount][2] = posZ;
        savedPositions[savedCount][3] = mililitros; // Assuming savedPositions has 4 columns
        savedCount++;
        mililitros = 0;
        wait_ms(1000);

        for (int a = 0; a < 4; a++) { // Looping up to 4 to include mililitros
            pc.printf("\rsavedPositions[%i][%i]:%i\n", savedCount - 1, a, savedPositions[savedCount - 1][a]);
        }
        sinalJog = 1;
        }else{
            sinalJog = 3;
        }
    }
    while (sinalJog==3){
        enableX = 1;
        enableY = 1;
        enableZ = 1;
        break;
    }
}




int main() {
    wait(1);
    toggle.attach(&flip, 0.001); // Interval of 1/1000 of a second
    pc.baud(9600);
    pc.printf("inicio");
    enableX = 1;
    enableY = 1;
    enableZ = 1;
    StepDriverXY = 1;
    lcd.setBacklight(TextLCD::LightOn);


    while (1) {
        if (i==0){
            lcd_show(0);
            if(saveButton==0){
                i = 1;
                wait_ms(1000);
            }

        }
        else if (i == 1) { // Reference positioning
            lcd_show(1);
            refEixoX();
            refEixoY();
            refEixoZ();
          
            i = 2;
        }
        else if (i == 2){
            lcd_show(2);
            if(saveButton==0){
                i = 3;
                wait_ms(1000);
            }

        }

        else if (i == 3) { // Manual control (jog)
            jog();            
        }
    }
}
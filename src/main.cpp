#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include "M5StickV_Speech_Recognition.h"
#include "voice_model.h"

//DMAC usage
//DMAC_CHANNEL0 : FFT
//DMAC_CHANNEL1 : FFT
//DMAC_CHANNEL2 : I2S RX
//DMAC_CHANNEL3 : LCD

#ifdef M5STICKV
#define AXP192_ADDR 0x34
#define PIN_SDA 29
#define PIN_SCL 28
#endif

#define WIDTH 280
#define HEIGHT 240

#define LED_W       7
#define LED_R       6
#define LED_G       9
#define LED_B       8

TFT_eSPI lcd;

M5StickVSpeechRecognizer rec;

void drawText(char* str)
{
    lcd.fillRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);
    lcd.setTextFont(0);
    lcd.setTextSize(1);
    lcd.setCursor(56, 60);
    lcd.setTextColor(TFT_WHITE);
    lcd.printf(str);
}

void lcd_printf(char* str)
{
    drawText(str);
	return;
}

bool axp192_init() {
    Serial.printf("AXP192 init.\n");
    sysctl_set_power_mode(SYSCTL_POWER_BANK3,SYSCTL_POWER_V33);

    Wire.begin((uint8_t) PIN_SDA, (uint8_t) PIN_SCL, 400000);
    Wire.beginTransmission(AXP192_ADDR);
    int err = Wire.endTransmission();
    if (err) {
        Serial.printf("Power management ic not found.\n");
        return false;
    }
    Serial.printf("AXP192 found.\n");

    // Clear the interrupts
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x46);
    Wire.write(0xFF);
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x23);
    Wire.write(0x08); //K210_VCore(DCDC2) set to 0.9V
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x33);
    Wire.write(0xC1); //190mA Charging Current
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x36);
    Wire.write(0x6C); //4s shutdown
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x91);
    Wire.write(0xF0); //LCD Backlight: GPIO0 3.3V
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x90);
    Wire.write(0x02); //GPIO LDO mode
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x28);
    Wire.write(0xF0); //VDD2.8V net: LDO2 3.3V,  VDD 1.5V net: LDO3 1.8V
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x27);
    Wire.write(0x2C); //VDD1.8V net:  DC-DC3 1.8V
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x12);
    Wire.write(0xFF); //open all power and EXTEN
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x23);
    Wire.write(0x08); //VDD 0.9v net: DC-DC2 0.9V
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x31);
    Wire.write(0x03); //Cutoff voltage 3.2V
    Wire.endTransmission();
    Wire.beginTransmission(AXP192_ADDR);
    Wire.write(0x39);
    Wire.write(0xFC); //Turnoff Temp Protect (Sensor not exist!)
    Wire.endTransmission();

    fpioa_set_function(23, (fpioa_function_t)(FUNC_GPIOHS0 + 26));
    gpiohs_set_drive_mode(26, GPIO_DM_OUTPUT);
    gpiohs_set_pin(26, GPIO_PV_HIGH); //Disable VBUS As Input, BAT->5V Boost->VBUS->Charing Cycle

    msleep(20);
    return true;
}

void setup()
{ 
    pll_init();
    plic_init();
    dmac_init();
    uarths_init();
    Serial.begin(115200);
    axp192_init();

    lcd.begin();
    lcd.setRotation(1); // 
//    tft_write_command(INVERSION_DISPALY_ON);

// demo1
//    rec.begin();
//    Serial.begin(115200);
//    Serial.println("start rec...");
//    if( rec.record(0, 0) == 0) //keyword_num, model_num 
//    {    
//      rec.print_model(0, 0);
//    }
//    else 
//        Serial.println("rec failed");

// demo2
    pinMode(LED_R, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(LED_G, OUTPUT);
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, HIGH);
    rec.begin();
    Serial.begin(115200);
    Serial.println("init model...");
    drawText("init model...");

//    uint8_t reg = 0;
//    int res = es8374_read_reg(0x6e, &reg); //flag
//    Serial.printf("reg %u", reg);

    rec.addVoiceModel(0, 0, red_0, fram_num_red_0); 
    rec.addVoiceModel(0, 1, red_1, fram_num_red_1); 
    rec.addVoiceModel(0, 2, red_2, fram_num_red_2); 
    rec.addVoiceModel(0, 3, red_3, fram_num_red_3); 
    rec.addVoiceModel(1, 0, green_0, fram_num_green_0);     
    rec.addVoiceModel(1, 1, green_1, fram_num_green_1);     
    rec.addVoiceModel(1, 2, green_2, fram_num_green_2);     
    rec.addVoiceModel(1, 3, green_3, fram_num_green_3);     
    rec.addVoiceModel(2, 0, blue_0, fram_num_blue_0);   
    rec.addVoiceModel(2, 1, blue_1, fram_num_blue_1);   
    rec.addVoiceModel(2, 2, blue_2, fram_num_blue_2);   
    rec.addVoiceModel(2, 3, blue_3, fram_num_blue_3);   
    rec.addVoiceModel(3, 0, turnoff_0, fram_num_turnoff_0);  
    rec.addVoiceModel(3, 1, turnoff_1, fram_num_turnoff_1);  
    rec.addVoiceModel(3, 2, turnoff_2, fram_num_turnoff_2);  
    rec.addVoiceModel(3, 3, turnoff_3, fram_num_turnoff_3);  
    Serial.println("init model ok!");
    drawText("init model ok!");
}

void loop()
{
    int res;
    res = rec.recognize();
    Serial.printf("res : %d ", res);
    drawText("");
    lcd.setCursor(16, 32);
    lcd.printf("res : %d ", res);
    if (res > 0){
        switch (res)
        {
        case 1:
            digitalWrite(LED_R, LOW); //power on red led
            digitalWrite(LED_G, HIGH);
            digitalWrite(LED_B, HIGH);
            Serial.println("rec : red ");
            drawText("rec : red ");
            break;
        case 2:
            digitalWrite(LED_G, LOW); //power on green led
            digitalWrite(LED_R, HIGH);
            digitalWrite(LED_B, HIGH);
            Serial.println("rec : green ");
            drawText("rec : green ");
            break;
        case 3:
            digitalWrite(LED_B, LOW); //power on blue led
            digitalWrite(LED_R, HIGH);
            digitalWrite(LED_G, HIGH);
            Serial.println("rec : blue ");
            drawText("rec : blue ");
            break;
        case 4:
            digitalWrite(LED_R, HIGH);
            digitalWrite(LED_G, HIGH);
            digitalWrite(LED_B, HIGH);   //power off all leds
            Serial.println("rec : turnoff ");
            drawText("rec : turnoff ");
        default:
            break;
        }
    }else
    {
        Serial.println("recognize failed.");
        drawText("recognize failed.");
    }
    delay(1000);
}

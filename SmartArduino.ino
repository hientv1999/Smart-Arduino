 #include <TouchScreen.h> //touch library
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library
#include "switch_font.c" //graphic for switches library
LCDWIKI_KBV my_lcd(ILI9486,A3,A2,A1,A0,A4); //model,cs,cd,wr,rd,reset
                             /*  r     g    b */
#define BLACK        0x0000  /*   0,   0,   0 */
#define BLUE         0x001F  /*   0,   0, 255 */
#define RED          0xF800  /* 255,   0,   0 */
#define GREEN        0x07E0  /*   0, 255,   0 */
#define CYAN         0x07FF  /*   0, 255, 255 */
#define MAGENTA      0xF81F  /* 255,   0, 255 */
#define YELLOW       0xFFE0  /* 255, 255,   0 */
#define WHITE        0xFFFF  /* 255, 255, 255 */
#define NAVY         0x000F  /*   0,   0, 128 */
#define DARKGREEN    0x03E0  /*   0, 128,   0 */
#define DARKCYAN     0x03EF  /*   0, 128, 128 */
#define MAROON       0x7800  /* 128,   0,   0 */
#define PURPLE       0x780F  /* 128,   0, 128 */
#define OLIVE        0x7BE0  /* 128, 128,   0 */
#define LIGHTGREY    0xC618  /* 192, 192, 192 */
#define DARKGREY     0x7BEF  /* 128, 128, 128 */
#define ORANGE       0xFD20  /* 255, 165,   0 */
#define GREENYELLOW  0xAFE5  /* 173, 255,  47 */
#define PINK         0xF81F  /* 255,   0, 255 */

/******************* UI details */
#define BIG_BUTTON 65 //the half length of big button 
#define SMALL_BUTTON 40 //the half width of small button
#define BUTTON_SPACING_X 20 //the horizontal distance between button
#define BUTTON_SPACING_Y 20  //the vertical distance between button
#define EDG_Y 20 //lower edge distance 
#define EDG_X 20 //left and right distance

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

//touch sensitivity for X
#define TS_MINX 906
#define TS_MAXX 116

//touch sensitivity for Y
#define TS_MINY 92
#define TS_MAXY 952

//touch sensitivity for press
#define MINPRESSURE 10
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

enum St{menu, standby, temphumid, theclock, timer, voltmeter};
St state = menu;
int timer_state = 0;
bool first_time_show{false};
unsigned long long start_time{millis()};
bool reset{true};
TSPoint p;
unsigned long long time_voltage{0};
double voltage{0};
int number {0};
double sum {0};
double reference_voltage{0};
bool reference_voltage_change{false};
enum Shape{sqr, rnd, rect};
uint8_t second_old{0};

typedef struct _button_info
{
     uint8_t name[30];
     uint16_t button_size;  //radius of circle / half side of square
     uint16_t button_width; //width for rectangle
     Shape shape;            //2 = rectangle, 1 = round; 0 = square
     uint8_t name_size;
     uint16_t name_colour;
     uint16_t colour;
     uint16_t x;
     uint16_t y;     
 }button_info;

 
//display string
void show_string(uint8_t *str,int16_t x,int16_t y,uint8_t csize,uint16_t fc, uint16_t bc,boolean mode)
{
    my_lcd.Set_Text_Mode(mode);
    my_lcd.Set_Text_Size(csize);
    my_lcd.Set_Text_colour(fc);
    my_lcd.Set_Text_Back_colour(bc);
    my_lcd.Print_String(str,x,y);
}

void show_picture(const uint8_t *color_content,int16_t size,int16_t x1,int16_t y1,int16_t x2,int16_t y2)
{
    my_lcd.Set_Addr_Window(x1, y1, x2, y2); 
    my_lcd.Push_Any_Color(color_content, size, 1, 1);
}

//Check whether to press or not
boolean is_pressed(int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t px,int16_t py)
{
    if((px > x1 && px < x2) && (py > y1 && py < y2))
    {
        return true;  
    } 
    else
    {
        return false;  
    }
 }

bool check_button_touch(button_info button[], uint16_t size, St state_input[], bool grey)
{
    digitalWrite(13, HIGH);
    p = ts.getPoint();
    digitalWrite(13, LOW);
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
    {
        p.x = map(p.x, TS_MINX, TS_MAXX, my_lcd.Get_Display_Width(),0);
        p.y = map(p.y, TS_MINY, TS_MAXY, my_lcd.Get_Display_Height(),0);
        for(uint16_t i=0;i<size;i++)
        {
            if(is_pressed(button[i].x-button[i].button_size,button[i].y-button[i].button_size,button[i].x+button[i].button_size,button[i].y+button[i].button_size,p.x,p.y))             //press the button
            {
                if (!grey){
                    if (button[i].shape == sqr ){             //button is square
                        my_lcd.Fill_Rect(button[i].x-button[i].button_size, button[i].y-button[i].button_size, button[i].button_size*2, button[i].button_size*2, DARKGREY);
                        show_string(button[i].name, button[i].x-strlen(button[i].name)*button[i].name_size*6/2+button[i].name_size/2+1, button[i].y-button[i].name_size*8/2+button[i].name_size/2+1, button[i].name_size, BLACK, BLACK, 1);
                        delay(100);
                        my_lcd.Set_Draw_color(button[i].colour);
                        my_lcd.Fill_Rect(button[i].x-button[i].button_size, button[i].y-button[i].button_size, button[i].button_size*2, button[i].button_size*2, button[i].colour);
                        show_string(button[i].name, button[i].x-strlen(button[i].name)*button[i].name_size*6/2+button[i].name_size/2+1, button[i].y-button[i].name_size*8/2+button[i].name_size/2+1, button[i].name_size, button[i].name_colour, BLACK, 1);
                    } else if(button[i].shape == rnd) {                            //button is round
                        my_lcd.Set_Draw_color(DARKGREY);
                        my_lcd.Fill_Circle(button[i].x, button[i].y, button[i].button_size);
                        show_string(button[i].name, button[i].x-strlen(button[i].name)*button[i].name_size*6/2+button[i].name_size/2+1, button[i].y-button[i].name_size*8/2+button[i].name_size/2+1, button[i].name_size, BLACK, BLACK, 1);
                        delay(100);
                        my_lcd.Set_Draw_color(button[i].colour);
                        my_lcd.Fill_Circle(button[i].x, button[i].y, button[i].button_size);
                        show_string(button[i].name, button[i].x-strlen(button[i].name)*button[i].name_size*6/2+button[i].name_size/2+1, button[i].y-button[i].name_size*8/2+button[i].name_size/2+1, button[i].name_size, button[i].name_colour, BLACK, 1);
                    } else {                                            //button is rectangle
                        my_lcd.Fill_Rect(button[i].x-button[i].button_size, button[i].y-button[i].button_width, button[i].button_size*2, button[i].button_width*2, DARKGREY);
                        show_string(button[i].name, button[i].x-strlen(button[i].name)*button[i].name_size*6/2+button[i].name_size/2+1, button[i].y-button[i].name_size*8/2+button[i].name_size/2+1, button[i].name_size, BLACK, BLACK, 1);
                        delay(100);
                        my_lcd.Set_Draw_color(button[i].colour);
                        my_lcd.Fill_Rect(button[i].x-button[i].button_size, button[i].y-button[i].button_width, button[i].button_size*2, button[i].button_width*2, button[i].colour);
                        show_string(button[i].name, button[i].x-strlen(button[i].name)*button[i].name_size*6/2+button[i].name_size/2+1, button[i].y-button[i].name_size*8/2+button[i].name_size/2+1, button[i].name_size, button[i].name_colour, BLACK, 1);
                    } 
                }
                
                for (uint16_t j=0; j<size;j++){
                    if(j == i)                  //passing the corresponding state to the number been pressed
                    {
                        state = state_input[j];
                        first_time_show=false;
                        j=size-1;     //to skip the loop faster
                    }
                }
                i=size-1;        //to skip the loop faster   
            }      
        }
    }
    return !first_time_show;
}

void show_buttons(button_info button[], uint16_t size)
{
    for(uint16_t i = 0;i < size;i++)
    {
        if (button[i].shape == sqr){             //button is square
            my_lcd.Fill_Rect(button[i].x-button[i].button_size, button[i].y-button[i].button_size, button[i].button_size*2, button[i].button_size*2, button[i].colour);
            show_string(button[i].name, button[i].x-strlen(button[i].name)*button[i].name_size*6/2+button[i].name_size/2+1, button[i].y-button[i].name_size*8/2+button[i].name_size/2+1, button[i].name_size, button[i].name_colour, BLACK, 1);
        } else if (button[i].shape == rnd) {                            //button is round
            my_lcd.Set_Draw_color(button[i].colour);
            my_lcd.Fill_Circle(button[i].x, button[i].y, button[i].button_size);
            show_string(button[i].name, button[i].x-strlen(button[i].name)*button[i].name_size*6/2+button[i].name_size/2+1, button[i].y-button[i].name_size*8/2+button[i].name_size/2+1, button[i].name_size, button[i].name_colour, BLACK, 1);
        } else {                                //button is rectangle
            my_lcd.Fill_Rect(button[i].x-button[i].button_size, button[i].y-button[i].button_width, button[i].button_size*2, button[i].button_width*2, button[i].colour);
            show_string(button[i].name, button[i].x-strlen(button[i].name)*button[i].name_size*6/2+button[i].name_size/2+1, button[i].y-button[i].name_size*8/2+button[i].name_size/2+1, button[i].name_size, button[i].name_colour, BLACK, 1);
        }
   }
   first_time_show=true;
}

void setup(){
    Serial.begin(9600);
    my_lcd.Init_LCD();  
    pinMode(A5, INPUT);
}

void loop(){
    Serial.println("working");
    state = menu;
    switch (state){
        case standby:
        {
            static button_info buttons[1]={"", 159, 239, rect, 2, BLACK, BLACK, 160, 240};
            static St option[1] = {menu};
            if (first_time_show == false){
                show_buttons(buttons, sizeof(buttons)/sizeof(button_info));
            }
            check_button_touch(buttons, sizeof(buttons)/sizeof(button_info), option, 1);
        }
        break;

        case menu:
        {
            //4 buttons: temp&humid, clock, timer, voltmeter
            static button_info buttons[5]=
                {
                    "temp&humid", BIG_BUTTON, BIG_BUTTON, sqr, 2, BLACK, CYAN, EDG_X+BIG_BUTTON-1, my_lcd.Get_Display_Height()-EDG_Y-BUTTON_SPACING_Y-BIG_BUTTON*3-1,
                    "clock", BIG_BUTTON, BIG_BUTTON, sqr, 2, BLACK, CYAN, EDG_X+BIG_BUTTON*3+BUTTON_SPACING_X-1, my_lcd.Get_Display_Height()-EDG_Y-BUTTON_SPACING_Y-BIG_BUTTON*3-1,
                    "timer", BIG_BUTTON, BIG_BUTTON, sqr, 2, BLACK, CYAN, EDG_X+BIG_BUTTON-1, my_lcd.Get_Display_Height()-EDG_Y-BIG_BUTTON-1,
                    "voltmeter", BIG_BUTTON, BIG_BUTTON, sqr, 2, BLACK, CYAN,EDG_X+BIG_BUTTON*3+BUTTON_SPACING_X-1, my_lcd.Get_Display_Height()-EDG_Y-BIG_BUTTON-1,
                    "", 45, 45, sqr, 2, BLACK, BLACK, 159, 125
                };
            static St option[5] = {temphumid, theclock, timer, voltmeter, standby};
            if (first_time_show==false){
                my_lcd.Fill_Screen(BLACK);
                show_buttons(buttons, sizeof(buttons)/sizeof(button_info));
                //show title
                my_lcd.Set_Draw_color(WHITE);
                my_lcd.Fill_Rectangle(1, 1, my_lcd.Get_Display_Width()-2, 3);
                my_lcd.Fill_Rectangle(1, 45, my_lcd.Get_Display_Width()-2, 50);
                my_lcd.Fill_Rectangle(1, 1, 3, 50);
                my_lcd.Fill_Rectangle(my_lcd.Get_Display_Width()-4, 1, my_lcd.Get_Display_Width()-2, 50);
                show_string("Smart Arduino", CENTER, 20, 2, WHITE, BLACK, 1);
                //show power button
                show_picture(switch_on_big,sizeof(switch_on_big)/2,115,81,204,170);
            }
            Serial.println("Ready to wait");
            while(!check_button_touch(buttons, sizeof(buttons)/sizeof(button_info), option, 0)){};
            Serial.println("Wait finish");
            timer_state =0;
            my_lcd.Fill_Screen(BLACK);
        }
        break;

        case temphumid:
        {
            static button_info buttons[1]={"Back", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, PURPLE, 160, 350};
            static St option[1]={menu};
            if (first_time_show==false){
                show_buttons(buttons, sizeof(buttons)/sizeof(button_info));
                // TODO
                show_string("The temperature is 23C", CENTER, 30, 2, GREENYELLOW, WHITE,1);
                show_string("The humidity is: 69%", CENTER, 80, 2, GREENYELLOW, WHITE,1);
                // TODO
            }
            
            check_button_touch(buttons, sizeof(buttons)/sizeof(button_info), option, 0);
        }
        break;
       
        case theclock:
        {
            static button_info buttons[1]={"Back", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, PURPLE, 160, 350};
            static St option[1]={menu};
            if (first_time_show==false){
                show_buttons(buttons, sizeof(buttons)/sizeof(button_info));
                // TODO
                show_string("It is 12:00pm now", CENTER, 30, 2, GREENYELLOW, BLACK,1);
                // TODO
            }
            check_button_touch(buttons, sizeof(buttons)/sizeof(button_info), option, 0);
        }
        break;

        case timer:
            if (timer_state==2){
                reset = true;
            }
            if (!reset){
                timer_state =1;
            }
            switch (timer_state){
                case 0:     //Get ready
                {
                    static button_info buttons[2]=
                    {
                        "Start", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, PURPLE, 85, 400,
                        "Back", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, CYAN, 235, 400
                    };
                    static St option[2]={timer, menu};
                    if (first_time_show==false){
                        show_buttons(buttons, sizeof(buttons)/sizeof(button_info));
                        show_string("Press to start timer", CENTER, 30, 2, GREENYELLOW, BLACK,1);
                        show_string("00:00:00.000", 55, 100, 3, CYAN, CYAN, 1);
                        my_lcd.Set_Draw_color(WHITE);
                        my_lcd.Draw_Circle(160, 240, 100);
                        show_string("12", CENTER, 145, 2, WHITE, BLACK, 1);
                        show_string("6", CENTER, 321, 2, WHITE, BLACK, 1);
                        show_string("3", 245, 232, 2, WHITE, BLACK, 1);
                        show_string("9", 65, 232, 2, WHITE, BLACK, 1);
                        for (int i=0; i<4; i++){
                            my_lcd.Draw_Line(160-78*cos(3.14*i/2), 240-78*sin(3.14*i/2), 160-72*cos(3.14*i/2), 240-72*sin(3.14*i/2));
                        }
                        my_lcd.Set_Draw_color(GREEN);
                        my_lcd.Draw_Line(160, 175, 160, 240);
                    }
                        // TODO
                    while (!check_button_touch(buttons, sizeof(buttons)/sizeof(button_info), option, 0)){}
                    timer_state=1;
                }
                break;

                case 1:     //display counter
                {
                    if (reset){
                        start_time = millis();
                        reset = false;
                    }
                    int milisec{0}, refresh{0}, intermediate{0};
                    uint8_t second{0}, minute{0}, hour{0};
                    static button_info buttons[2]=
                    {
                        "Stop", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, PURPLE, 85, 400,
                        "Back", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, CYAN, 235, 400
                    };
                    static St option[2]={timer, menu};
                    if (first_time_show==false){
                        my_lcd.Set_Draw_color(WHITE);
                        my_lcd.Draw_Circle(160, 240, 100);
                        show_string("12", CENTER, 145, 2, WHITE, BLACK, 1);
                        show_string("6", CENTER, 321, 2, WHITE, BLACK, 1);
                        show_string("3", 245, 232, 2, WHITE, BLACK, 1);
                        show_string("9", 65, 232, 2, WHITE, BLACK, 1);
                        for (int i=0; i<4; i++){
                            my_lcd.Draw_Line(160-78*cos(3.14*i/2), 240-78*sin(3.14*i/2), 160-72*cos(3.14*i/2), 240-72*sin(3.14*i/2));
                        }
                        my_lcd.Set_Draw_color(BLACK);
                        my_lcd.Fill_Rectangle(30, 20, 290, 50);
                        show_buttons(buttons, sizeof(buttons)/sizeof(button_info));
                    }
                    while (!check_button_touch(buttons, sizeof(buttons)/sizeof(button_info), option, 0)){
                        refresh = 0;
                        intermediate = (millis()-start_time) % 1000;
                        if (intermediate % 10 != milisec % 10){
                            refresh = 1;                 //=1
                        }
                        if ((intermediate / 10) % 10 != (milisec / 10) % 10){
                            refresh = 2;                 //=2
                        }
                        if (intermediate / 100 != milisec / 100){
                            refresh = 3;                 //=3
                        }
                        milisec = intermediate;
                        intermediate = ((millis()-start_time)/1000) % 60;
                        if (intermediate % 10 != second % 10){
                            refresh = 4;                 //=4
                        }
                        if (intermediate / 10 != second / 10){
                            refresh = 5;                 //=5
                        }
                        second = intermediate;
                        intermediate = ((millis()-start_time)/1000/ 60) % 60;
                        if (intermediate % 10 != minute % 10){
                            refresh = 6;                 //=6
                        }
                        if (intermediate / 10 != minute / 10){
                            refresh = 7;                 //=7
                        }
                        minute = intermediate;
                        intermediate = ((millis()-start_time)/1000/60 /60 ) % 24;
                        if (intermediate % 10 != hour % 10){
                            refresh = 8;                 //=8
                        }
                        if (intermediate / 10 != hour / 10){
                            refresh = 9;                 //=9
                        }
                        hour = intermediate;
                        char h[2] = {hour / 10 + '0', hour % 10 + '0'};
                        char minu[2] = {minute / 10 + '0', minute % 10 + '0'} ;
                        char sec[2] = {second / 10 + '0', second % 10 + '0'};
                        char mili[3] = {milisec / 100 + '0', (milisec / 10) % 10 + '0', milisec % 10 + '0'};
                        uint8_t string[15] = {h[0], h[1], ':', minu[0], minu[1], ':', sec[0], sec[1], '.', mili[0], mili[1], mili[2],' ',' ',' '};
                        switch (refresh){
                            case 1:
                            my_lcd.Fill_Rect(250, 100, 20, 25, BLACK);
                            break;

                            case 2:
                            my_lcd.Fill_Rect(235, 100, 40, 25, BLACK);
                            break;

                            case 3:
                            my_lcd.Fill_Rect(215, 100, 60, 25, BLACK);
                            break;

                            case 4:
                            my_lcd.Fill_Rect(215, 100, 60, 25, BLACK);
                            my_lcd.Fill_Rect(180, 100, 17, 25, BLACK);
                            break;

                            case 5:
                            my_lcd.Fill_Rect(215, 100, 60, 25, BLACK);
                            my_lcd.Fill_Rect(160, 100, 40, 25, BLACK);
                            break;

                            case 6:
                            my_lcd.Fill_Rect(215, 100, 60, 25, BLACK);
                            my_lcd.Fill_Rect(160, 100, 40, 25, BLACK);
                            my_lcd.Fill_Rect(125, 100, 17, 25, BLACK);
                            break;

                            case 7:
                            my_lcd.Fill_Rect(215, 100, 60, 25, BLACK);
                            my_lcd.Fill_Rect(160, 100, 40, 25, BLACK);
                            my_lcd.Fill_Rect(105, 100, 40, 25, BLACK);
                            break;

                            case 8:
                            my_lcd.Fill_Rect(215, 100, 60, 25, BLACK);
                            my_lcd.Fill_Rect(160, 100, 40, 25, BLACK);
                            my_lcd.Fill_Rect(105, 100, 40, 25, BLACK);
                            my_lcd.Fill_Rect(70, 100, 20, 25, BLACK);
                            break;

                            case 9:
                            my_lcd.Fill_Rect(215, 100, 60, 25, BLACK);
                            my_lcd.Fill_Rect(160, 100, 40, 25, BLACK);
                            my_lcd.Fill_Rect(105, 100, 40, 25, BLACK);
                            my_lcd.Fill_Rect(50, 100, 20, 40, BLACK);

                            break;
                        }
                        show_string(string, 55, 100, 3, CYAN, CYAN, 1);   
                        if (second_old != second){
                            my_lcd.Set_Draw_color(BLACK);
                            my_lcd.Draw_Line(160+65*sin(3.14*second_old/30), 240-65*cos(3.14*second_old/30), 160, 240);
                            second_old = second;
                            my_lcd.Set_Draw_color(GREEN);
                            my_lcd.Draw_Line(160+65*sin(3.14*second_old/30), 240-65*cos(3.14*second_old/30), 160, 240);
                        }
                    }
                    timer_state=2;
                }
                break;

                case 2:     //display stop time
                {
                    reset = true;
                    static button_info buttons[2]=
                    {
                        "Reset", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, PURPLE, 85, 400,
                        "Back", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, CYAN, 235, 400
                    };
                    static St option[2] = {timer, menu};
                    if (first_time_show==false){
                        show_buttons(buttons, sizeof(buttons)/sizeof(button_info));
                    }
                    while (!check_button_touch(buttons, sizeof(buttons)/sizeof(button_info), option, 0)){}
                    if (state == timer){
                        static button_info buttons_2[2]=
                        {
                          "Start", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, PURPLE, 85, 400,
                          "Back", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, CYAN, 235, 400
                        };
                        show_buttons(buttons_2, sizeof(buttons_2)/sizeof(button_info));
                        show_string("Press to start timer", CENTER, 30, 2, GREENYELLOW, BLACK,1);
                        my_lcd.Fill_Rect(20, 50, 250, 80, BLACK);
                        my_lcd.Set_Draw_color(BLACK);
                        my_lcd.Draw_Line(160+65*sin(3.14*second_old/30), 240-65*cos(3.14*second_old/30), 160, 240);
                        my_lcd.Set_Draw_color(GREEN);
                        my_lcd.Draw_Line(160, 175, 160, 240);
                        show_string("00:00:00.000", 55, 100, 3, CYAN, CYAN, 1);
                        while (!check_button_touch(buttons_2, sizeof(buttons_2)/sizeof(button_info), option, 0)){}
                    }
                    timer_state=1;
                }
                break;
            }
        break;
            
        case voltmeter:
        {
            static button_info buttons[3]={"Back", BIG_BUTTON, BIG_BUTTON, rnd, 3, BLACK, PURPLE, 240, 400,
                                           "GND REF", BIG_BUTTON, BIG_BUTTON/2-5, rect, 2, BLACK, BLUE, 80, 370,
                                           "RESET REF", BIG_BUTTON, BIG_BUTTON/2-5, rect, 2, BLACK, BLUE, 80, 440};
            static St option[3]={menu, voltmeter, theclock};
            if (!first_time_show){
                my_lcd.Set_Draw_color(WHITE);
                my_lcd.Draw_Circle(160, 225, 140);
                my_lcd.Set_Draw_color(BLACK);
                my_lcd.Fill_Rectangle(20, 225, 300, 365);
                show_buttons(buttons, sizeof(buttons)/sizeof(button_info));
                // TODO
                show_string("Voltmeter", CENTER, 30, 3, GREENYELLOW, BLACK,1);
                show_string("0", 5, 215, 2, WHITE, BLACK, 1);
                show_string("5", 305, 215, 2, WHITE, BLACK, 1);
                show_string("2.5", CENTER, 67, 2, WHITE, BLACK, 1);
                show_string("1.25", 12, 110, 2, WHITE, BLACK, 1);
                show_string("3.75", 265, 110, 2, WHITE, BLACK, 1);
                my_lcd.Set_Draw_color(WHITE);
                for (int i=0; i<=4; i++){
                    my_lcd.Draw_Line(160-140*cos(3.14*i/4), 225-140*sin(3.14*i/4), 160-120*cos(3.14*i/4), 225-120*sin(3.14*i/4));
                    if (i <4){
                      for (int j=1; j<=4; j++){
                        my_lcd.Draw_Line(160-140*cos(3.14*i/4+3.14*j/20), 225-140*sin(3.14*i/4+3.14*j/20), 160-130*cos(3.14*i/4+3.14*j/20), 225-130*sin(3.14*i/4+3.14*j/20));
                      }
                    }
                }
                time_voltage=millis();
                reference_voltage = 0;
                my_lcd.Print_String("Ground Reference: ", 20, 280);
                my_lcd.Print_String("0.00" , 235, 280);        //print default reference GND
                my_lcd.Print_String("V", 285, 280);            //print unit for reference GND
                my_lcd.Print_String("0.00", 140, 250);         //print default voltage
                my_lcd.Print_String("V", 190, 250);            //print unit for voltage
                my_lcd.Draw_Fast_HLine(0, 320, 320);             //draw horizontal line
                show_picture(switch_off_small,sizeof(switch_off_small)/2,70,240,84,254);
                show_picture(switch_off_small,sizeof(switch_off_small)/2,236,240,250,254);
                my_lcd.Draw_Line(45, 225, 160, 225);
            }
            while (!check_button_touch(buttons, sizeof(buttons)/sizeof(button_info), option, 0)){

                double new_voltage = analogRead(A5)/1023.0;
                number ++;
                sum += new_voltage;
                if (millis()-time_voltage >= 100){
                    if (abs(voltage - sum/number) > 0.01 || reference_voltage_change){
                        my_lcd.Set_Draw_color(BLACK);
                        my_lcd.Draw_Line(160-115*cos(3.14*(voltage-reference_voltage)), 225-115*sin(3.14*(voltage-reference_voltage)), 160, 225);      //erase old needle 
                        my_lcd.Draw_Line(45, 225, 160, 225);        //erase the 0 needle in case lagging
                        my_lcd.Fill_Rectangle(100, 250, 185, 270);  //erase old voltage
                        my_lcd.Set_Text_colour(WHITE);
                        my_lcd.Set_Draw_color(WHITE);
                        double average_voltage = sum/number;
                        voltage = average_voltage;
                        if (average_voltage < reference_voltage){           //voltage will be negative so display 0V
                            average_voltage = reference_voltage;
                            show_picture(switch_on_small,sizeof(switch_on_small)/2,70,240,84,254);
                        } else {
                            show_picture(switch_off_small,sizeof(switch_off_small)/2,70,240,84,254);
                        }
                        if (average_voltage > 1){
                            show_picture(switch_on_small,sizeof(switch_on_small)/2,236,240,250,254);
                        } else {
                            show_picture(switch_off_small,sizeof(switch_off_small)/2,236,240,250,254);
                        }
                        my_lcd.Draw_Line(160-115*cos(3.14*(average_voltage-reference_voltage)), 225-115*sin(3.14*(average_voltage-reference_voltage)), 160, 225);       //draw needle
                        my_lcd.Print_String(String((average_voltage-reference_voltage)*5), 140, 250);                                                                   //print voltage
                        reference_voltage_change = false;
                    }
                    time_voltage = millis();
                    sum = 0;
                    number = 0;
                }
            }
            if (state != menu){
                my_lcd.Set_Draw_color(BLACK);
                if (voltage > reference_voltage){
                    my_lcd.Draw_Line(160-115*cos(3.14*(voltage-reference_voltage)), 225-115*sin(3.14*(voltage-reference_voltage)), 160, 225);      //erase old needle 
                } else {
                    my_lcd.Draw_Line(45, 225, 160, 225); 
                }
                my_lcd.Fill_Rectangle(235, 275, 280, 300);                                                                                     //erase old reference voltage
                if (state == theclock){
                    reference_voltage = 0;
                } else {
                    reference_voltage = voltage;
                }
                state = voltmeter;
                my_lcd.Set_Text_colour(WHITE);
                my_lcd.Print_String(String(reference_voltage*5) , 235, 280);       //print reference GND
                first_time_show = true;
                reference_voltage_change = true;
            }

        }
        break;
    }
    Serial.println("working2");
}

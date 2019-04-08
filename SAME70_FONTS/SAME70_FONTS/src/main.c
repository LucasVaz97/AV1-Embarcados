#include "asf.h"
#include <asf.h>
#include "tfont.h"
#include "sourcecodepro_28.h"
#include "calibri_36.h"
#include "arial_72.h"

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/

/**
 *  Informacoes para o RTC
 *  poderia ser extraida do __DATE__ e __TIME__
 *  ou ser atualizado pelo PC.
 */
#define YEAR        2018
#define MOUNTH      3
#define DAY         19
#define WEEK        12
#define HOUR        15
#define MINUTE      59
#define SECOND      50

/**
* LEDs
*/
#define LED_PIO_ID	   ID_PIOC
#define LED_PIO        PIOC
#define LED_PIN		   30
#define LED_PIN_MASK   (1<<LED_PIN)

#define LED1_PIO_ID	   ID_PIOC
#define LED1_PIO        PIOC
#define LED1_PIN		   8
#define LED1_PIN_MASK   (1<<LED1_PIN)

/**
* Botão
*/
#define BUT_PIO_ID			  ID_PIOA
#define BUT_PIO				  PIOA
#define BUT_PIN				  19
#define BUT_PIN_MASK			  (1 << BUT_PIN)
#define BUT_DEBOUNCING_VALUE  79

/************************************************************************/
/* VAR globais                                                          */
/************************************************************************/

volatile uint8_t flag_led0 = 1;

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/
void font_draw_text(tFont *font, const char *text, int x, int y, int spacing);
void configure_lcd(void);
void BUT_init(void);
void LED_init(int estado);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
void RTC_init(void);
void pin_toggle(Pio *pio, uint32_t mask);

/************************************************************************/
/* Handlers                                                             */
/************************************************************************/

/**
*  Handle Interrupcao botao 1
*/

volatile int loops=-1;
volatile double distancia=0;
volatile double velocidade=0;
static void Button1_Handler(uint32_t id, uint32_t mask)
{
	loops = loops +1;
	distancia = distancia+2*3.14159265*0.5*loops;
	velocidade=(loops*2*3.14159265)/4;
}

/**
*  Interrupt handler for TC1 interrupt.
*/

volatile int segundos = SECOND;
volatile int minutos = MINUTE;
volatile int horas = HOUR;

void TC1_Handler(void){
	volatile uint32_t ul_dummy;

	/****************************************************************
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	******************************************************************/
	ul_dummy = tc_get_status(TC0, 1);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);
	int someInt3 = segundos;
	char str3[12];
	sprintf(str3, "%d", someInt3);
	int someInt4 = minutos;
	char str4[12];
	sprintf(str4, "%d", someInt4);
	
	int someInt5 = horas;
	char str5[12];
	sprintf(str5, "%d", someInt5);
	
	
	if (segundos>=60)
	
	{	configure_lcd();
		segundos=0;
		minutos= minutos+1;
		
	}
	if(minutos>=60){
		configure_lcd();
		horas=horas+1;
		minutos=0;
		
	}
	
	
	
	
	font_draw_text(&calibri_36,"Horario", 50, 300, 1);
	font_draw_text(&calibri_36,str3, 150, 350, 1);
	font_draw_text(&calibri_36,str4, 100, 350, 1);
	font_draw_text(&calibri_36,str5, 50, 350, 1);
	segundos=segundos+1;
	

	/** Muda o estado do LED */
	
	if(flag_led0){
		pin_toggle(LED_PIO, LED_PIN_MASK);
		}
	
}
void TC0_Handler(void){
	volatile uint32_t ul_dummy;

	/****************************************************************
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	******************************************************************/
	ul_dummy = tc_get_status(TC0, 0);

	/* Avoid compiler warning */
	UNUSED(ul_dummy);

	/** Muda o estado do LED */
	if(flag_led0){
		pin_toggle(LED1_PIO, LED1_PIN_MASK);}
}

/**
* \brief Interrupt handler for the RTC. Refresh the display.
*/
void RTC_Handler(void)
{
	uint32_t ul_status = rtc_get_status(RTC);

	/*
	*  Verifica por qual motivo entrou
	*  na interrupcao, se foi por segundo
	*  ou Alarm
	*/
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		
		int someInt = distancia;
		char str[12];
		sprintf(str, "%d", someInt);
		
		int someInt2 = velocidade;
		char str2[12];
		sprintf(str2, "%d", someInt2);
		configure_lcd();
		font_draw_text(&calibri_36,"Distancia", 50, 100, 1);
		font_draw_text(&calibri_36,str, 50, 150, 1);
		font_draw_text(&calibri_36,"Velocidade", 50, 200, 1);
		font_draw_text(&calibri_36,str2, 50, 250, 1);
		rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
		rtc_set_date_alarm(RTC, 1, MOUNTH, 1, DAY);
		uint32_t hora, min=MINUTE, sec=SECOND;
		rtc_get_time(RTC, &hora, &min, &sec);
				if(sec>=60){
					sec=0;
					min=min+1;
				}
				else{
					sec= sec+4;
					velocidade=0;
				}
				if(min>=60){
					min=0;
					hora=hora+1;
				}
				
				rtc_set_time_alarm(RTC, 1, hora, 1, min, 1, sec);
		
		if(flag_led0){

			tc_stop(TC0, 0);
			flag_led0 = 0;
		}
		else{
			tc_start(TC0, 1);
			tc_start(TC0, 0);
			flag_led0 = 1;
		}
	}
	
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
	
}


/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/

/**
*  Toggle pin controlado pelo PIO (out)
*/
void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

/**
* @Brief Inicializa o pino do BUT
*/
void BUT_init(void){
	/* config. pino botao em modo de entrada */
	pmc_enable_periph_clk(BUT_PIO_ID);
	pio_set_input(BUT_PIO, BUT_PIN_MASK, PIO_PULLUP | PIO_DEBOUNCE);

	/* config. interrupcao em borda de descida no botao do kit */
	/* indica funcao (but_Handler) a ser chamada quando houver uma interrupção */
	pio_enable_interrupt(BUT_PIO, BUT_PIN_MASK);
	pio_handler_set(BUT_PIO, BUT_PIO_ID, BUT_PIN_MASK, PIO_IT_FALL_EDGE, Button1_Handler);

	/* habilita interrupçcão do PIO que controla o botao */
	/* e configura sua prioridade                        */
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 1);
};

/**
* @Brief Inicializa o pino do LED
*/
void LED_init(int estado){
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_PIN_MASK, estado, 0, 0 );
	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_set_output(LED1_PIO, LED1_PIN_MASK, estado, 0, 0 );
};

/**
* Configura TimerCounter (TC) para gerar uma interrupcao no canal (ID_TC e TC_CHANNEL)
* na taxa de especificada em freq.
*/
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	uint32_t channel = 1;

	/* Configura o PMC */
	/* O TimerCounter é meio confuso
	o uC possui 3 TCs, cada TC possui 3 canais
	TC0 : ID_TC0, ID_TC1, ID_TC2
	TC1 : ID_TC3, ID_TC4, ID_TC5
	TC2 : ID_TC6, ID_TC7, ID_TC8
	*/
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  4Mhz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura e ativa interrupçcão no TC canal 0 */
	/* Interrupção no C */
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);

	/* Inicializa o canal 0 do TC */
	tc_start(TC, TC_CHANNEL);
}

/**
* Configura o RTC para funcionar com interrupcao de alarme
*/
void RTC_init(){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(RTC, YEAR, MOUNTH, DAY, WEEK);
	rtc_set_time(RTC, HOUR, MINUTE, SECOND);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 0);
	NVIC_EnableIRQ(RTC_IRQn);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(RTC,  RTC_IER_ALREN);

}



struct ili9488_opt_t g_ili9488_display_opt;

void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
	
}


void font_draw_text(tFont *font, const char *text, int x, int y, int spacing) {
	char *p = text;
	while(*p != NULL) {
		char letter = *p;
		int letter_offset = letter - font->start_char;
		if(letter <= font->end_char) {
			tChar *current_char = font->chars + letter_offset;
			ili9488_draw_pixmap(x, y, current_char->image->width, current_char->image->height, current_char->image->data);
			x += current_char->image->width + spacing;
		}
		p++;
	}
}



/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/
int main(void){
	
	
	board_init();
	sysclk_init();
	configure_lcd();
	
	
	/* Initialize the SAM system */
	sysclk_init();

	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	
	
	/* Configura Leds */
	LED_init(0);

	/* Configura os botões */
	BUT_init();

	/** Configura RTC */
	RTC_init();

	/* configura alarme do RTC */
	rtc_set_date_alarm(RTC, 1, MOUNTH, 1, DAY);
	rtc_set_time_alarm(RTC, 1, HOUR, 1, MINUTE, 1, SECOND+4);
	
	/** Configura timer TC0, canal 1 */
	TC_init(TC0, ID_TC1, 1, 1);
	TC_init(TC0, ID_TC0, 0, 1);


	while (1) {
		/* Entrar em modo sleep */
		//pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);

	}

}

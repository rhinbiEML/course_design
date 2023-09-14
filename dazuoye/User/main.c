#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/TIMER/btim.h"
#include "./MAP/map.h"
#include "time.h"

#define SCREEN_WIDTH lcddev.width
#define SCREEN_HEIGHT lcddev.height
#define OBSTACLE_SIZE 5
#define CHARACTER_SIZE 2

uint16_t rtime=0;

uint8_t draw_pilar(Node *node,uint8_t path_width,int color,uint16_t position){
		uint16_t left,right,down,up;
		if(node->x>=path_width)
				left = node->x - path_width;
		else left=0;
		right = node->x + path_width;
		down = node->y + OBSTACLE_SIZE;
		if(node->y>=OBSTACLE_SIZE)
				up = node->y - OBSTACLE_SIZE;
		else up=0;
		lcd_draw_line(0,up,left,up,color);
		lcd_draw_line(right,up,SCREEN_WIDTH,up,color);
		lcd_draw_line(0,down,left,down,color);
		lcd_draw_line(right,down,SCREEN_WIDTH,down,color);
		lcd_draw_line(left,up,left,down,color);
		lcd_draw_line(right,up,right,down,color);
		if(up <=240&&down >=240)
				if(position-CHARACTER_SIZE<left || position+CHARACTER_SIZE >right)
					return 1;
		return 0;
}

uint8_t stepper(List *list,uint16_t path_width, uint16_t threshold,uint16_t position) {
    Node *current = list->head->next;
    while (current != list->tail) {
        // 用白色覆盖之前的形状
				draw_pilar(current,path_width,WHITE,position);
        // 更新障碍物位置
        current->y++;
				if(draw_pilar(current,path_width,BLACK,position))
						return 1;
        if (current->y > threshold + OBSTACLE_SIZE) {
            Node *temp = current;
            current = current->next;
            delete_node(list, temp);
        } else {
            current = current->next;
        }
    }
		return 0;
}

void clearList(List *list) {
    if (list->head == NULL || list->tail == NULL) {
        return;
    }
    Node *current = list->head->next;
    while (current != list->tail) {
        Node *next = current->next;
        free(current);
        current = next;
    }
    list->head->next = list->tail;
    list->tail->prev = list->head;
}

void BTIM_TIMX_INT_IRQHandler(void)
{ 
	if (BTIM_TIMX_INT->SR & 0X0001)   /* 溢出中断 */
	{
		rtime++;
	}

	BTIM_TIMX_INT->SR &= ~(1 << 0);   /* 清除中断标志位 */
} 

void btim_timx_int_init(uint16_t arr, uint16_t psc)
{
	BTIM_TIMX_INT_CLK_ENABLE();
	BTIM_TIMX_INT->ARR = arr;           /* 设定计数器自动重装值 */
	BTIM_TIMX_INT->PSC = psc;           /* 设置预分频器  */
	BTIM_TIMX_INT->DIER |= 1 << 0;      /* 允许更新中断 */
	BTIM_TIMX_INT->CR1 |= 1 << 0;       /* 使能定时器TIMX */
	sys_nvic_init(1, 3, BTIM_TIMX_INT_IRQn, 2); /* 抢占1，子优先级3，组2 */
}

int main(){
	sys_stm32_clock_init(9);    /* 设置时钟, 72Mhz */
	delay_init(72);             /* 延时初始化 */
	usart_init(72, 115200);     /* 串口初始化为115200 */
	led_init();                 /* 初始化LED */
	lcd_init();                 /* 初始化LCD */
	key_init();
	List *map = create_map();
	uint8_t mode = 1;
	uint8_t drawed = 0;
	uint16_t seed = 0;
	uint16_t time=0;
	uint16_t path_position=0;
	uint16_t path_size=0;
	uint16_t move_extent=0;
	uint16_t position=0;
	//uint16_t char_pos[2]={0,0};
	while(1){
		uint8_t key = key_scan(0);
		switch(mode)
		{
			case(1):	//菜单界面
				seed++;
				if(drawed==0){
					lcd_clear(WHITE);
					lcd_show_string(SCREEN_WIDTH/2,SCREEN_HEIGHT/2,240, 32,16,"start",BLACK);
					drawed=1;
				}
				if(key==KEY1)
				{
					clearList(map);
					mode++,
					time=0;
					rtime=0;
					srand(seed);
					lcd_clear(WHITE);
					path_position = SCREEN_WIDTH / 2;
					move_extent = 1;
					path_size=3*CHARACTER_SIZE;
					position = SCREEN_WIDTH/2;
					drawed=0;
					btim_timx_int_init(10000 - 1, 7200 - 1); /* 10Khz的计数频率，计数1w次为1s */
				}
				break;
			
			case(2):	//游戏中
				key=key_scan(1);
				lcd_show_string(0,0,SCREEN_WIDTH,16,16,"In Game",BLACK);
				time++;
				delay_us(1000);
				lcd_show_xnum(0,16,time,16,16,0,BLACK);
				lcd_show_xnum(0,32,rtime,16,16,0,BLACK);

				// 定义变量来存储a和b的值

				path_position=(SCREEN_WIDTH-move_extent) / 2+rand() % move_extent;
				if (time%100)
					if (move_extent<SCREEN_WIDTH)
						move_extent++;

				// 确保活路不靠近边界
				if (path_position < CHARACTER_SIZE) {
					path_position = CHARACTER_SIZE;
				} else if (path_position > SCREEN_WIDTH - CHARACTER_SIZE) {
					path_position = SCREEN_WIDTH - CHARACTER_SIZE;
				}
				lcd_draw_circle(position,240,CHARACTER_SIZE,WHITE);
				switch(key)
				{		
					case KEY0_PRES:
						if(position >CHARACTER_SIZE+2) position -=2;
						break;
					case KEY1_PRES:
						if(position <SCREEN_WIDTH-CHARACTER_SIZE-2) position +=2;
						break;
					case WKUP_PRES:
						while(key_scan(1)!=KEY1_PRES){
							if(key_scan(1)==KEY0_PRES){
								mode--;
								break;}
						}
						break;
				
				}
				lcd_draw_circle(position,240,CHARACTER_SIZE,RED);
				if(stepper(map,path_size,SCREEN_HEIGHT,position))
					mode++;
				// 向链表中添加障碍物
				if(time%150==0)
				{
					add_path(map, SCREEN_WIDTH, path_position);
				}

				break;
			case(3):
				lcd_show_string(SCREEN_WIDTH/3,SCREEN_HEIGHT/3,240,16,24,"WASTED",BLACK);
				if(key_scan(0)==KEY1_PRES)
					mode=1;
				break;
		}
	}
}

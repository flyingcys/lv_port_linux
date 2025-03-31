#include "lvgl/lvgl.h"
#include "stdlib.h"
#include "qieshuiguo.h"

#define max_fruit  10//���ӳش�С
#define max_act_fruit  8//����׳�������
#define min_act_fruit  1//��С�׳�������

typedef struct
{
	lv_obj_t * obj;
	int x;
	int y;
	int delay;
	int angle;
	uint8_t alive;
  uint8_t img_id;
}fruit_obj_type;


LV_IMG_DECLARE(xigua_img)
LV_IMG_DECLARE(banana_img)
LV_IMG_DECLARE(apple_img)
LV_IMG_DECLARE(pear_img)
LV_IMG_DECLARE(dragon_img)
LV_IMG_DECLARE(split_xigua_img)
LV_IMG_DECLARE(split_banana_img)
LV_IMG_DECLARE(split_apple_img)
LV_IMG_DECLARE(split_pear_img)
LV_IMG_DECLARE(split_dragon_img)
LV_IMG_DECLARE(yellow_blood_img)
LV_IMG_DECLARE(red_blood_img)
LV_IMG_DECLARE(pink_blood_img)
LV_IMG_DECLARE(green_blood_img)
LV_IMG_DECLARE(boom_img)
LV_IMG_DECLARE(start_img)

static const lv_img_dsc_t * blood_img_lib[5]={&red_blood_img,&yellow_blood_img,&green_blood_img,&yellow_blood_img,&pink_blood_img};
static const lv_img_dsc_t * fruit_img_lib[6]={&xigua_img,&banana_img,&apple_img,&pear_img,&dragon_img,&boom_img};
static const lv_img_dsc_t * split_fruit_img_lib[5]={&split_xigua_img,&split_banana_img,&split_apple_img,&split_pear_img,&split_dragon_img};
static lv_timer_t * timer_creat_fruit,* timer_creat_boom;
static  lv_point_t  blade_point[22]={0,};
static int point_count=0;
static lv_obj_t * split1,* split2;
static fruit_obj_type fruit_obj[max_fruit]={0,};
static int score,ok;
static lv_obj_t * screen1,* game_window,*start_btn,*bgmap,*blade_track,*exit_btn,*coin,*score_lable;
static float screen_ratio;
static void	game_init();


static void x_move_cb(void * var, int32_t v);
static void y_move_cb(void * var, int32_t v);
static void angle_cb(void * var, int32_t v);
static void exchange_done_cb(lv_anim_t * a);
static void same_color_move_to_coin();
static int same_color_check();
static void obj_move_down();
static void set_obj_userdata();
static void move_deleted_cb(lv_anim_t * a);
static bool map_is_full();
static bool has_same_color();
static void map_del_all();
static void clike_screen_cb(lv_event_t * e);
static void exit_game_cb(lv_event_t * e);
static void clear_all_clickable();
static void add_all_clickable();
static void move_to_coin_end_cb(lv_anim_t * a);
static void flash_end_cb(lv_anim_t * a);
static void flash_cb(void * var, int32_t v);
static void same_color_flash();
static void release_screen_cb(lv_event_t * e);
static void creat_fruit_cb(lv_timer_t * t);
static void split_fruit_deleted_cb(lv_anim_t * a);
static void fruit_deleted_cb(lv_anim_t * a);
static void split_angle_cb(void * var, int32_t v);
static void split_x_move_cb(void * var, int32_t v);
static void split_y_move_cb(void * var, int32_t v);
static void disappear_cb(void * var, int32_t v);
static void creat_boom_cb(lv_timer_t * t);
static void clear_all_fruit();
static void screen_disappear_cb(void * var, int32_t v);
static void start_game_cb(lv_event_t * e);





LV_IMG_DECLARE(refs_btn_img)
// LV_IMG_DECLARE(exit_img)
LV_IMG_DECLARE(fruit_bg_img)
LV_IMG_DECLARE(xiaoxiaole_bg_img)

void qieshuiguo()
{
		lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    screen_ratio=(float)lv_disp_get_hor_res(lv_disp_get_default())/800;

		screen1=lv_tileview_create(lv_scr_act());
		lv_obj_set_style_bg_color(screen1,lv_color_hex(0x000000), LV_PART_MAIN);
		lv_obj_clear_flag(screen1, LV_OBJ_FLAG_SCROLLABLE);

		lv_obj_add_event_cb(screen1,clike_screen_cb,LV_EVENT_PRESSING,0);
    lv_obj_add_event_cb(screen1,release_screen_cb,LV_EVENT_RELEASED,0);

	  bgmap=lv_img_create(screen1);
		lv_img_set_src(bgmap, &fruit_bg_img);
    lv_obj_set_size(bgmap,lv_disp_get_hor_res(lv_disp_get_default()),lv_disp_get_ver_res(lv_disp_get_default())+50);

		// exit_btn=lv_img_create(screen1);
		// lv_img_set_src(exit_btn, &exit_img);
		// lv_obj_set_align(exit_btn,LV_ALIGN_TOP_RIGHT);
		// lv_obj_add_flag(exit_btn, LV_OBJ_FLAG_CLICKABLE);
		// lv_obj_add_event_cb(exit_btn,exit_game_cb,LV_EVENT_CLICKED,0);

		score=0;

		score_lable=lv_label_create(screen1);
		lv_label_set_text_fmt(score_lable,"SCORE:%d",score);
		lv_obj_set_style_text_font(score_lable,&lv_font_montserrat_22,0);
		lv_obj_set_style_text_color(score_lable, lv_color_hex(0x00aaff), LV_PART_MAIN);


		start_btn=lv_img_create(screen1);
		lv_img_set_src(start_btn, &start_img);
		lv_obj_center(start_btn);
		lv_obj_add_flag(start_btn, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_event_cb(start_btn,start_game_cb,LV_EVENT_CLICKED,0);

		timer_creat_fruit=lv_timer_create(creat_fruit_cb, 3000, 0);
		timer_creat_boom=lv_timer_create(creat_boom_cb, 10000, 0);

		lv_timer_pause(timer_creat_fruit);
		lv_timer_pause(timer_creat_boom);

}

static void	start_game_cb(lv_event_t * e)
{
  lv_obj_t * xxx=(lv_obj_t *)e->target;
	lv_timer_resume(timer_creat_fruit);
	lv_timer_resume(timer_creat_boom);
	lv_obj_add_flag(xxx, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(screen1, LV_OBJ_FLAG_CLICKABLE);
}

static void clike_screen_cb(lv_event_t * e)
{
	lv_indev_get_point(lv_indev_get_act(),&blade_point[point_count]);


	for(int i=0;i<max_fruit;i++)
	{

		if(fruit_obj[i].alive==1)
			{
  				if((blade_point[point_count].x-fruit_obj[i].x<lv_disp_get_ver_res(lv_disp_get_default())/4)&&(blade_point[point_count].x-fruit_obj[i].x>-20)
					 &&(blade_point[point_count].y-fruit_obj[i].y<100)&&(blade_point[point_count].y-fruit_obj[i].y>0))
					{

						if(fruit_obj[i].img_id==5)
							{
								lv_timer_pause(timer_creat_fruit);
								lv_timer_pause(timer_creat_boom);
								lv_anim_del_all();

								lv_obj_t *white_screen=lv_tileview_create(screen1);
								lv_obj_set_style_bg_color(white_screen,lv_color_hex(0xffffff), LV_PART_MAIN);
								lv_obj_set_style_bg_opa(white_screen,255, LV_PART_MAIN);
								lv_obj_clear_flag(white_screen, LV_OBJ_FLAG_SCROLLABLE);

								lv_anim_t a;
								lv_anim_init(&a);
								lv_anim_set_var(&a,white_screen);
								lv_anim_set_exec_cb(&a,screen_disappear_cb);
								lv_anim_set_time(&a,4000);
								lv_anim_set_delay(&a,1000);
								lv_anim_set_values(&a,255,0);
								lv_anim_set_deleted_cb(&a, split_fruit_deleted_cb);
								lv_anim_start(&a);

								clear_all_fruit();
								lv_obj_clear_flag(screen1, LV_OBJ_FLAG_CLICKABLE);
								lv_obj_clean(bgmap);
								score=0;

								return;
							}






            score++;


						lv_obj_t * blood=lv_img_create(bgmap);
						lv_img_set_src(blood,blood_img_lib[fruit_obj[i].img_id]);
						lv_obj_set_pos(blood,fruit_obj[i].x,fruit_obj[i].y);
						lv_img_set_angle(blood,fruit_obj[i].angle);
            lv_img_set_zoom(blood,256+rand()%256);

						lv_anim_t a;
						lv_anim_init(&a);
						lv_anim_set_var(&a,blood);
						lv_anim_set_exec_cb(&a,disappear_cb);
						lv_anim_set_time(&a,2000);
						lv_anim_set_values(&a,255,0);
						lv_anim_set_deleted_cb(&a, split_fruit_deleted_cb);
						lv_anim_start(&a);


						split1=lv_img_create(bgmap);
						lv_img_set_src(split1,split_fruit_img_lib[fruit_obj[i].img_id]);
						lv_obj_set_pos(split1,fruit_obj[i].x,fruit_obj[i].y);

						split2=lv_img_create(bgmap);
						lv_img_set_src(split2,split_fruit_img_lib[fruit_obj[i].img_id]);
						lv_obj_set_pos(split2,fruit_obj[i].x,fruit_obj[i].y);

						lv_anim_t a1;
						lv_anim_init(&a1);
						lv_anim_set_var(&a1,split1);
						lv_anim_set_exec_cb(&a1,split_x_move_cb);
						lv_anim_set_time(&a1,900);
						lv_anim_set_values(&a1,fruit_obj[i].x,fruit_obj[i].x-rand()%400*screen_ratio);
						lv_anim_start(&a1);

						lv_anim_t a2;
						lv_anim_init(&a2);
						lv_anim_set_var(&a2,split1);
						lv_anim_set_exec_cb(&a2,split_y_move_cb);
						lv_anim_set_time(&a2,900);
						lv_anim_set_values(&a2,fruit_obj[i].y,lv_disp_get_ver_res(lv_disp_get_default())+50);
						lv_anim_set_path_cb(&a2, lv_anim_path_ease_in);
						lv_anim_start(&a2);

						lv_anim_t a3;
						lv_anim_init(&a3);
						lv_anim_set_var(&a3,split1);
						lv_anim_set_exec_cb(&a3,split_angle_cb);
						lv_anim_set_time(&a3,1000);
						lv_anim_set_values(&a3,fruit_obj[i].angle,rand()%7200);
						lv_anim_set_deleted_cb(&a3, split_fruit_deleted_cb);
						lv_anim_start(&a3);


						lv_anim_t a4;
						lv_anim_init(&a4);
						lv_anim_set_var(&a4,split2);
						lv_anim_set_exec_cb(&a4,split_x_move_cb);
						lv_anim_set_time(&a4,900);
						lv_anim_set_values(&a4,fruit_obj[i].x,fruit_obj[i].x+rand()%400*screen_ratio);
						lv_anim_start(&a4);

						lv_anim_t a5;
						lv_anim_init(&a5);
						lv_anim_set_var(&a5,split2);
						lv_anim_set_exec_cb(&a5,split_y_move_cb);
						lv_anim_set_time(&a5,900);
						lv_anim_set_values(&a5,fruit_obj[i].y,lv_disp_get_ver_res(lv_disp_get_default())+50);
						lv_anim_set_path_cb(&a5, lv_anim_path_ease_in);
						lv_anim_start(&a5);

						lv_anim_t a6;
						lv_anim_init(&a6);
						lv_anim_set_var(&a6,split2);
						lv_anim_set_exec_cb(&a6,split_angle_cb);
						lv_anim_set_time(&a6,1000);
						lv_anim_set_values(&a6,fruit_obj[i].angle,rand()%7200);
						lv_anim_set_deleted_cb(&a6, split_fruit_deleted_cb);
						lv_anim_start(&a6);


						lv_anim_del(fruit_obj[i].obj,x_move_cb);
						lv_anim_del(fruit_obj[i].obj,y_move_cb);
						lv_anim_del(fruit_obj[i].obj,angle_cb);

						if(lv_obj_is_valid(fruit_obj[i].obj)){lv_obj_del(fruit_obj[i].obj);}
						fruit_obj[i].alive=0;

					}
			}
	}

	point_count++;

		 if(point_count==2)
		 {
				 blade_track=lv_line_create(screen1);
				 lv_obj_set_style_line_width(blade_track,5,0);
				 lv_obj_set_style_line_color(blade_track,lv_color_hex(0xffffff),0);
				 ok=1;
     }

		 if(ok==1){lv_line_set_points(blade_track,blade_point,point_count);}

     if(point_count==15&&ok==1)
     {
			 lv_obj_del(blade_track);point_count=0;ok=0;
			 lv_label_set_text_fmt(score_lable,"SCORE:%d",score);
		 }


}



static void release_screen_cb(lv_event_t * e)
{
	if(ok==1)
		{
			lv_obj_del(blade_track);
		}
		point_count=0;ok=0;
		lv_label_set_text_fmt(score_lable,"SCORE:%d",score);
}



static void x_move_cb(void * var, int32_t v)
{
	lv_obj_t * xxx=(lv_obj_t *)var;
	fruit_obj_type * kkk=(fruit_obj_type *)xxx->user_data;
	kkk->x=v;
	lv_obj_set_x(xxx,v);
}

static void y_move_cb(void * var, int32_t v)
{
	lv_obj_t * xxx=(lv_obj_t *)var;
	fruit_obj_type * kkk=(fruit_obj_type *)xxx->user_data;
	kkk->y=v;
	lv_obj_set_y(xxx,v);
}

static void angle_cb(void * var, int32_t v)
{
	lv_obj_t * xxx=(lv_obj_t *)var;
	fruit_obj_type * kkk=(fruit_obj_type *)xxx->user_data;
	kkk->angle=v;
	lv_img_set_angle(xxx,v);
}



static void fruit_deleted_cb(lv_anim_t * a)
{
	lv_obj_t * xxx=(lv_obj_t *)a->var;
	fruit_obj_type * kkk=(fruit_obj_type *)xxx->user_data;
	kkk->alive=0;
	lv_obj_del_delayed(xxx,100);
}

static void split_angle_cb(void * var, int32_t v)
{
	lv_obj_t * xxx=(lv_obj_t *)var;
	lv_img_set_angle(xxx,v);
}

static void split_fruit_deleted_cb(lv_anim_t * a)
{
	lv_obj_t * xxx=(lv_obj_t *)a->var;
	lv_obj_del(xxx);
}

static void split_x_move_cb(void * var, int32_t v)
{
	lv_obj_t * xxx=(lv_obj_t *)var;
	lv_obj_set_x(xxx,v);
}

static void split_y_move_cb(void * var, int32_t v)
{
	lv_obj_t * xxx=(lv_obj_t *)var;
	lv_obj_set_y(xxx,v);
}


static void disappear_cb(void * var, int32_t v)
{
		lv_obj_t * xxx=(lv_obj_t *)var;
	  lv_obj_set_style_img_opa(xxx,v,0);
}


static void screen_disappear_cb(void * var, int32_t v)
{
		lv_obj_t * xxx=(lv_obj_t *)var;
	  lv_obj_set_style_bg_opa(xxx,v,0);
	  if(v==0){lv_obj_clear_flag(start_btn, LV_OBJ_FLAG_HIDDEN);}
}


static void y_end_cb(lv_anim_t * a)
{
	lv_obj_t * xxx=(lv_obj_t *)a->var;

		lv_anim_t a2;
		lv_anim_init(&a2);
		lv_anim_set_var(&a2,xxx);
		lv_anim_set_exec_cb(&a2,y_move_cb);
		lv_anim_set_time(&a2,900);
		lv_anim_set_values(&a2,lv_disp_get_ver_res(lv_disp_get_default())/4,lv_disp_get_ver_res(lv_disp_get_default())+50);
		lv_anim_set_path_cb(&a2, lv_anim_path_ease_in);
		lv_anim_start(&a2);
}


static void creat_fruit_cb(lv_timer_t * t)
{
	int i,j,k;
	j=rand()%(max_act_fruit-min_act_fruit+1)+min_act_fruit;

	for(k=0;k<j;k++)
	{
			for(i=0;i<max_fruit;i++)
			{

				if(fruit_obj[i].alive==0)
					{
						fruit_obj[i].obj=lv_img_create(screen1);
						fruit_obj[i].x=(300+rand()%200)*screen_ratio;
						fruit_obj[i].y=lv_disp_get_ver_res(lv_disp_get_default())+50;
						fruit_obj[i].alive=1;
						fruit_obj[i].img_id=rand()%5;
						fruit_obj[i].delay=rand()%500;
						fruit_obj[i].obj->user_data=&fruit_obj[i];
						lv_obj_set_pos(fruit_obj[i].obj,fruit_obj[i].x,fruit_obj[i].y);
						lv_img_set_src(fruit_obj[i].obj, fruit_img_lib[fruit_obj[i].img_id]);

						lv_anim_t a1;
						lv_anim_init(&a1);
						lv_anim_set_var(&a1,fruit_obj[i].obj);
						lv_anim_set_exec_cb(&a1,x_move_cb);
						lv_anim_set_time(&a1,1800);
						lv_anim_set_delay(&a1,fruit_obj[i].delay);
						lv_anim_set_values(&a1,fruit_obj[i].x,fruit_obj[i].x+(rand()%400-200)*screen_ratio);
						lv_anim_start(&a1);

						lv_anim_t a2;
						lv_anim_init(&a2);
						lv_anim_set_var(&a2,fruit_obj[i].obj);
						lv_anim_set_exec_cb(&a2,y_move_cb);
						lv_anim_set_time(&a2,900);
						lv_anim_set_delay(&a2,fruit_obj[i].delay);
						lv_anim_set_values(&a2,lv_disp_get_ver_res(lv_disp_get_default())+50,lv_disp_get_ver_res(lv_disp_get_default())/4);
						lv_anim_set_deleted_cb(&a2, y_end_cb);
						lv_anim_set_path_cb(&a2, lv_anim_path_ease_out);
						lv_anim_start(&a2);

						lv_anim_t a3;
						lv_anim_init(&a3);
						lv_anim_set_var(&a3,fruit_obj[i].obj);
						lv_anim_set_exec_cb(&a3,angle_cb);
						lv_anim_set_time(&a3,1800);
						lv_anim_set_delay(&a3,fruit_obj[i].delay);
						lv_anim_set_values(&a3,rand()%1800,rand()%3600);
						lv_anim_set_deleted_cb(&a3, fruit_deleted_cb);
						lv_anim_start(&a3);
						break;
					}
			}
		}
}


static void creat_boom_cb(lv_timer_t * t)
{
	int i,j;


	for(i=0;i<max_fruit;i++)
	{

		if(fruit_obj[i].alive==0)
			{
				fruit_obj[i].obj=lv_img_create(screen1);
				fruit_obj[i].x=(300+rand()%200)*screen_ratio;
				fruit_obj[i].y=lv_disp_get_ver_res(lv_disp_get_default())+50;
				fruit_obj[i].alive=1;
				fruit_obj[i].img_id=5;
				fruit_obj[i].delay=rand()%500;
				fruit_obj[i].obj->user_data=&fruit_obj[i];
				lv_obj_set_pos(fruit_obj[i].obj,fruit_obj[i].x,fruit_obj[i].y);
				lv_img_set_src(fruit_obj[i].obj, fruit_img_lib[fruit_obj[i].img_id]);

				lv_anim_t a1;
				lv_anim_init(&a1);
				lv_anim_set_var(&a1,fruit_obj[i].obj);
				lv_anim_set_exec_cb(&a1,x_move_cb);
				lv_anim_set_time(&a1,1800);
				lv_anim_set_delay(&a1,fruit_obj[i].delay);
				lv_anim_set_values(&a1,fruit_obj[i].x,fruit_obj[i].x+(rand()%400-200)*screen_ratio);
				lv_anim_start(&a1);

				lv_anim_t a2;
				lv_anim_init(&a2);
				lv_anim_set_var(&a2,fruit_obj[i].obj);
				lv_anim_set_exec_cb(&a2,y_move_cb);
				lv_anim_set_time(&a2,900);
				lv_anim_set_delay(&a2,fruit_obj[i].delay);
				lv_anim_set_values(&a2,lv_disp_get_ver_res(lv_disp_get_default())+50,lv_disp_get_ver_res(lv_disp_get_default())/4);
				lv_anim_set_deleted_cb(&a2, y_end_cb);
				lv_anim_set_path_cb(&a2, lv_anim_path_ease_out);
				lv_anim_start(&a2);

				lv_anim_t a3;
				lv_anim_init(&a3);
				lv_anim_set_var(&a3,fruit_obj[i].obj);
				lv_anim_set_exec_cb(&a3,angle_cb);
				lv_anim_set_time(&a3,1800);
				lv_anim_set_delay(&a3,fruit_obj[i].delay);
				lv_anim_set_values(&a3,rand()%1800,rand()%3600);
				lv_anim_set_deleted_cb(&a3, fruit_deleted_cb);
				lv_anim_start(&a3);
				return;
			}
	}
}


static void clear_all_fruit()
{
	for(int i=0;i<max_fruit;i++)
	{
		if(fruit_obj[i].alive==1)
			{
				lv_obj_del(fruit_obj[i].obj);
				fruit_obj[i].alive=0;
			}
	}
}



static void exit_game_cb(lv_event_t * e)
{
	  lv_anim_del_all();
	  lv_obj_del(screen1);
}

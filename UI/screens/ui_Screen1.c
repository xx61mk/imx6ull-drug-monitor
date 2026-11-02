// UI Screen1: 数据信息看板（温度 / 湿度 / 设备号）
// 双屏界面 - 屏幕1（美化版，配色与 squareline_ui_preview.html 一致）
// 数据显示对象: ui_ValueTemp / ui_ValueHum / ui_ValueID（由 ui_data.c 刷新）

#include "../ui.h"

lv_obj_t * ui_Screen1 = NULL;
lv_obj_t * ui_Button1 = NULL;
lv_obj_t * ui_ValueTemp = NULL;
lv_obj_t * ui_ValueHum = NULL;
lv_obj_t * ui_ValueID = NULL;

/* 配色（与 HTML 预览一致） */
#define UI_BG_TOP       lv_color_hex(0x0d1828)
#define UI_BG_BOT       lv_color_hex(0x0a1322)
#define UI_CARD_COLOR   lv_color_hex(0x16233a)
#define UI_BTN_COLOR    lv_color_hex(0x2F6FED)
#define UI_TEXT_MAIN    lv_color_hex(0xFFFFFF)
#define UI_TEXT_SUB     lv_color_hex(0x8A94A8)
#define UI_ACCENT_TEMP  lv_color_hex(0xFF8A4C)   /* 温度-橙 */
#define UI_ACCENT_HUM   lv_color_hex(0x3FC4FF)   /* 湿度-蓝 */
#define UI_ACCENT_ID    lv_color_hex(0x46E3A0)   /* 设备-绿 */

/* 内部：创建一张数据卡片，返回数值标签（供 ui_data.c 刷新） */
static lv_obj_t * create_data_card(lv_obj_t * parent, lv_coord_t x, const char * title,
                                   const char * unit, lv_color_t accent)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_pos(card, x, 96);
    lv_obj_set_size(card, 232, 300);
    lv_obj_set_style_bg_color(card, UI_CARD_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(card, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(card, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(card, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(card, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(card, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(card, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(card, 70, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(card, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(card, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    /* 卡片标题 */
    lv_obj_t * t = lv_label_create(card);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_font(t, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(t, UI_TEXT_SUB, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 28);

    /* 大号数值（由 ui_data.c 刷新） */
    lv_obj_t * v = lv_label_create(card);
    lv_label_set_text(v, "--");
    lv_obj_set_style_text_font(v, &lv_font_montserrat_36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(v, accent, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(v, LV_ALIGN_CENTER, 0, -18);

    /* 单位 */
    lv_obj_t * u = lv_label_create(card);
    lv_label_set_text(u, unit);
    lv_obj_set_style_text_font(u, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(u, UI_TEXT_SUB, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(u, LV_ALIGN_CENTER, 0, 34);

    /* 底部强调色条 */
    lv_obj_t * bar = lv_obj_create(card);
    lv_obj_set_size(bar, 56, 6);
    lv_obj_set_style_bg_color(bar, accent, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(bar, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -26);

    return v;
}

/* 内部：标题栏 */
static void create_title_bar(lv_obj_t * parent, const char * title)
{
    lv_obj_t * bar = lv_obj_create(parent);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_size(bar, 800, 60);
    lv_obj_set_style_bg_color(bar, UI_CARD_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * t = lv_label_create(bar);
    lv_label_set_text(t, title);
    lv_obj_set_style_text_font(t, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(t, UI_TEXT_MAIN, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(t, LV_ALIGN_LEFT_MID, 24, 0);
}

/* 事件：跳转到 Screen2（实时曲线） */
void ui_event_Button1(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, &ui_Screen2_screen_init);
    }
}

// build funtions

void ui_Screen1_screen_init(void)
{
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);
    /* 背景：竖直渐变 */
    lv_obj_set_style_bg_color(ui_Screen1, UI_BG_TOP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Screen1, UI_BG_BOT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Screen1, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen1, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 标题栏：环境监测系统 + 在线状态 */
    create_title_bar(ui_Screen1, "环境监测系统");

    lv_obj_t * dot = lv_obj_create(ui_Screen1);
    lv_obj_set_size(dot, 10, 10);
    lv_obj_set_style_bg_color(dot, UI_ACCENT_ID, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(dot, LV_ALIGN_TOP_RIGHT, -80, 25);

    lv_obj_t * st = lv_label_create(ui_Screen1);
    lv_label_set_text(st, "在线");
    lv_obj_set_style_text_font(st, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(st, UI_ACCENT_ID, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(st, LV_ALIGN_TOP_RIGHT, -32, 19);

    /* 三张数据卡片 */
    ui_ValueTemp = create_data_card(ui_Screen1, 32,  "温度", "°C",  UI_ACCENT_TEMP);
    ui_ValueHum  = create_data_card(ui_Screen1, 284, "湿度", "%RH", UI_ACCENT_HUM);
    ui_ValueID   = create_data_card(ui_Screen1, 536, "设备号", "ID", UI_ACCENT_ID);

    /* 底部提示 */
    lv_obj_t * hint = lv_label_create(ui_Screen1);
    lv_label_set_text(hint, "数据每3秒更新一次");
    lv_obj_set_style_text_font(hint, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(hint, UI_TEXT_SUB, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 32, -22);

    /* 切换按钮：实时曲线 */
    ui_Button1 = lv_btn_create(ui_Screen1);
    lv_obj_set_size(ui_Button1, 176, 56);
    lv_obj_align(ui_Button1, LV_ALIGN_BOTTOM_RIGHT, -32, -16);
    lv_obj_set_style_bg_color(ui_Button1, UI_BTN_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_Button1, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Button1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_Button1, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * bl = lv_label_create(ui_Button1);
    lv_label_set_text(bl, "实时曲线 >");
    lv_obj_set_style_text_font(bl, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bl, UI_TEXT_MAIN, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(bl);

    lv_obj_add_event_cb(ui_Button1, ui_event_Button1, LV_EVENT_ALL, NULL);
}

void ui_Screen1_screen_destroy(void)
{
    if(ui_Screen1) lv_obj_del(ui_Screen1);

    // NULL screen variables
    ui_Screen1 = NULL;
    ui_Button1 = NULL;
    ui_ValueTemp = NULL;
    ui_ValueHum = NULL;
    ui_ValueID = NULL;
}

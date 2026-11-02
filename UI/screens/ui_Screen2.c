// UI Screen2: 实时曲线（温度 / 湿度折线图）
// 双屏界面 - 屏幕2（美化版，双 Y 轴：温度0-60 / 湿度0-100）
// 数据入口: ui_Chart / ui_ChartTempSer / ui_ChartHumSer（由 ui_data.c 推送新点）

#include "../ui.h"

lv_obj_t * ui_Screen2 = NULL;
lv_obj_t * ui_Button2 = NULL;
lv_obj_t * ui_Chart = NULL;
lv_chart_series_t * ui_ChartTempSer = NULL;
lv_chart_series_t * ui_ChartHumSer = NULL;

#define CHART_POINTS 40    /* 曲线缓存点数：40 点 × 3 秒 ≈ 2 分钟历史 */

/* 配色（与 Screen1 / HTML 预览保持一致） */
#define UI_BG_TOP       lv_color_hex(0x0d1828)
#define UI_BG_BOT       lv_color_hex(0x0a1322)
#define UI_CARD_COLOR   lv_color_hex(0x16233a)
#define UI_BTN_COLOR    lv_color_hex(0x2F6FED)
#define UI_TEXT_MAIN    lv_color_hex(0xFFFFFF)
#define UI_TEXT_SUB     lv_color_hex(0x8A94A8)
#define UI_ACCENT_TEMP  lv_color_hex(0xFF8A4C)
#define UI_ACCENT_HUM   lv_color_hex(0x3FC4FF)

/* 事件：返回 Screen1（数据信息） */
void ui_event_Button2(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_CLICKED) {
        _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, &ui_Screen1_screen_init);
    }
}

// build funtions

void ui_Screen2_screen_init(void)
{
    ui_Screen2 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen2, LV_OBJ_FLAG_SCROLLABLE);
    /* 背景：竖直渐变 */
    lv_obj_set_style_bg_color(ui_Screen2, UI_BG_TOP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(ui_Screen2, UI_BG_BOT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui_Screen2, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen2, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 标题栏：实时曲线 */
    lv_obj_t * bar = lv_obj_create(ui_Screen2);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_size(bar, 800, 60);
    lv_obj_set_style_bg_color(bar, UI_CARD_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * t = lv_label_create(bar);
    lv_label_set_text(t, "实时曲线");
    lv_obj_set_style_text_font(t, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(t, UI_TEXT_MAIN, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(t, LV_ALIGN_LEFT_MID, 24, 0);

    /* 图例：温度 / 湿度（色点 + 文字） */
    lv_obj_t * dot_t = lv_obj_create(bar);
    lv_obj_set_size(dot_t, 10, 10);
    lv_obj_set_style_bg_color(dot_t, UI_ACCENT_TEMP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(dot_t, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(dot_t, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(dot_t, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(dot_t, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(dot_t, LV_ALIGN_TOP_RIGHT, -132, 25);

    lv_obj_t * lt = lv_label_create(bar);
    lv_label_set_text(lt, "温度");
    lv_obj_set_style_text_font(lt, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lt, UI_ACCENT_TEMP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(lt, LV_ALIGN_TOP_RIGHT, -84, 19);

    lv_obj_t * dot_h = lv_obj_create(bar);
    lv_obj_set_size(dot_h, 10, 10);
    lv_obj_set_style_bg_color(dot_h, UI_ACCENT_HUM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(dot_h, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(dot_h, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(dot_h, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(dot_h, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(dot_h, LV_ALIGN_TOP_RIGHT, -72, 25);

    lv_obj_t * lh = lv_label_create(bar);
    lv_label_set_text(lh, "湿度");
    lv_obj_set_style_text_font(lh, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lh, UI_ACCENT_HUM, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(lh, LV_ALIGN_TOP_RIGHT, -24, 19);

    /* 图表面板 */
    lv_obj_t * panel = lv_obj_create(ui_Screen2);
    lv_obj_set_pos(panel, 32, 84);
    lv_obj_set_size(panel, 736, 316);
    lv_obj_set_style_bg_color(panel, UI_CARD_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(panel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(panel, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(panel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(panel, 70, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(panel, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(panel, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    /* 折线图（双 Y 轴：温度 0-60 左轴，湿度 0-100 右轴） */
    ui_Chart = lv_chart_create(panel);
    lv_obj_set_pos(ui_Chart, 16, 12);
    lv_obj_set_size(ui_Chart, 704, 292);
    lv_chart_set_type(ui_Chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(ui_Chart, CHART_POINTS);
    lv_chart_set_range(ui_Chart, LV_CHART_AXIS_PRIMARY_Y,   0, 60);
    lv_chart_set_range(ui_Chart, LV_CHART_AXIS_SECONDARY_Y, 0, 100);
    lv_chart_set_update_mode(ui_Chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_div_line_count(ui_Chart, 5, 8);

    /* 坐标轴刻度（左轴温度 0~60，右轴湿度 0~100，显示标签） */
    lv_chart_set_axis_tick(ui_Chart, LV_CHART_AXIS_PRIMARY_Y,   10, 5, 6, 2, true, 44);
    lv_chart_set_axis_tick(ui_Chart, LV_CHART_AXIS_SECONDARY_Y, 10, 5, 5, 2, true, 44);
    lv_chart_set_axis_tick(ui_Chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, 0, 0, false, 0);

    /* 曲线：温度（橙，左轴）/ 湿度（蓝，右轴） */
    ui_ChartTempSer = lv_chart_add_series(ui_Chart, UI_ACCENT_TEMP, LV_CHART_AXIS_PRIMARY_Y);
    ui_ChartHumSer  = lv_chart_add_series(ui_Chart, UI_ACCENT_HUM,  LV_CHART_AXIS_SECONDARY_Y);

    /* 样式：线宽 3、隐藏数据点圆点、刻度文字灰色 */
    lv_obj_set_style_line_width(ui_Chart, 3, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_size(ui_Chart, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Chart, &lv_font_montserrat_14, LV_PART_TICKS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_Chart, UI_TEXT_SUB, LV_PART_TICKS | LV_STATE_DEFAULT);

    /* 初始点全部置空（等数据到来再画） */
    lv_chart_set_all_value(ui_Chart, ui_ChartTempSer, LV_CHART_POINT_NONE);
    lv_chart_set_all_value(ui_Chart, ui_ChartHumSer, LV_CHART_POINT_NONE);

    /* 底部提示 */
    lv_obj_t * hint = lv_label_create(ui_Screen2);
    lv_label_set_text(hint, "每3秒采集一次");
    lv_obj_set_style_text_font(hint, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(hint, UI_TEXT_SUB, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 32, -22);

    /* 返回按钮 */
    ui_Button2 = lv_btn_create(ui_Screen2);
    lv_obj_set_size(ui_Button2, 160, 56);
    lv_obj_align(ui_Button2, LV_ALIGN_BOTTOM_RIGHT, -32, -16);
    lv_obj_set_style_bg_color(ui_Button2, UI_BTN_COLOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui_Button2, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui_Button2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(ui_Button2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * bl = lv_label_create(ui_Button2);
    lv_label_set_text(bl, "< 返回数据");
    lv_obj_set_style_text_font(bl, &ui_font_UICN20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(bl, UI_TEXT_MAIN, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(bl);

    lv_obj_add_event_cb(ui_Button2, ui_event_Button2, LV_EVENT_ALL, NULL);
}

void ui_Screen2_screen_destroy(void)
{
    if(ui_Screen2) lv_obj_del(ui_Screen2);

    // NULL screen variables
    ui_Screen2 = NULL;
    ui_Button2 = NULL;
    ui_Chart = NULL;
    ui_ChartTempSer = NULL;
    ui_ChartHumSer = NULL;
}

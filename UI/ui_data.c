// UI 数据刷新接口：把邮箱推送的最新数据刷到 UI（必须在 LVGL 主线程调用）
// 由 main.c 在收到新数据时调用；同时更新屏幕1的数值标签和屏幕2的曲线。

#include "ui.h"

void ui_update_data(float temp, float hum, int id)
{
    /* 屏幕1：数值标签 */
    if (ui_ValueTemp) lv_label_set_text_fmt(ui_ValueTemp, "%.1f", temp);
    if (ui_ValueHum)  lv_label_set_text_fmt(ui_ValueHum, "%.1f", hum);
    if (ui_ValueID)   lv_label_set_text_fmt(ui_ValueID, "DEV-%04d", id);

    /* 屏幕2：折线图追加新点（SHIFT 模式下自动滚动）
     * 温度走左轴(0-60)，湿度走右轴(0-100)，裁剪到各自量程。 */
    if (ui_Chart && ui_ChartTempSer && ui_ChartHumSer) {
        lv_coord_t t = (lv_coord_t)(temp + 0.5f);
        lv_coord_t h = (lv_coord_t)(hum + 0.5f);
        if (t < 0) t = 0;
        if (t > 60) t = 60;
        if (h < 0) h = 0;
        if (h > 100) h = 100;
        lv_chart_set_next_value(ui_Chart, ui_ChartTempSer, t);
        lv_chart_set_next_value(ui_Chart, ui_ChartHumSer, h);
    }
}

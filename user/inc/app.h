/**
  ******************************************************************************
  * @file    app.h
  * @brief   应用层:状态机 + 主逻辑。main 只调用 app_init() 和 app_run()
  ******************************************************************************
  */
#ifndef __APP_H
#define __APP_H

#ifdef __cplusplus
extern "C" {
#endif

void app_init(void);
void app_run(void);   /* 放在主循环 while(1) 里周期调用 */

#ifdef __cplusplus
}
#endif

#endif /* __APP_H */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "link.h"
#include "../include/mailbox.h"
#include "../include/linkqueue.h"
#include "../include/mqtt.h"

int system_run_flag = 1;
void *get_data_th(void *arg)
{
  struct msgdata mydata;
  int cnt = 0;
  time_t sec;
  struct tm *ptm = NULL;

  while (system_run_flag)
  {
    mydata.id = 1;
    mydata.cnt = cnt++;
    mydata.temp = rand() % 60;
    mydata.hum = rand() % 100;
    time(&sec);
    ptm = localtime(&sec);
    mydata.tim = *ptm;

    send_mail_msg("get_data", mydata, "show_data");
    send_mail_msg("get_data", mydata, "save_data");
    send_mail_msg("get_data", mydata, "send_data");

    sleep(3);
  }

  return NULL;
}

void *show_data_th(void *arg)
{
  struct msgdata mydata;
  char send_name[32] = {0};
  int ret = 0;
  while (system_run_flag)
  {
    ret = recv_mail_msg(&mydata, send_name);
    if (0 != ret)
    {
      continue;
    }
    printf("%s->show: id = %d, cnt = %d, temp = %f,....\n", send_name, mydata.id, mydata.cnt, mydata.temp);
  }

  return NULL;
}

void *save_data_th(void *arg)
{
  struct msgdata mydata;
  char send_name[32] = {0};
  int ret = 0;
  while (system_run_flag)
  {
    ret = recv_mail_msg(&mydata, send_name);
    if (0 != ret)
    {
      continue;
    }
    printf("%s->save: id = %d, cnt = %d, temp = %f,....\n", send_name, mydata.id, mydata.cnt, mydata.temp);
  }

  return NULL;
}

void *send_data_th(void *arg)
{
  struct msgdata mydata;
  char send_name[32] = {0};
  int ret = 0;
  mqtt_init();
  while (system_run_flag)
  {
    ret = recv_mail_msg(&mydata, send_name);
    if (0 != ret)
    {
      continue;
    }
    printf("%s->send: id = %d, cnt = %d, temp = %f,....\n", send_name, mydata.id, mydata.cnt, mydata.temp);
    mqtt_send("id", mydata.cnt);
    mqtt_send("temp", mydata.temp);
    mqtt_send("hum", mydata.hum);
  }

  return NULL;
}
int main(void)
{
  Link_t *pmail = NULL;

  pmail = create_mailbox_system();
  if (NULL == pmail)
  {
    return -1;
  }

  register_thread_mailbox(pmail, "get_data", get_data_th);
  register_thread_mailbox(pmail, "show_data", show_data_th);
  register_thread_mailbox(pmail, "save_data", save_data_th);
  register_thread_mailbox(pmail, "send_data", send_data_th);

  destroy_mailbox(pmail);

  return 0;
}
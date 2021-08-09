/* 
 * Copyright (c) 2021, Justin Oh <0x4f48@gmail.com>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef __USBRELAY_H__
#define __USBRELAY_H__

#define NORMALLY_OPEN           0
#define NORMALLY_CLOSED         1

#define RELAY_OFF				0
#define RELAY_ON				1

#define MSG_QUERY_CIRCUIT_CONF  "@CC?"
#define MSG_QUERY_CIRCUIT_LEN   4

#define MSG_CC_N_OPEN_PRG		"@CC0"
#define MSG_CC_N_CLOSE_PRG		"@CC1"
#define MSG_CC_PRG_LEN			4

#define MSG_CC_N_OPEN           "#CC0"
#define MSG_CC_N_CLOSE          "#CC1"
#define MSG_CC_RESP_LEN         4

#define MSG_QUERY_ON_OFF        "@SW?"
#define MSG_QUERY_ON_OFF_LEN    4
#define MSG_RESP_OFF			"#SW0"
#define MSG_RESP_ON				"#SW1"
#define MSG_RESP_ON_OFF_LEN		4

#define MSG_ON                  "@SW1"
#define MSG_ON_LEN              4
#define MSG_OFF                 "@SW0"
#define MSG_OFF_LEN             4

#define MSG_QUERY_DEV_NAME		"@DN?"
#define MSG_QUERY_DEV_NAME_LEN	4
#define MSG_DEV_NAME_RESP		"#DN!"
#define MSG_DEV_NAME_RESP_LEN	4
#define MSG_DEV_NAME_PRG_CMD	"@DN!"

#define SERIAL_OPEN_DEALY		2


enum
{
   COL_0 = 0,
   COL_1,
   COL_2,
   COL_3,
   N_COLS,
};

typedef struct 
{
	int relay_on_off;
    int circuit_conf;
    gboolean connected;
    char dev_name[32];
}STATUS_DATA;

typedef struct
{
    GtkWidget       *main_window;
    GtkWidget       *dev_combobox;
    GtkTreeStore    *dev_store;
    GtkWidget       *lbl_status;
    GtkWidget       *btn_scan;
    GtkWidget		*lbl_device_name;
    GtkWidget		*lbl_circuit_conf;
    GtkWidget       *btn_on;
    GtkWidget       *btn_off;
    GtkWidget       *btn_pulse;
    GtkWidget       *btn_spin_delay;
    GtkWidget       *chbox_always_top;
    GtkImage        *img_banner;

	/* widgets related to settings */
	GtkWidget		*btn_settings;
    GtkWidget       *dlg_settings;
    GtkWidget       *btn_nopen;
    GtkWidget       *btn_nclose;
    GtkWidget       *tentry_devname;
        
    /* status info */
    STATUS_DATA     status;
}USBRELAY_OBJS;

#endif

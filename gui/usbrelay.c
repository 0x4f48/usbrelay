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
 
#include <gtk/gtk.h>
#include <unistd.h>

#include "common.h"
#include "usbrelay-gui.h"
#include "serial.h"
#include "usbrelay.h"
#include "util.h"


void show_message_box(gchar *first_msg, gchar* second_msg)
{
	GtkWidget* msg_dlg = gtk_message_dialog_new(
		NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
		"%s", first_msg);
	
	gtk_message_dialog_format_secondary_text(
		GTK_MESSAGE_DIALOG(msg_dlg),
		"%s", second_msg);
				
	int response = gtk_dialog_run(GTK_DIALOG(msg_dlg));

	debug_msg("response was %d (OK=%d, DELETE_EVENT=%d)\n",
		   response, GTK_RESPONSE_OK, GTK_RESPONSE_DELETE_EVENT);
		   
	gtk_widget_destroy(msg_dlg);
}

void enable_relay_ctrl_buttons(USBRELAY_OBJS *win_objs, gboolean enable)
{
    gtk_widget_set_sensitive(win_objs->btn_on, enable);
    gtk_widget_set_sensitive(win_objs->btn_off, enable);
    gtk_widget_set_sensitive(win_objs->btn_pulse, enable);
    gtk_widget_set_sensitive(win_objs->btn_settings, enable);
}

void update_relay_on_off_label(USBRELAY_OBJS *win_objs)
{
	gchar *green_on_markup = "<span foreground='green'>ON</span>";

	if ( win_objs->status.circuit_conf == NORMALLY_OPEN )
	{
		if ( win_objs->status.relay_on_off == RELAY_ON )
		{
			gtk_label_set_markup(GTK_LABEL(win_objs->lbl_status), green_on_markup);
		}
		else
		{
			gtk_label_set_text(GTK_LABEL(win_objs->lbl_status), "OFF");
		}
	}
	else
	{
		if ( win_objs->status.relay_on_off == RELAY_ON )
		{
			gtk_label_set_text(GTK_LABEL(win_objs->lbl_status), "OFF");
		}
		else
		{
			gtk_label_set_markup(GTK_LABEL(win_objs->lbl_status), green_on_markup);
		}
	}
}

void update_dev_name(USBRELAY_OBJS *win_objs)
{
	gtk_label_set_text(GTK_LABEL(win_objs->lbl_device_name), win_objs->status.dev_name);
}

gboolean pulse_time_handler(GtkWidget *widget)
{
    USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)widget;
    
    serial_write(MSG_OFF, MSG_OFF_LEN);
    
    /* enable the button back */
    enable_relay_ctrl_buttons(win_objs, TRUE);

	/* relay off */
	serial_write(MSG_OFF, MSG_OFF_LEN);
	win_objs->status.relay_on_off = RELAY_OFF;
	
	/* update leabel*/
	update_relay_on_off_label(win_objs);
	
	/* cancel timer */
	return FALSE;
}



void swap_on_off_btn_label(USBRELAY_OBJS *win_objs, gboolean swap)
{
	if ( swap )
	{
		gtk_button_set_label(GTK_BUTTON(win_objs->btn_on), "Off");
		gtk_button_set_label(GTK_BUTTON(win_objs->btn_off), "On");
	}
	else
	{
		gtk_button_set_label(GTK_BUTTON(win_objs->btn_on), "On");
		gtk_button_set_label(GTK_BUTTON(win_objs->btn_off), "Off");

	}
}

void query_dev_status(USBRELAY_OBJS *win_objs)
{
    char rx_buf[32];
    char msg_buf[128];
    int ret = 0;
    
    memset(rx_buf, 0x00, sizeof(rx_buf));
    
	debug_msg("Query CC\n");
    
    // send circuit config query
    ret = serial_write(MSG_QUERY_CIRCUIT_CONF, MSG_QUERY_CIRCUIT_LEN);
    if ( ret <= 0 )
    {
		sprintf(msg_buf, "(Code: %d)", ret );
		show_message_box("Fail to query cicuit config!", msg_buf);
	}

    if ( (ret = serial_read( rx_buf, sizeof(rx_buf))) > 0 )
    {
        if ( !memcmp( rx_buf, MSG_CC_N_OPEN, MSG_CC_RESP_LEN) )
        {
            /* enable normally open */
            debug_msg("CC: N-Open\n");
            win_objs->status.circuit_conf = NORMALLY_OPEN;

            swap_on_off_btn_label(win_objs, FALSE);
            gtk_label_set_text(GTK_LABEL(win_objs->lbl_circuit_conf), "Normally Open");
            
        }
        else if(!memcmp( rx_buf, MSG_CC_N_CLOSE, MSG_CC_RESP_LEN))
        {
            /* enable normally close */
            debug_msg("CC: N-Close\n");
            win_objs->status.circuit_conf = NORMALLY_CLOSED;
            swap_on_off_btn_label(win_objs, TRUE);
            gtk_label_set_text(GTK_LABEL(win_objs->lbl_circuit_conf), "Normally Close");
        }
        else
        {
			debug_msg("fial to get CC\n");
			sprintf(msg_buf, "(Code: %d)", ret );
			show_message_box("Fail to read circuit config!", msg_buf);
		}
    }

	debug_msg("Query relay status\n");
	/* query relay on/off status */
	ret = serial_write(MSG_QUERY_ON_OFF, MSG_QUERY_ON_OFF_LEN);
    if ( ret <= 0 )
    {
		sprintf(msg_buf, "(Code: %d)", ret );
		show_message_box("Fail to write on/off query!", msg_buf);
	}
	    
    memset( rx_buf, 0x00, sizeof(rx_buf));
    if ( (ret = serial_read( rx_buf, sizeof(rx_buf))) > 0 )
    {
		debug_msg("relay status: %s\n", rx_buf);
		if ( !memcmp(rx_buf, MSG_RESP_ON, MSG_RESP_ON_OFF_LEN) )
		{
			win_objs->status.relay_on_off = RELAY_ON;
		}
		else if(!memcmp(rx_buf, MSG_RESP_OFF, MSG_RESP_ON_OFF_LEN))
		{
			win_objs->status.relay_on_off = RELAY_OFF;
		}
		else
		{
			sprintf(msg_buf, "(Code: %d)", ret );
			show_message_box("Fail read status", msg_buf);
		}
	}
	
	debug_msg("Query device name\n");

	/* query device name */
    ret = serial_write(MSG_QUERY_DEV_NAME, MSG_QUERY_DEV_NAME_LEN);
    if ( ret <= 0 )
    {
		sprintf(msg_buf, "(Code: %d)", ret );
		show_message_box("Fail to write device name query!", msg_buf);
	}

    memset( rx_buf, 0x00, sizeof(rx_buf));
    if ( (ret = serial_read( rx_buf, sizeof(rx_buf))) > 0 )
    {
		debug_msg("query dev name: %s\n", rx_buf); 
        if ( !memcmp( rx_buf, MSG_DEV_NAME_RESP, MSG_DEV_NAME_RESP_LEN) )
        {
			strncpy(win_objs->status.dev_name, &rx_buf[4], sizeof(win_objs->status.dev_name));
		}
		else
		{
			sprintf(msg_buf, "(Code: %d)", ret );
			show_message_box("Fail read device name", msg_buf);
		}
	}
}

gboolean connect_ok_time_handler(GtkWidget *widget)
{
	USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)widget;
	
	query_dev_status(win_objs);
    update_relay_on_off_label(win_objs);
    update_dev_name(win_objs);
                    
	/* enable buttons */
	enable_relay_ctrl_buttons(win_objs, TRUE);
	
	return FALSE;
}

void scan_device_node(USBRELAY_OBJS *win_objs)
{
    int i = 0;
    int cnt = 0;
    char dev_path[128];
    GtkTreeIter iter;
        
    gtk_tree_store_clear((GtkTreeStore*)win_objs->dev_store);
    
    for( i = 0; i <32; i++)
    {
        snprintf(dev_path, sizeof(dev_path), "/dev/ttyUSB%d", i);
        if( access( dev_path, F_OK ) == 0 )
        {
            // add to combo box
            gtk_tree_store_append (
                win_objs->dev_store,
                &iter,
                NULL);

            gtk_tree_store_set (
                win_objs->dev_store, &iter, 
                COL_0, dev_path,
                -1 );
            cnt++;
        }
        else
        {
            break;
        }
    }
    
    if ( cnt > 0 )
        gtk_combo_box_set_active( (GtkComboBox*)win_objs->dev_combobox , 0 );
}

//=====================================================================

// called when window is closed
void on_window_main_destroy(GtkWidget *widget, gpointer user_data)
{
	USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)user_data;

	if ( win_objs->status.connected == TRUE )
	{
		serial_close();
	}

    gtk_main_quit();
}

void on_btn_on_clicked(GtkButton *button, gpointer user_data)
{
    USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)user_data;

	serial_write(MSG_ON, MSG_ON_LEN);
	win_objs->status.relay_on_off = RELAY_ON;
	update_relay_on_off_label(win_objs);
}

void on_btn_off_clicked(GtkButton *button, gpointer user_data)
{
    USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)user_data;
    
    serial_write(MSG_OFF, MSG_OFF_LEN);
    win_objs->status.relay_on_off = RELAY_OFF;
	update_relay_on_off_label(win_objs);
}


void on_btn_pulse_clicked(GtkButton *button, gpointer user_data)
{
    USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)user_data;

    int delay = (int)gtk_spin_button_get_value((GtkSpinButton*)win_objs->btn_spin_delay);

    
    /* disable pulse button */
    enable_relay_ctrl_buttons(win_objs, FALSE);
    
    /* relay on */
    serial_write(MSG_ON, MSG_ON_LEN);
    win_objs->status.relay_on_off = RELAY_ON;
    
	/* update leabel*/
	update_relay_on_off_label(win_objs);
    
    g_timeout_add((delay*1000), (GSourceFunc)pulse_time_handler, (gpointer) user_data);
}

void on_chbox_alwyas_top_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)user_data;
    GtkWindow *main_window = (GtkWindow*)win_objs->main_window;
    
    gboolean is_toggled;
     
    is_toggled = gtk_toggle_button_get_active(togglebutton);
    
    if ( is_toggled )
    {
        gtk_window_set_keep_above( main_window, TRUE );
    }
    else
    {
        gtk_window_set_keep_above( main_window, FALSE );
    }
}


void on_btn_scan_dev_clicked(GtkWidget *btn_scan, gpointer user_data)
{

    USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)user_data;
    
    if ( win_objs->status.connected  == FALSE )
    {
        scan_device_node(win_objs);
    }
}

void on_btn_settings_clicked(GtkWidget *btn_scan, gpointer user_data)
{
    USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)user_data;
    int ret = 0;
    
	if ( win_objs->status.connected )
	{
		/* apply current status */
		if ( win_objs->status.circuit_conf == NORMALLY_OPEN )
		{
			debug_msg("Crr CC: N-Open\n");
			gtk_toggle_button_set_active((GtkToggleButton*)win_objs->btn_nopen, TRUE);
		}
		else
		{
			debug_msg("Crr CC: N-Close\n");
			gtk_toggle_button_set_active((GtkToggleButton*)win_objs->btn_nclose, TRUE);
		}
		
		/* set device name */
		gtk_entry_set_text(GTK_ENTRY(win_objs->tentry_devname), win_objs->status.dev_name );
		
		/* start settings dialog */
		ret = gtk_dialog_run((GtkDialog*)win_objs->dlg_settings);
		
		switch(ret)
		{
			case GTK_RESPONSE_OK:
			{
				/* read current user selection and update */
				gboolean user_n_open = gtk_toggle_button_get_active((GtkToggleButton*)win_objs->btn_nopen);
				const gchar *user_dev_name; 
				if ( user_n_open )
				{
					/* user selected normally open */
					if ( win_objs->status.circuit_conf == NORMALLY_CLOSED )
					{
						/* update to normal open */
						serial_write( MSG_CC_N_OPEN_PRG, MSG_CC_PRG_LEN );

						/* update main window's ui too */
						win_objs->status.circuit_conf = NORMALLY_OPEN;
						debug_msg("New CC: N-Open\n");
					}
				}
				else
				{
					/* user selected normally close */
					if ( win_objs->status.circuit_conf == NORMALLY_OPEN )
					{
						/* update to normally close */
						serial_write( MSG_CC_N_CLOSE_PRG, MSG_CC_PRG_LEN );

						/* update main window's ui too */
						win_objs->status.circuit_conf = NORMALLY_CLOSED;
						debug_msg("New CC: N-Close\n");
					}
				}
				
				/* device name change? */
				user_dev_name = gtk_entry_get_text(GTK_ENTRY(win_objs->tentry_devname)); 
				if ( strcmp(user_dev_name, win_objs->status.dev_name) )
				{
					char prg_cmd[32];
					
					/* update device name */
					debug_msg("update dev name: %s\n", user_dev_name);
					memset( prg_cmd, 0x00, sizeof(prg_cmd));
					sprintf(prg_cmd, "%s%s", MSG_DEV_NAME_PRG_CMD, user_dev_name ); 
					serial_write(prg_cmd, (strlen(prg_cmd)+1));
				}
			}
			break;
			default:
				break;
		} // switch
		gtk_widget_hide((GtkWidget*)win_objs->dlg_settings);
		
		/* update ui */
		query_dev_status(win_objs);
		update_relay_on_off_label(win_objs);
		update_dev_name(win_objs);
		
	} // connected
}


void on_btn_connect_clicked(GtkWidget *btn_connect, gpointer user_data)
{
    USBRELAY_OBJS *win_objs = (USBRELAY_OBJS*)user_data;
    GtkTreeIter iter;
    char* curr_dev = NULL;

    if ( win_objs->status.connected )
    {
        /* do disconnect */
        serial_close();

        /* set flag and update button */
        win_objs->status.connected = FALSE;
        gtk_button_set_label(GTK_BUTTON(btn_connect), "Connect");
        
        /* enable scan btn */
        gtk_widget_set_sensitive(win_objs->btn_scan, TRUE);
        gtk_widget_set_sensitive(win_objs->dev_combobox, TRUE);
        enable_relay_ctrl_buttons(win_objs, FALSE);
        
        gtk_label_set_text(GTK_LABEL(win_objs->lbl_status), "?");
        
    }
    else
    {
        if( gtk_combo_box_get_active_iter((GtkComboBox*)win_objs->dev_combobox, &iter) )
        {
            // get active item in combobox
            gtk_tree_model_get ( (GtkTreeModel*)win_objs->dev_store, 
                                &iter, 
                                COL_0, &curr_dev,
                                -1 );
            
            // connect to this device
            if ( curr_dev != NULL )
            {
				debug_msg("try connect: %s\n", curr_dev);
				
                if ( serial_open(curr_dev) != 0 )
                {
					// TODO:
					show_message_box("Fail to connect!", NULL);
                }
                else
                {
					debug_msg("connected!\n");
                    /* update connection status*/
                    win_objs->status.connected = TRUE;
                    
                    /* change button label */
                    gtk_button_set_label(GTK_BUTTON(btn_connect), "Disconnect");
                    
                    /* disable scan and dev combo box */
                    gtk_widget_set_sensitive(win_objs->btn_scan, FALSE);
                    gtk_widget_set_sensitive(win_objs->dev_combobox, FALSE);
                    
                    //sleep(SERIAL_OPEN_DEALY);
                    gtk_label_set_text(GTK_LABEL(win_objs->lbl_status),"Connecting...");
                    g_timeout_add((SERIAL_OPEN_DEALY*1000), (GSourceFunc)connect_ok_time_handler, (gpointer)win_objs);

                }
            }                   
        }
    }
}

int main(int argc, char *argv[])
{
    GtkBuilder      *builder;
    USBRELAY_OBJS   win_objs; 


    gtk_init(&argc, &argv);
    
    builder = gtk_builder_new ();

    gtk_builder_add_from_string (
        builder,
        (gchar*)__usb_relay_gui_glade,
        sizeof(__usb_relay_gui_glade),
        NULL );
    
    win_objs.main_window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    win_objs.dev_combobox = GTK_WIDGET (gtk_builder_get_object( builder, "cbox_devices" ));
    win_objs.dev_store = (GtkTreeStore* )(gtk_builder_get_object( builder, "dev_treestore" ));
    win_objs.btn_scan = GTK_WIDGET(gtk_builder_get_object( builder, "btn_scan_dev" ));
    win_objs.lbl_status = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_status"));
    win_objs.lbl_circuit_conf = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_circuit_config"));
    win_objs.lbl_device_name = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_main_device_name"));
    win_objs.tentry_devname = GTK_WIDGET(gtk_builder_get_object(builder, "tentry_devname"));
    win_objs.btn_settings = GTK_WIDGET(gtk_builder_get_object(builder, "btn_settings"));
    win_objs.btn_on = GTK_WIDGET(gtk_builder_get_object(builder, "btn_on"));
    win_objs.btn_off = GTK_WIDGET(gtk_builder_get_object(builder, "btn_off"));
    win_objs.btn_pulse = GTK_WIDGET(gtk_builder_get_object(builder, "btn_pulse"));
    win_objs.btn_spin_delay = GTK_WIDGET(gtk_builder_get_object(builder, "btn_spin_delay"));
    win_objs.chbox_always_top = GTK_WIDGET(gtk_builder_get_object(builder, "chbox_alwyas_top"));

	win_objs.dlg_settings = GTK_WIDGET(gtk_builder_get_object(builder, "dlg_settings"));
    win_objs.btn_nopen = GTK_WIDGET(gtk_builder_get_object(builder, "btn_normally_open"));
    win_objs.btn_nclose = GTK_WIDGET(gtk_builder_get_object(builder, "btn_normall_close"));    
    win_objs.img_banner = (GtkImage*)gtk_builder_get_object(builder, "img_alien");
    
    
    gtk_dialog_add_buttons( (GtkDialog*)win_objs.dlg_settings, 
		"Cancel", GTK_RESPONSE_CANCEL,
		"OK", GTK_RESPONSE_OK, NULL );
    
    gtk_builder_connect_signals(builder, &win_objs);
    
    g_object_unref(builder);

    /* load file */
    set_private_data_path();
    load_window_icon( (GtkWindow*)win_objs.main_window );
    load_banner( win_objs.img_banner );
        
	/* set initial status */
    win_objs.status.connected = FALSE;
    win_objs.status.relay_on_off = RELAY_OFF;
    win_objs.status.circuit_conf = NORMALLY_OPEN;  
    gtk_label_set_text(GTK_LABEL(win_objs.lbl_status), "?");
    
    /* disable ON/OFF buttons */
    enable_relay_ctrl_buttons(&win_objs, FALSE);
    scan_device_node(&win_objs);
    
    gtk_widget_show(win_objs.main_window);                
    gtk_main();

    return 0;    
}

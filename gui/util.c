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
#include <stdlib.h>
#include <dirent.h> 
#include <pwd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "common.h"
#include "util.h"


static gchar        *private_data_path;

/*
 * 
 * sets application private path 
 * 
 */
void set_private_data_path()
{
    struct passwd   *pw      = getpwuid(getuid());
    const char      *homedir = pw->pw_dir;

    debug_msg( "Home dir: %s\n", homedir );
    
    private_data_path = (char*)malloc( strlen(homedir) + strlen(PRIVATE_FOLDER) + 1 );
    
    if ( !private_data_path ) 
    {
        fprintf( stderr, "fail to alloc private data path\n");
        return;
    }
        
    sprintf( private_data_path, "%s/"PRIVATE_FOLDER, homedir );
}

/*
 * 
 * sets window's icon
 * 
 */
void load_window_icon( GtkWindow* window )
{
    GError          *error = NULL;
    char            win_icon_path[MAX_FILE_SIZE];
    
    if ( !private_data_path )
    {
        fprintf( stderr, "private path is not set!\n");
        return;
    }
    
    sprintf( win_icon_path, "%s/images/"ICON_MAINWIN, private_data_path );
    
    /* set window icon */
    gtk_window_set_icon_from_file ( window, win_icon_path, &error );
}

void load_banner( GtkImage* image )
{
    char            banner_path[MAX_FILE_SIZE];
    
    if ( !private_data_path )
    {
        fprintf( stderr, "private path is not set!\n");
        return;
    }
    
    sprintf( banner_path, "%s/images/"IMAGE_BANNER, private_data_path );
    
    gtk_image_set_from_file (image, banner_path );
}

char* get_private_data_path()
{
    return (char*)private_data_path;
}

//------------------------------------------------------------------



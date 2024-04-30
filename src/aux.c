#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "defines.h"

gboolean find_msg(gpointer element, gpointer pid) {
    Msg msg = (Msg) element;
    int target_pid = GPOINTER_TO_INT(pid);
    return (msg->pid == target_pid);
}



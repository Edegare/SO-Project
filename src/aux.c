#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "defines.h"

gboolean find_msg_remove(GArray *array, int pid) {
    guint index;
    for (index = 0; index < array->len; index++) {
        Msg msg = g_array_index(array, Msg, index);
        if (msg->pid == pid) {
            free(msg);
            g_array_remove_index(array, index);

            return TRUE;
        }
    }
    return FALSE;
}



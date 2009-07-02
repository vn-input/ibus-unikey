#ifndef __UNIKEY_MACRO_TABLE_H__
#define __UNIKEY_MACRO_TABLE_H__

#include "mactab.h"

GtkWidget* unikey_macro_dialog_new();
void unikey_macro_dialog_load_macro(GtkDialog* dialog, CMacroTable macro);
void unikey_macro_dialog_save_macro(GtkDialog* dialog, CMacroTable* macro);

#endif

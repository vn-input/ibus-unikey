#ifndef __ENGINE_CONST_H__
#define __ENGINE_CONST_H__


#define UNIKEY_MACRO_FILE        ".ibus/unikey/macro"

#define CONFIG_SECTION           "engine/Unikey"

#define CONFIG_INPUTMETHOD       "input_method"
#define CONFIG_OUTPUTCHARSET     "output_charset"
#define CONFIG_SPELLCHECK        "spell_check"
#define CONFIG_AUTORESTORENONVN  "auto_restore_non_vn"
#define CONFIG_MODERNSTYLE       "modern_style"
#define CONFIG_FREEMARKING       "free_marking"
#define CONFIG_MACROENABLED      "macro_enabled"
#define CONFIG_PROCESSWATBEGIN   "process_word_at_begin"
#define CONFIG_MOUSECAPTURE      "mouse_capture"

// DEFAULT options
#define DEFAULT_CONF_SPELLCHECK         TRUE
#define DEFAULT_CONF_AUTONONVNRESTORE   TRUE
#define DEFAULT_CONF_MODERNSTYLE        FALSE
#define DEFAULT_CONF_FREEMARKING        TRUE
#define DEFAULT_CONF_MACROENABLED       FALSE
#define DEFAULT_CONF_PROCESSWATBEGIN    TRUE
#define DEFAULT_CONF_MOUSECAPTURE       TRUE

#define GCONF_PREFIX                    "/desktop/ibus/engine/Unikey/"
#define UNIKEY_INPUTMETHOD              "InputMethod"
#define UNIKEY_OUTPUTCHARSET            "OutputCharset"
#define UNIKEY_OPTION_SPELLCHECK        "Options/SpellCheckEnabled"
#define UNIKEY_OPTION_AUTONONVNRESTORE  "Options/AutoNonVnRestore"
#define UNIKEY_OPTION_MODERNSTYLE       "Options/ModernStyle"
#define UNIKEY_OPTION_FREEMARKING       "Options/FreeMarking"
#define UNIKEY_OPTION_MACROENABLED      "Options/MacroEnabled"
#define UNIKEY_OPTION_PROCESSWATBEGIN   "Options/ProcessWAtBegin"
#define UNIKEY_OPTION_MOUSECAPTURE      "Options/MouseCapture"

#endif


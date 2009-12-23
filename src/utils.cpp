#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libintl.h>

#include <ibus.h>
#include "ukengine.h"
#include "utils.h"
#include "engine_const.h"

#define _(string) gettext(string)

#define IU_DESC _("Vietnamese Input Method Engine for IBus using Unikey Engine")

IBusComponent* ibus_unikey_get_component()
{
    IBusComponent* component;
    IBusEngineDesc* engine;

    component = ibus_component_new("org.freedesktop.IBus.Unikey",
                                   "Unikey",
                                   PACKAGE_VERSION,
                                   "GPLv2",
                                   "Lê Quốc Tuấn <mr.lequoctuan@gmail.com>",
                                   PACKAGE_BUGREPORT,
                                   "",
                                   PACKAGE_NAME);

    engine = ibus_engine_desc_new
    (
        "Unikey",
        "Unikey",
        IU_DESC,
        "vi",
        "GPLv2",
        "Lê Quốc Tuấn <mr.lequoctuan@gmail.com>",
        PKGDATADIR"/icons/ibus-unikey.png",
        "us"
    );
    
    engine->rank = 99;

    ibus_component_add_engine(component, engine);

    return component;
}

void unikey_create_default_options(UnikeyOptions *pOpt)
{
  pOpt->freeMarking         = UNIKEY_OPT_FREEMARKING;
  pOpt->modernStyle         = UNIKEY_OPT_MODERNSTYLE;
  pOpt->macroEnabled        = UNIKEY_OPT_MACROENABLED;
  pOpt->useUnicodeClipboard = 0; // not use
  pOpt->alwaysMacro         = 0; // not use
  pOpt->spellCheckEnabled   = UNIKEY_OPT_SPELLCHECKENABLED;
  pOpt->autoNonVnRestore    = UNIKEY_OPT_AUTONONVNRESTORE;
}

// code from x-unikey, for convert charset that not is XUtf-8
int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize)
{
    int i;
    int outLeft;
    unsigned char ch;

    outLeft = *pOutSize;

    for (i=0; i<inSize; i++)
    {
        ch = *src++;
        if (ch < 0x80)
        {
            outLeft -= 1;
            if (outLeft >= 0)
                *dst++ = ch;
        }
        else
        {
            outLeft -= 2;
            if (outLeft >= 0)
            {
                *dst++ = (0xC0 | ch >> 6);
                *dst++ = (0x80 | (ch & 0x3F));
            }
        }
    }

    *pOutSize = outLeft;
    return (outLeft >= 0);
}


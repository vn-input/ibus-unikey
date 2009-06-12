// -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2000-2005 Pham Kim Long
 * Contact:
 *   unikey@gmail.com
 *   UniKey project: http://unikey.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <iostream>
#include <string.h>
#include <stdio.h>
using namespace std;

#include <ctype.h>
#include "usrkeymap.h"


int getLabelIndex(int action);
void initKeyMap(int keyMap[256]);

#define OPT_COMMENT_CHAR ';'

struct UkEventLabelPair
{
    char label[32];
    int ev;
};

UkEventLabelPair UkEvLabelList[] = {
    {"Tone0", vneTone0},
    {"Tone1", vneTone1},
    {"Tone2", vneTone2},
    {"Tone3", vneTone3},
    {"Tone4", vneTone4},
    {"Tone5", vneTone5},
    {"Roof-All", vneRoofAll},
    {"Roof-A", vneRoof_a},
    {"Roof-E", vneRoof_e}, 
    {"Roof-O", vneRoof_o},
    {"Hook-Bowl", vneHookAll},
    {"Hook-UO", vneHook_uo},
    {"Hook-U", vneHook_u},
    {"Hook-O", vneHook_o},
    {"Bowl", vneBowl},
    {"D-Mark", vneDd},
    {"Telex-W", vne_telex_w},
    {"Escape", vneEscChar},
    {"DD", vneCount + vnl_DD},
    {"dd", vneCount + vnl_dd},
    {"A^", vneCount + vnl_Ar},
    {"a^", vneCount + vnl_ar},
    {"A(", vneCount + vnl_Ab},
    {"a(", vneCount + vnl_ab},
    {"E^", vneCount + vnl_Er},
    {"e^", vneCount + vnl_er},
    {"O^", vneCount + vnl_Or},
    {"o^", vneCount + vnl_or},
    {"O+", vneCount + vnl_Oh},
    {"o+", vneCount + vnl_oh},
    {"U+", vneCount + vnl_Uh},
    {"u+", vneCount + vnl_uh}
};

const int UkEvLabelCount = sizeof(UkEvLabelList)/sizeof(UkEventLabelPair);

//--------------------------------------------------
static int parseNameValue(char *line, char **name, char **value)
{
    char *p, *mark;
    char ch;

    if (line == 0)
        return 0;

    // get rid of comment
    p = strchr(line, OPT_COMMENT_CHAR);
    if (p)
        *p = 0;

    //get option name
    for (p=line; *p == ' '; p++);
    if (*p == 0)
        return 0;

    *name = p;
    mark = p; //mark the last non-space character
    p++;
    while ((ch=*p) != '=' && ch!=0) {
        if (ch != ' ')
            mark = p;
        p++;
    }

    if (ch == 0)
        return 0;
    *(mark+1) = 0; //terminate name with a null character

    //get option value
    p++;
    while (*p == ' ') p++;
    if (*p == 0)
        return 0;

    *value = p;
    mark = p;
    while (*p) { //strip trailing spaces
        if (*p != ' ')
            mark = p;
        p++;
    }
    *++mark = 0;
    return 1;
}

//-----------------------------------------------------
DllExport int UkLoadKeyMap(const char *fileName, int keyMap[256])
{
    int i, mapCount;
    UkKeyMapPair orderMap[256];
    if (!UkLoadKeyOrderMap(fileName, orderMap, &mapCount))
        return 0;

    initKeyMap(keyMap);
    for (i=0; i < mapCount; i++) {
        keyMap[orderMap[i].key] = orderMap[i].action;
        if (orderMap[i].action < vneCount) {
            keyMap[tolower(orderMap[i].key)] = orderMap[i].action;
        }
    }
    return 1;
}

//------------------------------------------------------------------
DllExport int UkLoadKeyOrderMap(const char *fileName, UkKeyMapPair *pMap, int *pMapCount)
{
    FILE *f;
    char *buf;
    char *name, *value;
    size_t len;
    int i, bufSize, lineCount;
    unsigned char c;
    int mapCount;
    int keyMap[256];

    f = fopen(fileName, "r");
    if (f == 0) {
        cerr << "Failed to open file: " << fileName << endl;
        return 0;
    }

    initKeyMap(keyMap);
    bufSize = 256;
    buf = new char[bufSize];

    lineCount = 0;
    mapCount = 0;
    while (!feof(f)) {
        if (fgets((char *)buf, bufSize, f) == 0)
            break;
        lineCount++;
        len = strlen(buf);
        if (len == 0)
            break;

        if (buf[len-1] == '\n')
            buf[len-1] = 0;
        if (parseNameValue(buf, (char **)&name, (char **)&value)) {
            if (strlen(name) == 1) {
                for (i=0; i < UkEvLabelCount; i++) {
                    if (strcmp(UkEvLabelList[i].label, value) == 0) {
                        c = (unsigned char)name[0];
                        if (keyMap[c] != vneNormal) {
                            //already assigned, don't accept this map
                            break;
                        }
                        //cout << "key: " << c << " value: " << UkEvLabelList[i].ev << endl; //DEBUG
                        keyMap[c] = UkEvLabelList[i].ev;
                        pMap[mapCount].action = UkEvLabelList[i].ev;
                        if (keyMap[c] < vneCount) {
                            pMap[mapCount].key = toupper(c);
                            keyMap[toupper(c)] = UkEvLabelList[i].ev;
                        }
                        else {
                            pMap[mapCount].key = c;
                        }
                        mapCount++;
                        break;
                    }
                }
                if (i == UkEvLabelCount) {
                    cerr << "Error in user key layout, line " << lineCount << ": command not found" << endl;
                }
            }
            else {
                cerr << "Error in user key layout, line " << lineCount 
                     << ": key name is not a single character" << endl;	
            }
        }
    }
    delete [] buf;
    fclose(f);

    *pMapCount = mapCount;

    return 1;
}

//-------------------------------------------
void initKeyMap(int keyMap[256])
{
    unsigned int c;
    for (c=0; c<256; c++)
        keyMap[c] = vneNormal;
}

const char *UkKeyMapHeader = 
    "; This is UniKey user-defined key mapping file, generated from UniKey (Windows)\n\n";

DllExport int UkStoreKeyOrderMap(const char *fileName, UkKeyMapPair *pMap, int mapCount)
{
    FILE *f;
    int i;
    int labelIndex;
    char line[128];

    f = fopen(fileName, "wt");
    if (f == 0) {
        cerr << "Failed to open file: " << fileName << endl;
        return 0;
    }

    fputs(UkKeyMapHeader, f);
    for (i=0; i < mapCount; i++) {
        labelIndex = getLabelIndex(pMap[i].action);
        if (labelIndex != -1) {
            sprintf(line, "%c = %s\n", pMap[i].key, UkEvLabelList[labelIndex].label);
            fputs(line, f);
        }
    }
    fclose(f);
    return 1;
}

int getLabelIndex(int event)
{
    int i;
    for (i = 0; i < UkEvLabelCount; i++) {
        if (UkEvLabelList[i].ev == event)
            return i;
    }
    return -1;
}

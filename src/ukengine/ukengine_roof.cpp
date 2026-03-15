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

/**
 * @file ukengine.cpp
 * @brief Core Vietnamese input engine implementation.
 *
 * This file contains the internal lookup tables, validation helpers, and key
 * processing routines used to transform raw keystrokes into Vietnamese text,
 * including tone placement, macro expansion, spell-check-aware restoration,
 * and charset-specific output generation.
 */
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "keycons.h"

/*
#if defined(_WIN32)
#include "keyhook.h"
#endif
*/

#include "vnlexi.h"
#include "ukengine.h"

#include "charset.h"

using namespace std;

#include "ukengine_internal.h"

int UkEngine::processRoof(UkKeyEvent & ev)
{
    if (!m_pCtrl->vietKey || m_current < 0 || m_buffer[m_current].vOffset < 0)
        return processAppend(ev);

    VnLexiName target;
    switch (ev.evType) {
    case vneRoof_a:
        target = vnl_ar;
        break;
    case vneRoof_e:
        target = vnl_er;
        break;
    case vneRoof_o:
        target = vnl_or;
        break;
    default:
        target = vnl_nonVnChar;
    }


    VowelSeq vs, newVs;
    int i, vStart, vEnd;
    int curTonePos, newTonePos, tone;
    int changePos;
    bool roofRemoved = false;

    vEnd = m_current - m_buffer[m_current].vOffset;
    vs = m_buffer[vEnd].vseq;
    vStart = vEnd - (VSeqList[vs].len - 1);
    curTonePos = vStart + getTonePosition(vs, vEnd == m_current);
    tone = m_buffer[curTonePos].tone;

    bool doubleChangeUO = false;
    if (vs == vs_uho || vs == vs_uhoh || vs == vs_uhoi || vs == vs_uhohi) {
        //special cases: u+o+ -> uo^, u+o -> uo^, u+o+i -> uo^i, u+oi -> uo^i
        newVs = lookupVSeq(vnl_u, vnl_or, VSeqList[vs].v[2]);
        doubleChangeUO = true;
    }
    else {
        newVs = VSeqList[vs].withRoof;
    }

    VowelSeqInfo *pInfo;

    if (newVs == vs_nil) {
        if (VSeqList[vs].roofPos == -1)
            return processAppend(ev); //roof is not applicable
    
        //a roof already exists -> undo roof
        VnLexiName curCh = m_buffer[vStart + VSeqList[vs].roofPos].vnSym;
        if (target != vnl_nonVnChar && curCh != target)
            return processAppend(ev); //specific roof and the roof character don't match

        VnLexiName newCh = (curCh == vnl_ar)? vnl_a : ((curCh == vnl_er)? vnl_e : vnl_o);
        changePos = vStart + VSeqList[vs].roofPos;

        if (!m_pCtrl->options.freeMarking && changePos != m_current)
            return processAppend(ev);

        markChange(changePos);
        m_buffer[changePos].vnSym = newCh;

        if (VSeqList[vs].len == 3)
            newVs = lookupVSeq(m_buffer[vStart].vnSym, m_buffer[vStart+1].vnSym, m_buffer[vStart+2].vnSym);
        // two-character vowel sequence when removing a roof
        else if (VSeqList[vs].len == 2)
            newVs = lookupVSeq(m_buffer[vStart].vnSym, m_buffer[vStart+1].vnSym);
        else
            newVs = lookupVSeq(m_buffer[vStart].vnSym);

        pInfo = &VSeqList[newVs];
        roofRemoved = true;
    }
    else {
        pInfo = &VSeqList[newVs];
        if (target != vnl_nonVnChar &&  pInfo->v[pInfo->roofPos] != target)
            return processAppend(ev);

        //check validity of new VC and CV
        bool valid = true;
        ConSeq c1 = cs_nil;
        ConSeq c2 = cs_nil;
        if (m_buffer[m_current].c1Offset != -1)
            c1 = m_buffer[m_current-m_buffer[m_current].c1Offset].cseq;
        
        if (m_buffer[m_current].c2Offset != -1)
            c2 = m_buffer[m_current-m_buffer[m_current].c2Offset].cseq;

        valid = isValidCVC(c1, newVs, c2);
        if (!valid)
            return processAppend(ev);

        if (doubleChangeUO) {
            changePos = vStart;
        }
        else {
            changePos = vStart + pInfo->roofPos;
        }
        if (!m_pCtrl->options.freeMarking && changePos != m_current)
            return processAppend(ev);
        markChange(changePos);
        if (doubleChangeUO) {
            m_buffer[vStart].vnSym = vnl_u;
            m_buffer[vStart+1].vnSym = vnl_or;
        }
        else {
            m_buffer[changePos].vnSym = pInfo->v[pInfo->roofPos];
        }
    }

    for (i=0; i < pInfo->len; i++) { //update sub-sequences
        m_buffer[vStart+i].vseq = pInfo->sub[i];
    }

    //check if tone re-position is needed
    newTonePos = vStart + getTonePosition(newVs, vEnd == m_current);
    /* //For now, users don't seem to like the following processing, thus commented out
    if (roofRemoved && tone != 0 &&
        (!pInfo->complete || changePos == curTonePos)) {
        //remove tone if the vowel sequence becomes incomplete as a result of roof removal OR
        //if removed roof is at the same position as the current tone
        markChange(curTonePos);
        m_buffer[curTonePos].tone = 0;
    } else
    */
    if (curTonePos != newTonePos && tone != 0) {
        markChange(newTonePos);
        m_buffer[newTonePos].tone = tone;
        markChange(curTonePos);
        m_buffer[curTonePos].tone = 0;
    }

    if (roofRemoved) {
        m_singleMode = false;
        processAppend(ev);
        m_reverted = true;
    }

    return 1;
}

//------------------------------------------------------------------
// can only be called from processHook
//------------------------------------------------------------------
/**
 * @brief Handle hook/bowl toggles for the special `uo` family of vowels.
 *
 * @param ev Hook-related key event.
 * @return Non-zero when the sequence was updated.
 */
int UkEngine::processHookWithUO(UkKeyEvent & ev)
{
    VowelSeq vs, newVs;
    int i, vStart, vEnd;
    int curTonePos, newTonePos, tone;
    bool hookRemoved = false;
    bool removeWithUndo = true;
    bool toneRemoved = false;
    
    (void)toneRemoved; // fix warning
    
    VnLexiName *v;

    if (!m_pCtrl->options.freeMarking && m_buffer[m_current].vOffset != 0)
        return processAppend(ev);    

    vEnd = m_current - m_buffer[m_current].vOffset;
    vs = m_buffer[vEnd].vseq;
    vStart = vEnd - (VSeqList[vs].len - 1);
    v = VSeqList[vs].v;
    curTonePos = vStart + getTonePosition(vs, vEnd == m_current);
    tone = m_buffer[curTonePos].tone;

    switch (ev.evType) {
    case vneHook_u:
        if (v[0] == vnl_u) {
            newVs = VSeqList[vs].withHook;
            markChange(vStart);
            m_buffer[vStart].vnSym = vnl_uh;
        }
        else {// v[0] = vnl_uh, -> uo
            newVs = lookupVSeq(vnl_u, vnl_o, v[2]);
            markChange(vStart);
            m_buffer[vStart].vnSym = vnl_u;
            m_buffer[vStart+1].vnSym = vnl_o;
            hookRemoved = true;
            toneRemoved =  (m_buffer[vStart].tone != 0);
        }
        break;
    case vneHook_o:
        if (v[1] == vnl_o || v[1] == vnl_or) {
            if (vEnd == m_current && VSeqList[vs].len == 2 && 
                m_buffer[m_current].form == vnw_cv && m_buffer[m_current-2].cseq == cs_th)
            {
                // o|o^ -> o+
                newVs = VSeqList[vs].withHook;
                markChange(vStart+1);
                m_buffer[vStart+1].vnSym = vnl_oh;
            }
            else {
                newVs = lookupVSeq(vnl_uh, vnl_oh, v[2]);
                if (v[0] == vnl_u) {
                    markChange(vStart);
                    m_buffer[vStart].vnSym = vnl_uh;
                    m_buffer[vStart+1].vnSym = vnl_oh;
                }
                else {
                    markChange(vStart+1);
                    m_buffer[vStart+1].vnSym = vnl_oh;
                }
            }
        }
        else {// v[1] = vnl_oh, -> uo
            newVs = lookupVSeq(vnl_u, vnl_o, v[2]);
            if (v[0] == vnl_uh) {
                markChange(vStart);
                m_buffer[vStart].vnSym = vnl_u;
                m_buffer[vStart+1].vnSym = vnl_o;
            }
            else {
                markChange(vStart+1);
                m_buffer[vStart+1].vnSym = vnl_o;
            }
            hookRemoved = true;
            toneRemoved = (m_buffer[vStart+1].tone != 0);
        }
        break;
    default:  //vneHookAll, vneHookUO:
        if (v[0] == vnl_u) {
            if (v[1] == vnl_o || v[1] == vnl_or) { 
                //uo -> uo+ if prefixed by "th"
                if ((vs == vs_uo || vs == vs_uor) && vEnd == m_current && 
                    m_buffer[m_current].form == vnw_cv && m_buffer[m_current-2].cseq == cs_th) 
                {
                    newVs = vs_uoh;
                    markChange(vStart+1);
                    m_buffer[vStart+1].vnSym = vnl_oh;
                }
                else {
                    //uo -> u+o+
                    newVs = VSeqList[vs].withHook;
                    markChange(vStart);
                    m_buffer[vStart].vnSym = vnl_uh;
                    newVs = VSeqList[newVs].withHook;
                    m_buffer[vStart+1].vnSym = vnl_oh;
                }
            }
            else {//uo+ -> u+o+
                newVs = VSeqList[vs].withHook;
                markChange(vStart);
                m_buffer[vStart].vnSym = vnl_uh;
            }
        }
        else {//v[0] == vnl_uh
            if (v[1] == vnl_o) { // u+o -> u+o+
                newVs = VSeqList[vs].withHook;
                markChange(vStart+1);
                m_buffer[vStart+1].vnSym = vnl_oh;
            }
            else { //v[1] == vnl_oh, u+o+ -> uo
                newVs = lookupVSeq(vnl_u, vnl_o, v[2]); //vs_uo;
                markChange(vStart);
                m_buffer[vStart].vnSym = vnl_u;
                m_buffer[vStart+1].vnSym = vnl_o;
                hookRemoved = true;
                toneRemoved = (m_buffer[vStart].tone != 0 || m_buffer[vStart+1].tone != 0);
            }
        }
        break;
    }

    VowelSeqInfo *p = &VSeqList[newVs];
    for (i=0; i < p->len; i++) { //update sub-sequences
        m_buffer[vStart+i].vseq = p->sub[i];
    }

    //check if tone re-position is needed
    newTonePos = vStart + getTonePosition(newVs, vEnd == m_current);
    /* //For now, users don't seem to like the following processing, thus commented out
    if (hookRemoved && tone != 0 && (!p->complete || toneRemoved)) {
        //remove tone if the vowel sequence becomes incomplete as a result of hook removal
        //OR if a removed hook is at the same position as the current tone
        markChange(curTonePos);
        m_buffer[curTonePos].tone = 0;
    }
    else 
    */
    if (curTonePos != newTonePos && tone != 0) {
        markChange(newTonePos);
        m_buffer[newTonePos].tone = tone;
        markChange(curTonePos);
        m_buffer[curTonePos].tone = 0;
    }

    if (hookRemoved && removeWithUndo) {
        m_singleMode = false;
        processAppend(ev);
        m_reverted = true;
    }

    return 1;
}

//------------------------------------------------------------------
/**
 * @brief Apply or remove hook and bowl marks on the active vowel sequence.
 *
 * @param ev Key event describing the requested mark.
 * @return Non-zero when the event changed the composition.
 */
int UkEngine::processHook(UkKeyEvent & ev)
{
    if (!m_pCtrl->vietKey || m_current < 0 || m_buffer[m_current].vOffset < 0)
        return processAppend(ev);

    VowelSeq vs, newVs;
    int i, vStart, vEnd;
    int curTonePos, newTonePos, tone;
    int changePos;
    bool hookRemoved = false;
    VowelSeqInfo *pInfo;
    VnLexiName *v;

    vEnd = m_current - m_buffer[m_current].vOffset;
    vs = m_buffer[vEnd].vseq;

    v = VSeqList[vs].v;
  
    if (VSeqList[vs].len > 1 && 
        ev.evType != vneBowl &&
        (v[0] == vnl_u || v[0] == vnl_uh) &&
        (v[1] == vnl_o || v[1] == vnl_oh || v[1] == vnl_or))
        return processHookWithUO(ev);

    vStart = vEnd - (VSeqList[vs].len - 1);
    curTonePos = vStart + getTonePosition(vs, vEnd == m_current);
    tone = m_buffer[curTonePos].tone;

    newVs = VSeqList[vs].withHook;
    if (newVs == vs_nil) {
        if (VSeqList[vs].hookPos == -1)
            return processAppend(ev); //hook is not applicable

        //a hook already exists -> undo hook
        VnLexiName curCh = m_buffer[vStart + VSeqList[vs].hookPos].vnSym;
        VnLexiName newCh = (curCh == vnl_ab)? vnl_a : ((curCh == vnl_uh)? vnl_u : vnl_o);
        changePos = vStart + VSeqList[vs].hookPos;
        if (!m_pCtrl->options.freeMarking && changePos != m_current)
            return processAppend(ev);

        switch (ev.evType) {
        case vneHook_u:
            if (curCh != vnl_uh)
                return processAppend(ev);
            break;
        case vneHook_o:
            if (curCh != vnl_oh)
                return processAppend(ev);
            break;
        case vneBowl:
            if (curCh != vnl_ab)
                return processAppend(ev);
            break;
        default:
            if (ev.evType == vneHook_uo && curCh == vnl_ab)
                return processAppend(ev);
        }

        markChange(changePos);
        m_buffer[changePos].vnSym = newCh;

        if (VSeqList[vs].len == 3)
            newVs = lookupVSeq(m_buffer[vStart].vnSym, m_buffer[vStart+1].vnSym, m_buffer[vStart+2].vnSym);
        // two-character vowel sequence when removing a hook/bowl
        else if (VSeqList[vs].len == 2)
            newVs = lookupVSeq(m_buffer[vStart].vnSym, m_buffer[vStart+1].vnSym);
        else
            newVs = lookupVSeq(m_buffer[vStart].vnSym);

        pInfo = &VSeqList[newVs];
        hookRemoved = true;
    }
    else {
        pInfo = &VSeqList[newVs];

        switch (ev.evType) {
        case vneHook_u:
            if (pInfo->v[pInfo->hookPos] != vnl_uh)
                return processAppend(ev);
            break;
        case vneHook_o:
            if (pInfo->v[pInfo->hookPos] != vnl_oh)
                return processAppend(ev);
            break;
        case vneBowl:
            if (pInfo->v[pInfo->hookPos] != vnl_ab)
                return processAppend(ev);
            break;
        default: //vneHook_uo, vneHookAll
            if (ev.evType == vneHook_uo && pInfo->v[pInfo->hookPos] == vnl_ab)
                return processAppend(ev);
        }

        //check validity of new VC and CV
        bool valid = true;
        ConSeq c1 = cs_nil;
        ConSeq c2 = cs_nil;
        if (m_buffer[m_current].c1Offset != -1)
            c1 = m_buffer[m_current-m_buffer[m_current].c1Offset].cseq;
        
        if (m_buffer[m_current].c2Offset != -1)
            c2 = m_buffer[m_current-m_buffer[m_current].c2Offset].cseq;

        valid = isValidCVC(c1, newVs, c2);

        if (!valid)
            return processAppend(ev);

        changePos = vStart + pInfo->hookPos;
        if (!m_pCtrl->options.freeMarking && changePos != m_current)
            return processAppend(ev);

        markChange(changePos);
        m_buffer[changePos].vnSym = pInfo->v[pInfo->hookPos];
    }
   
    for (i=0; i < pInfo->len; i++) { //update sub-sequences
        m_buffer[vStart+i].vseq = pInfo->sub[i];
    }

    //check if tone re-position is needed
    newTonePos = vStart + getTonePosition(newVs, vEnd == m_current);
    /* //For now, users don't seem to like the following processing, thus commented out
    if (hookRemoved && tone != 0 && 
        (!pInfo->complete || (hookRemoved && curTonePos == changePos))) {
        //remove tone if the vowel sequence becomes incomplete as a result of hook removal
        //OR if a removed hook was at the same position as the current tone
        markChange(curTonePos);
        m_buffer[curTonePos].tone = 0;
    }
    else */
    if (curTonePos != newTonePos && tone != 0) {
        markChange(newTonePos);
        m_buffer[newTonePos].tone = tone;
        markChange(curTonePos);
        m_buffer[curTonePos].tone = 0;
    }

    if (hookRemoved) {
        m_singleMode = false;
        processAppend(ev);
        m_reverted = true;
    }

    return 1;
}

//----------------------------------------------------------
/**
 * @brief Determine which character in a vowel sequence should carry the tone.
 *
 * @param vs Vowel sequence being analyzed.
 * @param terminated `true` when the syllable already ended with a consonant or separator.
 * @return Zero-based offset of the tone-bearing character inside the sequence.
 */
int UkEngine::getTonePosition(VowelSeq vs, bool terminated)
{
    VowelSeqInfo & info = VSeqList[vs];
    if (info.len == 1)
        return 0;

    if (info.roofPos != -1)
        return info.roofPos;
    if (info.hookPos != -1) {
        if (vs == vs_uhoh || vs == vs_uhohi || vs == vs_uhohu) //u+o+, u+o+u, u+o+i
            return 1;
        return info.hookPos;
    }
  
    if (info.len == 3)
        return 1;

    if (m_pCtrl->options.modernStyle &&
        (vs == vs_oa || vs == vs_oe ||vs == vs_uy))
        return 1;

    return terminated ? 0 : 1;
}

//----------------------------------------------------------
/**
 * @brief Apply, replace, or remove a tone mark on the current syllable.
 *
 * @param ev Tone-mark key event.
 * @return Non-zero when the tone logic handled the event.
 */
int UkEngine::processTone(UkKeyEvent & ev)
{
    if (m_current < 0 || !m_pCtrl->vietKey)
        return processAppend(ev);

    if (m_buffer[m_current].form == vnw_c && 
        (m_buffer[m_current].cseq == cs_gi || m_buffer[m_current].cseq == cs_gin)) {
        int p = (m_buffer[m_current].cseq == cs_gi)? m_current : m_current - 1;
        if (m_buffer[p].tone == 0 && ev.tone == 0)
            return processAppend(ev);
        markChange(p);
        if (m_buffer[p].tone == ev.tone) {
            m_buffer[p].tone = 0;
            m_singleMode = false;
            processAppend(ev);
            m_reverted = true;
            return 1;
        }
        m_buffer[p].tone = ev.tone;
        return 1;
    }

    if (m_buffer[m_current].vOffset < 0)
        return processAppend(ev);

    int vEnd;
    VowelSeq vs;

    vEnd = m_current - m_buffer[m_current].vOffset;
    vs = m_buffer[vEnd].vseq;
    VowelSeqInfo & info = VSeqList[vs];
    if (m_pCtrl->options.spellCheckEnabled && !m_pCtrl->options.freeMarking && !info.complete)
        return processAppend(ev);

    if (m_buffer[m_current].form == vnw_vc || m_buffer[m_current].form == vnw_cvc) {
        ConSeq cs = m_buffer[m_current].cseq;
        if ((cs == cs_c || cs == cs_ch || cs == cs_p || cs == cs_t) &&
            (ev.tone == 2 || ev.tone == 3 || ev.tone == 4))
            return processAppend(ev); // c, ch, p, t suffixes don't allow ` ? ~
    }
      
    int toneOffset = getTonePosition(vs, vEnd == m_current);
    int tonePos = vEnd - (info.len -1 ) + toneOffset;
    if (m_buffer[tonePos].tone == 0 && ev.tone == 0)
        return processAppend(ev);

    if (m_buffer[tonePos].tone == ev.tone) {
        markChange(tonePos);
        m_buffer[tonePos].tone = 0;
        m_singleMode = false;
        processAppend(ev);
        m_reverted = true;
        return 1;
    }

    markChange(tonePos);
    m_buffer[tonePos].tone = ev.tone;
    return 1;
}

//----------------------------------------------------------
/**
 * @brief Toggle the `d`/`đ` transformation for the current word position.
 *
 * @param ev Key event corresponding to the `d` trigger.
 * @return Non-zero when the transformation was applied or reverted.
 */
int UkEngine::processDd(UkKeyEvent & ev)
{
    if (!m_pCtrl->vietKey || m_current < 0)
        return processAppend(ev);
    
    int pos;

    // we want to allow dd even in non-vn sequence, because dd is used a lot in abbreviation
    // we allow dd only if preceding character is not a vowel
    if (m_buffer[m_current].form == vnw_nonVn && 
        m_buffer[m_current].vnSym == vnl_d &&
        (m_buffer[m_current-1].vnSym == vnl_nonVnChar ||!IsVnVowel[m_buffer[m_current-1].vnSym]))
    {
        m_singleMode = true;
        pos = m_current;
        markChange(pos);
        m_buffer[pos].cseq = cs_dd;
        m_buffer[pos].vnSym = vnl_dd;
        m_buffer[pos].form = vnw_c;
        m_buffer[pos].c1Offset = 0;
        m_buffer[pos].c2Offset = -1;
        m_buffer[pos].vOffset = -1;
        return 1;
    }

    if (m_buffer[m_current].c1Offset < 0) {
        return processAppend(ev);
    }

    pos = m_current - m_buffer[m_current].c1Offset;
    if (!m_pCtrl->options.freeMarking && pos != m_current)
        return processAppend(ev);

    if (m_buffer[pos].cseq == cs_d) {
        markChange(pos);
        m_buffer[pos].cseq = cs_dd;
        m_buffer[pos].vnSym = vnl_dd;
        //never spellcheck a word which starts with dd, because it's used alot in abbreviation
        m_singleMode = true;
        return 1;
    }

    if (m_buffer[pos].cseq == cs_dd) {
        //undo dd
        markChange(pos);
        m_buffer[pos].cseq = cs_d;
        m_buffer[pos].vnSym = vnl_d;
        m_singleMode = false;
        processAppend(ev);
        m_reverted = true;
        return 1;
    }
  
    return processAppend(ev);
}

//----------------------------------------------------------
/**
 * @brief Flip a Vietnamese lexicon symbol between lowercase and uppercase.
 *
 * @param x Symbol to convert.
 * @return Same symbol in the opposite case, or unchanged for non-Vietnamese values.
 */
VnLexiName changeCase(VnLexiName x)
{
    if (x == vnl_nonVnChar)
        return x;
    if (!(x & 0x01))
        return (VnLexiName)(x+1);
    return (VnLexiName)(x-1);
}

//----------------------------------------------------------
/**
 * @brief Normalize a Vietnamese lexicon symbol to lowercase.
 *
 * @param x Symbol to normalize.
 * @return Lowercase equivalent, or unchanged for non-Vietnamese values.
 */
VnLexiName vnToLower(VnLexiName x)
{
    if (x == vnl_nonVnChar)
        return x;
    if (!(x & 0x01)) //even
        return (VnLexiName)(x+1);
    return x;
}

//----------------------------------------------------------
/**
 * @brief Process a character that may map to a Vietnamese symbol or action.
 *
 * @param ev Event to append and possibly reinterpret using map-char rules.
 * @return Non-zero when the mapped processing changed engine state.
 */

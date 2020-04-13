/*===--- asubimpuretheorem.cpp - source for DarkLogic library --------------*- C++ -*-===*
*
*   Part of the DarkLogic Project, under the CC0 license.
*   See https://creativecommons.org/publicdomain/zero/1.0/deed.fr for license information.
*
*===----------------------------------------------------------------------===*
*
*   This file implements asubimpuretheorem class for DarkLogic Library
*   Author: BlackWalker
*   Date: april 12 2020
*
*===----------------------------------------------------------------------===*/

#include "asubimpuretheorem.h"

using namespace N_DarkLogic;

IISubTheoremFormula::TheoremFormulaType ASubImpureTheorem::getFormulaType() const
{
    return SUBIMPURETH;
}

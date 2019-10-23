//===-------- LoopOptTutorial.cpp - Loop Opt Tutorial Pass ------*- C++ -*-===//
//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file contains a small loop pass to be used to illustrate several
/// aspects about writing a loop optimization. It was developed as part of the
/// "Writing a Loop Optimization" tutorial, presented at LLVM Devepeloper's
/// Conference, 2019.
//===----------------------------------------------------------------------===

#include "llvm/Transforms/Scalar/LoopOptTutorial.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "loop-opt-tutorial"

bool LoopSplit::run(Loop &L) const {

  LLVM_DEBUG(dbgs() << "Entering " << __func__ << "\n");

  if (!isCandidate(L)) {
    LLVM_DEBUG(dbgs() << "Loop " << L.getName()
                      << " is not a candidate for splitting.\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "Loop " << L.getName()
                    << " is a candidate for splitting!\n");

  return false;
}

bool LoopSplit::isCandidate(const Loop &L) const {
  // Require loops with preheaders and dedicated exits.
  if (!L.isLoopSimplifyForm())
    return false;

  // Since we use cloning to split the loop, it has to be safe to clone.
  if (!L.isSafeToClone())
    return false;

  // If the loop has multiple exiting blocks, do not split.
  if (!L.getExitingBlock())
    return false;

  // If loop has multiple exit blocks, do not split.
  if (!L.getExitBlock())
    return false;

  // Only split innermost loops. Thus, if the loop has any children, it cannot
  // be split.
  if (!L.getSubLoops().empty())
    return false;

  return true;
}

PreservedAnalyses LoopOptTutorialPass::run(Loop &L, LoopAnalysisManager &LAM,
                                           LoopStandardAnalysisResults &AR,
                                           LPMUpdater &U) {
  LLVM_DEBUG(dbgs() << "Entering LoopOptTutorialPass::run\n");
  LLVM_DEBUG(dbgs() << "Loop: "; L.dump(); dbgs() << "\n");

  bool Changed = LoopSplit(AR.LI).run(L);

  if (!Changed)
    return PreservedAnalyses::all();

  return llvm::getLoopPassPreservedAnalyses();
}

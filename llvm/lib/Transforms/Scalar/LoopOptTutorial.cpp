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
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;

#define DEBUG_TYPE "loop-opt-tutorial"

//===----------------------------------------------------------------------===//
// LoopSplit implementation
//

bool LoopSplit::run(Loop &L) const {
  return splitLoop(L);
}

bool LoopSplit::splitLoop(Loop &L) const {
  assert(L.isLoopSimplifyForm() && "Expecting a loop in simplify form");
  assert(L.isSafeToClone() && "Loop is not safe to be cloned");

  // Clone the original loop.

  BasicBlock *Preheader = L.getLoopPreheader();
  BasicBlock *Pred = Preheader;
  //Preheader = SplitBlock(Preheader, Preheader->getTerminator(), &DT);

  ValueToValueMapTy VMap;
  LLVM_DEBUG(dbgs() << "InsertPoint: " << Preheader->getName() << "\n");
  Loop *ClonedLoop = cloneLoop(L, *Preheader, *Pred, VMap);
  LLVM_DEBUG(dbgs() << "Created " << ClonedLoop << ":" << *ClonedLoop << "\n");

  Preheader = ClonedLoop->getLoopPreheader();

  return true;
}

Loop *LoopSplit::cloneLoop(Loop &L, BasicBlock &Preheader, BasicBlock &Pred,
                           ValueToValueMapTy &VMap) const {
  assert(L.getSubLoops().empty() && "Expecting a innermost loop");

  BasicBlock *ExitBlock = L.getExitBlock();
  assert(ExitBlock && "Expecting outermost loop to have a valid exit block");

  // Clone the original loop and remap instructions in the cloned loop.
  SmallVector<BasicBlock *, 4> ClonedLoopBlocks;
  DominatorTree *DT = nullptr;
  Loop *NewLoop = cloneLoopWithPreheader(&Preheader, &Pred, &L,
                                         VMap, "", &LI, DT, ClonedLoopBlocks);
  VMap[ExitBlock] = &Preheader;
  remapInstructionsInBlocks(ClonedLoopBlocks, VMap);
  Pred.getTerminator()->replaceUsesOfWith(&Preheader,
                                          NewLoop->getLoopPreheader());

  // Update the immediate dominator for the origianl loop with the exiting block
  // of the new loop created. Dominance within the loop is updated in
  // cloneLoopWithPreheader.
  //DT.changeImmediateDominator(&Preheader, NewLoop->getExitingBlock());

  return NewLoop;
}

PreservedAnalyses LoopOptTutorialPass::run(Loop &L, LoopAnalysisManager &LAM,
                                           LoopStandardAnalysisResults &AR,
                                           LPMUpdater &U) {
  bool Changed = LoopSplit(AR.LI).run(L);

  LLVM_DEBUG(dbgs() << "Entering LoopOptTutorialPass::run\n");
  LLVM_DEBUG(dbgs() << "Loop: "; L.dump(); dbgs() << "\n");
  if (!Changed)
    return PreservedAnalyses::all();

  return llvm::getLoopPassPreservedAnalyses();
}

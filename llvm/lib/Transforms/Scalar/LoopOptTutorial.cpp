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
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

#define DEBUG_TYPE "loop-opt-tutorial"
static const char *VerboseDebug = DEBUG_TYPE "-verbose";

/// Clones a loop \p OrigLoop.  Returns the loop and the blocks in \p
/// Blocks.
/// Updates LoopInfo assuming the loop is dominated by block \p LoopDomBB.
/// Insert the new blocks before block specified in \p Before.
static Loop *myCloneLoopWithPreheader(BasicBlock *Before, BasicBlock *LoopDomBB,
                                      Loop *OrigLoop, ValueToValueMapTy &VMap,
                                      const Twine &NameSuffix, LoopInfo *LI,
                                      SmallVectorImpl<BasicBlock *> &Blocks);

//===----------------------------------------------------------------------===//
// LoopSplit implementation
//

bool LoopSplit::run(Loop &L) const {
  // TODO:  filter non-candidates & diagnose them

  return splitLoopInHalf(L);
}

bool LoopSplit::splitLoopInHalf(Loop &L) const {
  assert(L.isLoopSimplifyForm() && "Expecting a loop in simplify form");
  assert(L.isSafeToClone() && "Loop is not safe to be cloned");
  assert(L.getSubLoops().empty() && "Expecting a innermost loop");

  // Split the loop preheader to create an insertion point for the cloned loop.
  BasicBlock *Preheader = L.getLoopPreheader();
  BasicBlock *Pred = Preheader;
  DEBUG_WITH_TYPE(VerboseDebug,
                  dumpLoopFunction("Before splitting preheader:\n", L););
  BasicBlock *InsertBefore = SplitBlock(Preheader, Preheader->getTerminator(), DT);
  DEBUG_WITH_TYPE(VerboseDebug,
                  dumpLoopFunction("After splitting preheader:\n", L););

  // Clone the original loop.
  Loop *ClonedLoop = cloneLoop(L, *InsertBefore, *Pred);
  DEBUG_WITH_TYPE(VerboseDebug,
                  dumpLoopFunction("After cloning the loop:\n", L););

  return true;
}

Loop *LoopSplit::cloneLoop(Loop &L, BasicBlock &InsertBefore, BasicBlock &Pred) const {
  // Clone the original loop, insert the clone before the "InsertBefore" BB.
  SmallVector<BasicBlock *, 4> ClonedLoopBlocks;
  ValueToValueMapTy VMap;
  Loop *NewLoop = myCloneLoopWithPreheader(&InsertBefore, &Pred, &L, VMap,
                                           "ClonedLoop", &LI, ClonedLoopBlocks);
  assert(NewLoop && "Run ot of memory");
  DEBUG_WITH_TYPE(VerboseDebug,
                  dbgs() << "Create new loop: " << NewLoop->getName() << "\n";
                  dumpLoopFunction("After cloning loop:\n", L););

  // Update instructions referencing the original loop basic blocks to
  // reference the corresponding block in the cloned loop.
  VMap[L.getExitBlock()] = &InsertBefore;
  remapInstructionsInBlocks(ClonedLoopBlocks, VMap);
  DEBUG_WITH_TYPE(
      VerboseDebug, dumpLoopFunction("After instruction remapping:\n", L););

  // Make the predecessor of original loop jump to the cloned loop.
  Pred.getTerminator()->replaceUsesOfWith(&InsertBefore,
                                          NewLoop->getLoopPreheader());

  // Update the immediate dominator for the origianl loop with the exiting block
  // of the new loop created. Dominance within the loop is updated in
  // cloneLoopWithPreheader.
  //DT.changeImmediateDominator(&Preheader, NewLoop->getExitingBlock());

  return NewLoop;
}

void LoopSplit::dumpLoopFunction(const StringRef Msg, const Loop &L) const {
  const Function &F = *L.getHeader()->getParent();
  dbgs() << Msg;
  F.dump();
}

/// Clones a loop \p OrigLoop.  Returns the loop and the blocks in \p
/// Blocks.
/// Updates LoopInfo assuming the loop is dominated by block \p LoopDomBB.
/// Insert the new blocks before block specified in \p Before.
static Loop *myCloneLoopWithPreheader(BasicBlock *Before, BasicBlock *LoopDomBB,
                                      Loop *OrigLoop, ValueToValueMapTy &VMap,
                                      const Twine &NameSuffix, LoopInfo *LI,
                                      SmallVectorImpl<BasicBlock *> &Blocks) {
  Function *F = OrigLoop->getHeader()->getParent();
  Loop *ParentLoop = OrigLoop->getParentLoop();
  DenseMap<Loop *, Loop *> LMap;

  Loop *NewLoop = LI->AllocateLoop();
  LMap[OrigLoop] = NewLoop;
  if (ParentLoop)
    ParentLoop->addChildLoop(NewLoop);
  else
    LI->addTopLevelLoop(NewLoop);

  BasicBlock *OrigPH = OrigLoop->getLoopPreheader();
  assert(OrigPH && "No preheader");
  BasicBlock *NewPH = CloneBasicBlock(OrigPH, VMap, NameSuffix, F);
  // To rename the loop PHIs.
  VMap[OrigPH] = NewPH;
  Blocks.push_back(NewPH);

  // Update LoopInfo.
  if (ParentLoop)
    ParentLoop->addBasicBlockToLoop(NewPH, *LI);

  for (Loop *CurLoop : OrigLoop->getLoopsInPreorder()) {
    Loop *&NewLoop = LMap[CurLoop];
    if (!NewLoop) {
      NewLoop = LI->AllocateLoop();

      // Establish the parent/child relationship.
      Loop *OrigParent = CurLoop->getParentLoop();
      assert(OrigParent && "Could not find the original parent loop");
      Loop *NewParentLoop = LMap[OrigParent];
      assert(NewParentLoop && "Could not find the new parent loop");

      NewParentLoop->addChildLoop(NewLoop);
    }
  }

  for (BasicBlock *BB : OrigLoop->getBlocks()) {
    Loop *CurLoop = LI->getLoopFor(BB);
    Loop *&NewLoop = LMap[CurLoop];
    assert(NewLoop && "Expecting new loop to be allocated");

    BasicBlock *NewBB = CloneBasicBlock(BB, VMap, NameSuffix, F);
    VMap[BB] = NewBB;

    // Update LoopInfo.
    NewLoop->addBasicBlockToLoop(NewBB, *LI);
    if (BB == CurLoop->getHeader())
      NewLoop->moveToHeader(NewBB);

    Blocks.push_back(NewBB);
  }

  // Move them physically from the end of the block list.
  F->getBasicBlockList().splice(Before->getIterator(), F->getBasicBlockList(),
                                NewPH);
  F->getBasicBlockList().splice(Before->getIterator(), F->getBasicBlockList(),
                                NewLoop->getHeader()->getIterator(), F->end());

  return NewLoop;
}

//===----------------------------------------------------------------------===//
// LoopOptTutorialPass implementation
//

PreservedAnalyses LoopOptTutorialPass::run(Loop &L, LoopAnalysisManager &LAM,
                                           LoopStandardAnalysisResults &AR,
                                           LPMUpdater &U) {
  LLVM_DEBUG(dbgs() << "Entering LoopOptTutorialPass::run\n");
  LLVM_DEBUG(dbgs() << "Loop: "; L.dump(); dbgs() << "\n");

  bool Changed = LoopSplit(AR.LI, AR.SE).run(L);

  if (!Changed)
    return PreservedAnalyses::all();

  return llvm::getLoopPassPreservedAnalyses();
}

//===-------- LoopOptTutorial.h - Loop Opt Tutorial Pass ------*- C++ -*-===//
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

#ifndef LLVM_TRANSFORMS_SCALAR_LOOPOPTTUTORIAL_H
#define LLVM_TRANSFORMS_SCALAR_LOOPOPTTUTORIAL_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Analysis/LoopInfo.h"

namespace llvm {

class Loop;
class LPMUpdater;

/// This class splits the innermost loop in a loop nest in the middle.
class LoopSplit {
public:
  LoopSplit(LoopInfo &LI, ScalarEvolution &SE) : LI(LI), SE(SE) {}

  // Excute the transformation on the loop nest rooted by \p L.
  bool run(Loop &L) const;

private:
  /// Split the given loop in the middle by creating a new loop that traverse
  /// the first half of the original iteration space and adjusting the loop
  /// bounds of \p L to traverse the remaining half.
  /// Note: \p L is expected to be the innermost loop in a loop nest or a top
  /// level loop.
  bool splitLoopInHalf(Loop &L) const;

  /// Clone loop \p L and insert the cloned loop before the basic block \p
  /// InsertBefore, \p Pred is the predecessor of \p L.
  /// Note: \p L is expected to be the innermost loop in a loop nest or a top
  /// level loop.
  Loop *cloneLoop(Loop &L, BasicBlock &InsertBefore, BasicBlock &Pred) const;

  // Dump the LLVM IR for function containing the given loop \p L.
  void dumpLoopFunction(const StringRef Msg, const Loop &L) const;

 private:
  LoopInfo &LI;
  ScalarEvolution &SE;
  DominatorTree *DT = nullptr;
};

class LoopOptTutorialPass : public PassInfoMixin<LoopOptTutorialPass> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR, LPMUpdater &U);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_LOOPOPTTUTORIAL_H

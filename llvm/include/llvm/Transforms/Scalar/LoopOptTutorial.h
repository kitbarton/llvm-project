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
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

namespace llvm {

class Loop;
class LPMUpdater;
class Statistic;
class OptimizationRemarkEmitter;

/// This class splits the innermost loop in a loop nest in the middle.
class LoopSplit {
public:
  LoopSplit(LoopInfo &LI, ScalarEvolution &SE, DominatorTree &DT,
            OptimizationRemarkEmitter &ORE)
      : LI(LI), SE(SE), DT(DT), ORE(ORE) {}

  /// Execute the transformation on the loop nest rooted by \p L.
  bool run(Loop &L) const;

private:
  /// Determines if \p L is a candidate for splitting.
  bool isCandidate(const Loop &L) const;

  /// Split the given loop in the middle by creating a new loop that traverse
  /// the first half of the original iteration space and adjusting the loop
  /// bounds of \p L to traverse the remaining half.
  /// Note: \p L is expected to be the innermost loop in a loop nest or a top
  /// level loop.
  bool splitLoop(Loop &L) const;

  /// Clone loop \p L and insert the cloned loop before the basic block \p
  /// InsertBefore, \p Pred is the predecessor of \p L.
  /// Note: \p L is expected to be the innermost loop in a loop nest or a top
  /// level loop.
  Loop *cloneLoop(Loop &L, BasicBlock &InsertBefore, BasicBlock &Pred) const;

  /// Compute the point where to split the loop \p L. Return the instruction
  /// calculating the split point.
  Instruction *computeSplitPoint(const Loop &L,
                                 Instruction *InsertBefore) const;

  /// Get the latch comparison instruction of loop \p L.
  ICmpInst *getLatchCmpInst(const Loop &L) const;

  /// Update the dominator tree after cloning the loop.
  void updateDominatorTree(const Loop &OrigLoop, const Loop &ClonedLoop,
                           BasicBlock &InsertBefore, BasicBlock &Pred,
                           ValueToValueMapTy &VMap) const;

  /// Report that loop \p L that is not a candidate for splitting.
  bool reportInvalidCandidate(const Loop &L, Statistic &Stat) const;

  /// Report that loop \p L was successfully split.
  void reportSuccess(const Loop &L, Statistic &Stat) const;

  /// Report that loop \p L was not successfully split.
  void reportFailure(const Loop &L, Statistic &Stat) const;

  // Dump the LLVM IR for function \p F.
  void dumpFunction(const StringRef Msg, const Function &F) const;

private:
  LoopInfo &LI;
  ScalarEvolution &SE;
  DominatorTree &DT;
  OptimizationRemarkEmitter &ORE;
};

class LoopOptTutorialPass : public PassInfoMixin<LoopOptTutorialPass> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR, LPMUpdater &U);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_LOOPOPTTUTORIAL_H

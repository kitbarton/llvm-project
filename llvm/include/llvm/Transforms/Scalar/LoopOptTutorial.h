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
  LoopSplit(LoopInfo &LI) : LI(LI) {}

  bool run(Loop &L) const;

private:
  ///
  bool splitLoop(Loop &L) const;

  /// Clone the loop rooted by \p L.
  Loop *cloneLoop(Loop &L, BasicBlock &Preheader, BasicBlock &Pred,
                  ValueToValueMapTy &VMap) const;

 private:
  LoopInfo &LI;
};

class LoopOptTutorialPass : public PassInfoMixin<LoopOptTutorialPass> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR, LPMUpdater &U);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_LOOPOPTTUTORIAL_H

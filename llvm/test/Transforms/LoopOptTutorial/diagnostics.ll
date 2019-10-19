; RUN: opt -disable-output -passes='require<opt-remark-emit>,loop(rotate,loop-opt-tutorial)' -pass-remarks-analysis=loop-opt-tutorial < %s 2>&1 | FileCheck %s

; CHECK: remark: diagnostic.c:10:10: [multiple_exits]: Loop is not a candidate for splitting: Loop doesn't have a unique exiting block
define void @multiple_exits(i32* noalias %arg) {
entry:
  br label %header

header:
  %i = phi i64 [ %inci, %latch ], [ 0, %entry ]
  %exitcond4 = icmp ne i64 %i, 100
  br i1 %exitcond4, label %body, label %exit

body:
  %tmp = add nsw i64 %i, -3
  %tmp8 = add nuw nsw i64 %i, 3
  %tmp10 = mul nsw i64 %tmp, %tmp8
  %tmp12 = srem i64 %tmp10, %i
  %tmp13 = getelementptr inbounds i32, i32* %arg, i64 %i
  %tmp14 = trunc i64 %tmp12 to i32
  store i32 %tmp14, i32* %tmp13, align 4
  %earlyexitcond = icmp eq i32 %tmp14, 70
  br i1 %exitcond4, label %exit, label %latch, !dbg !8

latch:
  %inci = add nuw nsw i64 %i, 1
  br label %header

exit:
  ret void
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "diagnostic.c", directory: "/tmp")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"PIC Level", i32 2}
!5 = !{!"clang version 9.0.0 "}
!6 = distinct !DISubprogram(name: "", scope: !1, file: !1, line: 1, type: !7, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !2)
!7 = !DISubroutineType(types: !2)
!8 = !DILocation(line: 10, column: 10, scope: !6)

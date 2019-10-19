; RUN: opt -S -passes='require<opt-remark-emit>,loop(rotate,loop-opt-tutorial)' < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -passes='require<opt-remark-emit>,loop(rotate,loop-opt-tutorial)' -pass-remarks=loop-opt-tutorial < %s 2>&1 | FileCheck --check-prefix=REMARKS %s
; REMARKS: remark: simple.c:10:10: [dep_free]: Loop has been splitted
; REQUIRES: asserts
; CHECK-LABEL: dep_free
; CHECK-LABEL: entry:
; CHECK-NEXT:   [[SUB:%[0-9]*]] = sub i64 100, 0
; CHECK-NEXT:   [[SPLIT:%[0-9]*]] = udiv i64 [[SUB]], 2
; CHECK-NEXT:   br label %[[L1_PREHEADER:.*]]
; First loop:
;    for (long i = 0; i < 100/2; ++i)
;        ...
; CHECK:      [[L1_PREHEADER]]:
; CHECK-NEXT:   br label %[[L1_HEADER:.*]]
; CHECK:      [[L1_HEADER]]:
; CHECK-NEXT:   [[L1_I:%i[0-9]*]] = phi i64 [ 0, %[[L1_PREHEADER]] ], [ [[L1_INCI:%inci[0-9]*]], %[[L1_LATCH:.*]] ]
; CHECK:      br label %[[L1_LATCH]]
; CHECK:      [[L1_LATCH]]:
; CHECK-NEXT:   [[L1_INCI]] = add nuw nsw i64 [[L1_I]], 1
; CHECK-NEXT:   [[L1_CMP:%exitcond[0-9]*]] = icmp ne i64 [[L1_INCI]], [[SPLIT]]
; CHECK-NEXT:   br i1 [[L1_CMP]], label %[[L1_HEADER]], label %[[L2_PREHEADER:.*]]

; Second loop:
;    for (long i = 100/2; i < 100; ++i)
;        ...
; CHECK:      [[L2_PREHEADER]]:
; CHECK-NEXT:   br label %[[L2_HEADER:.*]]
; CHECK:      [[L2_HEADER]]:
; CHECK-NEXT:   [[L2_I:%i[0-9]*]] = phi i64 [ [[SPLIT]], %[[L2_PREHEADER]] ], [ [[L2_INCI:%inci[0-9]*]], %[[L2_LATCH:.*]] ]
; CHECK:      br label %[[L2_LATCH]]
; CHECK:      [[L2_LATCH]]:
; CHECK-NEXT:   [[L2_INCI]] = add nuw nsw i64 [[L2_I]], 1
; CHECK-NEXT:   [[L2_CMP:%exitcond[0-9]*]] = icmp ne i64 [[L2_INCI]], 100
; CHECK-NEXT:   br i1 [[L2_CMP]], label %[[L2_HEADER]], label %[[EXIT:.*]]
; CHECK:      [[EXIT]]:

define void @dep_free(i32* noalias %arg) {
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
  br label %latch, !dbg !8


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
!1 = !DIFile(filename: "simple.c", directory: "/tmp")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"PIC Level", i32 2}
!5 = !{!"clang version 9.0.0 "}
!6 = distinct !DISubprogram(name: "", scope: !1, file: !1, line: 1, type: !7, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !2)
!7 = !DISubroutineType(types: !2)
!8 = !DILocation(line: 10, column: 10, scope: !6)

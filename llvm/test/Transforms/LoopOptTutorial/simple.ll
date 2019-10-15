; RUN: opt -S -passes='loop(rotate,loop-opt-tutorial)' < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; CHECK-LABEL: dep_free
; CHECK-LABEL: entry:
; First loop:
;    for (long i = 0; i < 100/2; ++i)
;        ...
; CHECK-NEXT:   br label %[[L1_PREHEADER:.*]]
; CHECK:      [[L1_PREHEADER]]:
; CHECK-NEXT:   [[SUB:%[0-9]*]] = sub i64 100, 0
; CHECK-NEXT:   [[SPLIT:%[0-9]*]] = udiv i64 [[SUB]], 2
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
  br label %latch

latch:                                            
  %inci = add nuw nsw i64 %i, 1
  br label %header

exit:                                            
  ret void
}

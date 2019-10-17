; RUN: opt -S -passes='require<opt-remark-emit>,loop(rotate,loop-opt-tutorial)' < %s 2>&1 | FileCheck %s
;
; Opts: -correlated-propagation -mem2reg -instcombine -loop-simplify -indvars -instnamer
;
; Source:
;   int A[1024][1024];
;   int B[1024][1024];
;
;   #define EXPENSIVE_PURE_COMPUTATION(i) ((i - 3) * (i + 3) % i)
;
;   void dep_free() {
;     for (int i = 0; i < 100; i++)
;       for (int j = 0; j < 100; j++)
;         A[i][j] = EXPENSIVE_PURE_COMPUTATION(i);
;   }
;
@A = common dso_local global [1024 x [1024 x i32]] zeroinitializer, align 4
@B = common dso_local global [1024 x [1024 x i32]] zeroinitializer, align 4

; CHECK-LABEL: dep_free
; CHECK-LABEL: entry:
; CHECK-NEXT:   br label %[[I_HEADER:.*]]
; CHECK:      [[I_HEADER:.*]]:
; CHECK-NEXT:   [[I:%i[0-9]*]] = phi i64 [ 0, %entry ], [ [[INCI:%inci[0-9]*]], %[[I_LATCH:.*]] ]
; CHECK-NEXT:   [[SUB:%[0-9]*]] = sub i64 100, 0
; CHECK-NEXT:   [[SPLIT:%[0-9]*]] = udiv i64 [[SUB]], 2
; CHECK-NEXT:   br label %[[L1_PREHEADER:.*]]
; First loop:
;    for (long j = 0; j < 100/2; ++j)
;        ...
; CHECK:      [[L1_PREHEADER]]:
; CHECK-NEXT:   br label %[[L1_HEADER:.*]]
; CHECK:      [[L1_HEADER]]:
; CHECK-NEXT:   [[L1_J:%j[0-9]*]] = phi i64 [ 0, %[[L1_PREHEADER]] ], [ [[L1_INCJ:%incj[0-9]*]], %[[L1_LATCH:.*]] ]
; CHECK:      br label %[[L1_LATCH]]
; CHECK:      [[L1_LATCH]]:
; CHECK-NEXT:   [[L1_INCJ]] = add nuw nsw i64 [[L1_J]], 1
; CHECK-NEXT:   [[L1_CMP:%exitcond[0-9]*]] = icmp ne i64 [[L1_INCJ]], [[SPLIT]]
; CHECK-NEXT:   br i1 [[L1_CMP]], label %[[L1_HEADER]], label %[[L2_PREHEADER:.*]]

; Second loop:
;    for (long j = 100/2; j < 100; ++j)
;        ...
; CHECK:      [[L2_PREHEADER]]:
; CHECK-NEXT:   br label %[[L2_HEADER:.*]]
; CHECK:      [[L2_HEADER]]:
; CHECK-NEXT:   [[L2_J:%j[0-9]*]] = phi i64 [ [[SPLIT]], %[[L2_PREHEADER]] ], [ [[L2_INCJ:%incj[0-9]*]], %[[L2_LATCH:.*]] ]
; CHECK:      br label %[[L2_LATCH]]
; CHECK:      [[L2_LATCH]]:
; CHECK-NEXT:   [[L2_INCJ]] = add nuw nsw i64 [[L2_J]], 1
; CHECK-NEXT:   [[L2_CMP:%exitcond[0-9]*]] = icmp ne i64 [[L2_INCJ]], 100
; CHECK-NEXT:   br i1 [[L2_CMP]], label %[[L2_HEADER]], label %[[I_LATCH:.*]]

; CHECK:      [[I_LATCH]]:
; CHECK-NEXT:   [[INCI]] = add nuw nsw i64 [[I]], 1
; CHECK-NEXT:   [[I_CMP:%exitcond[0-9]*]] = icmp ne i64 [[INCI]], 100
; CHECK-NEXT:   br i1 [[I_CMP]], label %[[I_HEADER]], label %[[EXIT:.*]]

; CHECK:      [[EXIT]]:

define void @dep_free() {
entry:
  br label %i_header

i_header:
  %i = phi i64 [ %inci, %i_latch ], [ 0, %entry ]
  %exitcond5 = icmp ne i64 %i, 100
  br i1 %exitcond5, label %j_header, label %exit

j_header:       
  %j = phi i64 [ %incj, %j_latch ], [ 0, %i_header ]
  %exitcond = icmp ne i64 %j, 100
  br i1 %exitcond, label %j_body, label %i_latch

j_body:        
  %sub = add nsw i64 %i, -3
  %tmp = add nuw nsw i64 %i, 3
  %mul = mul nsw i64 %sub, %tmp
  %rem = srem i64 %mul, %i
  %arrayidx6 = getelementptr inbounds [1024 x [1024 x i32]], [1024 x [1024 x i32]]* @A, i64 0, i64 %i, i64 %j
  %tmp7 = trunc i64 %sub to i32
  store i32 %tmp7, i32* %arrayidx6, align 4, !tbaa !2
  br label %j_latch

j_latch:
  %incj = add nuw nsw i64 %j, 1
  br label %j_header

i_latch:         
  %inci = add nuw nsw i64 %i, 1
  br label %i_header

exit:         
  ret void
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0 (https://github.com/kitbarton/llvm-project.git 8ec1040ded5afb786efc933363420571aa6ec5ea)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

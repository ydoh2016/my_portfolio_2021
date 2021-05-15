define i32 @f(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f) {
; CHECK-LABEL: @f(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f)
; CHECK-NEXT:    [[COND1:%.*]] = icmp eq i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    br i1 [[COND1]], label [[BB_TRUE1:%.*]], label [[BB_ELSE:%.*]]
; CHECK:       bb_true1:
; CHECK-NEXT:    [[COND2:%.*]] = icmp eq i32 [[A]], [[C:%.*]]
; CHECK-NEXT:    br i1 [[COND2]], label [[BB_TRUE2:%.*]], label [[BB_ELSE:%.*]]
; CHECK:       bb_true2:
; CHECK-NEXT:    [[COND3:%.*]] = icmp eq i32 [[A]], [[D:%.*]]
; CHECK-NEXT:    br i1 [[COND3]], label [[BB_TRUE3:%.*]], label [[BB_ELSE:%.*]]
; CHECK:       bb_true3:
; CHECK-NEXT:    [[COND4:%.*]] = icmp eq i32 [[A]], [[E:%.*]]
; CHECK-NEXT:    br i1 [[COND4]], label [[BB_TRUE4:%.*]], label [[BB_ELSE:%.*]]
; CHECK:       bb_true4:
; CHECK-NEXT:    [[COND5:%.*]] = icmp eq i32 [[A]], [[F:%.*]]
; CHECK-NEXT:    br i1 [[COND5]], label [[BB_TRUE5:%.*]], label [[BB_ELSE:%.*]]
; CHECK:       bb_true5:
; CHECK-NEXT:    ret i32 [[A]]
; CHECK:       bb_else:
; CHECK-NEXT:    ret i32 [[F]]
;
  %cond1 = icmp eq i32 %a, %b
  br i1 %cond1, label %bb_true1, label %bb_else
bb_true1:
  %cond2 = icmp eq i32 %b, %c
  br i1 %cond2, label %bb_true2, label %bb_else
bb_true2:
  %cond3 = icmp eq i32 %c, %d
  br i1 %cond3, label %bb_true3, label %bb_else
bb_true3:
  %cond4 = icmp eq i32 %d, %e
  br i1 %cond4, label %bb_true4, label %bb_else
bb_true4:
  %cond5 = icmp eq i32 %e, %f
  br i1 %cond5, label %bb_true5, label %bb_else
bb_true5:
  ret i32 %f
bb_else:
  ret i32 %f
}

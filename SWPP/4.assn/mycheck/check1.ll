define i32 @f(i32 %a, i32 %b) {
; CHECK-LABEL: @f(i32 %a, i32 %b)
; CHECK-NEXT:    [[COND:%.*]] = icmp eq i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    br i1 [[COND]], label [[BB_ELSE:%.*]], label [[BB_TRUE:%.*]]
; CHECK:       bb_true:
; CHECK-NEXT:    ret i32 [[B]]
; CHECK:       bb_else:
; CHECK-NEXT:    ret i32 [[A]]
;
  %cond = icmp eq i32 %a, %b
  br i1 %cond, label %bb_else, label %bb_true
bb_true:
  ret i32 %b
bb_else:
  ret i32 %b
}

define i32 @f(i32 %x, i32 %y) {
; CHECK-LABEL: @f(i32 %x, i32 %y)
; CHECK-NEXT:    [[XP:%.*]] = alloca i32
; CHECK-NEXT:    store i32 [[X:%.*]], i32* [[XP]]
; CHECK-NEXT:    [[YP:%.*]] = alloca i32
; CHECK-NEXT:    store i32 [[Y:%.*]], i32* [[YP]]
; CHECK-NEXT:    [[A:%.*]] = load i32, i32* [[YP]]
; CHECK-NEXT:    [[B:%.*]] = load i32, i32* [[XP]]
; CHECK-NEXT:    [[COND:%.*]] = icmp eq i32 [[A]], [[B]]
; CHECK-NEXT:    br i1 [[COND]], label [[BB_TRUE:%.*]], label [[BB_FALSE:%.*]]
; CHECK:       bb_true:
; CHECK-NEXT:    ret i32 [[A]]
; CHECK:       bb_false:
; CHECK-NEXT:    ret i32 [[B]]
;
  %xp = alloca i32
  store i32 %x, i32* %xp
  %yp = alloca i32
  store i32 %y, i32* %yp
  %a = load i32, i32* %yp
  %b = load i32, i32* %xp
  %cond = icmp eq i32 %a, %b
  br i1 %cond, label %bb_true, label %bb_false
bb_true:
  ret i32 %b
bb_false:
  ret i32 %b
}

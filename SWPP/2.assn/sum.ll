define double @sum(double* %input, i32 %n) {
  %1 = alloca double*
  %2 = alloca i32
  %3 = alloca double
  %4 = alloca double
  %5 = alloca double
  %6 = alloca i32
  store double* %input, double** %1
  store i32 %n, i32* %2
  store double 0.000000e+00, double* %3
  store double 0.000000e+00, double* %4
  store double 1.000000e+15, double* %5
  store i32 0, i32* %6
  br label %loop
loop:
  %7 = load i32, i32* %6
  %8 = load i32, i32* %2
  %9 = icmp slt i32 %7, %8
  br i1 %9, label %continue, label %break
continue:
  %10 = load double*, double** %1
  %11 = load i32, i32* %6
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds double, double* %10, i64 %12
  %14 = load double, double* %13
  %15 = load double, double* %5
  %16 = fcmp olt double %14, %15
  br i1 %16, label %small1, label %big
small1:
  %17 = load double*, double** %1
  %18 = load i32, i32* %6
  %19 = sext i32 %18 to i64
  %20 = getelementptr inbounds double, double* %17, i64 %19
  %21 = load double, double* %20
  %22 = load double, double* %5
  %23 = fneg double %22
  %24 = fcmp ogt double %21, %23
  br i1 %24, label %small2, label %big
small2:
  %25 = load double*, double** %1
  %26 = load i32, i32* %6
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds double, double* %25, i64 %27
  %29 = load double, double* %28
  %30 = load double, double* %4
  %31 = fadd double %30, %29
  store double %31, double* %4
  br label %loopcheck
big:
  %32 = load double*, double** %1
  %33 = load i32, i32* %6
  %34 = sext i32 %33 to i64
  %35 = getelementptr inbounds double, double* %32, i64 %34
  %36 = load double, double* %35
  %37 = load double, double* %3
  %38 = fadd double %37, %36
  store double %38, double* %3
  br label %loopcheck
loopcheck:
  %39 = load i32, i32* %6
  %40 = add nsw i32 %39, 1
  store i32 %40, i32* %6
  br label %loop
break:
  %41 = load double, double* %3, align 8
  %42 = load double, double* %4, align 8
  %43 = fadd double %41, %42
  ret double %43
}

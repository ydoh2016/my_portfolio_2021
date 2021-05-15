define void @add(i32* %ptr1, i32* %ptr2, i32* %val) {
  %1 = alloca i32*
  %2 = alloca i32*
  store i32* %ptr1, i32** %1
  store i32* %ptr2, i32** %2
  %3 = load i32, i32* %val
  %4 = load i32*, i32** %1
  %5 = load i32, i32* %4
  %6 = add i32 %3, %5
  store i32 %6, i32* %4
  %7 = load i32, i32* %val
  %8 = load i32*, i32** %2
  %9 = load i32, i32* %8
  %10 = add i32 %7, %9
  store i32 %10, i32* %8
  ret void
}

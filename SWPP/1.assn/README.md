## SWPP, Assignment 1: Rebasing & Merging Branches

What you should do is to:

0. Clone this repo into your computer

1. Make a new commit that replaces `aqjune` from `hello.cpp` with your github id,
so it prints:

```
make
./hello
Hello, my name is <your Github ID>
```

The commit message should be "Update hello.cpp".

2. This repo has three branches: `master`, `bugfix`, `usemap`.
Checkout the existing `bugfix` branch using `git checkout` command.
Then, **rebase** it onto `master` using `git rebase` command.
See following diagram.

```
*----------* master
 \
  \
   --------* bugfix

==> (after rebase)

*----------* master
            \
             \
              --------* bugfix
```

3. Checkout `master` and merge `bugfix` into `master` using `git merge` command.
Thanks to the rebase done before, there will be no merge commit created.

3. On top of that, **merge** the existing `usemap` branch again. This causes a merge conflict;
please fix it carefully.

NOTE: You should carefully see why it causes merge conflict. To do this, you
need to understand what `usemap` branch did.

4. Check that `make; ./check.sh` works successfully. :)

After this, `git log --oneline` at `master` branch should show the commits
including bugfix/usemap commits, "Update hello.cpp" commit,
and a merge commit.

`git log --online --no-merges` should hide the merge commits.

5. `./package.sh` will create an archive `submit.tar.gz`. Submit this archive to eTL.

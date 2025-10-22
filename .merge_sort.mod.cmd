savedcmd_merge_sort.mod := printf '%s\n'   merge_sort.o | awk '!x[$$0]++ { print("./"$$0) }' > merge_sort.mod

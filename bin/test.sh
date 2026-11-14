#!/bin/bash
# Initial Source - https://stackoverflow.com/a/10118989
diff="diff -iad"

# Loop the tests
for file_in in tests/*.in; do
    file_base="${file_in%.in}"

    # Optional argument to only run one test
    if [[ -n "$1" && "tests/$1" != "$file_base" ]]; then
        continue;
    fi

    file_out_val="$file_base.out" # Stores success case
    file_out_bf_tst="$file_base.out.bf" # Updated with compiled bf
    file_out_bf_tst_c="$file_base.out.c" # The BF program converted to C
    file_out_bf_test_c_compiled="$file_base" # The comiled C program
    file_out_tst="$file_base.out.tst" # Updated with ouput of compiled bf

    # Validate infile exists (do the same for out validate file)
    if [ ! -f "$file_in" ]; then
        echo
        printf "In file %s is missing\n" "$file_in"
        continue;
    fi
    if [ ! -f "$file_out_val" ]; then
        printf "Validation file %s is missing\n" "$file_out_val"
        continue;
    fi

    printf "Testing against %s\n" "$file_in"

    # Run the compile pipeline
    "./bf-lang" "$file_in" > "$file_out_bf_tst"
    "./bin/bf" -c "$file_out_bf_tst"
    gcc -o "$file_out_bf_test_c_compiled" "$file_out_bf_tst_c"
    "./$file_out_bf_test_c_compiled" > "$file_out_tst"

    # Execute diff
    $diff "$file_out_tst" "$file_out_val"


    # Check exit code from previous command (ie diff)
    # We need to add this to a variable else we can't print it
    # as it will be changed by the if [
    # Iff not 0 then the files differ (at least with diff)
    e_code=$?
    if [ $e_code != 0 ]; then
            printf "TEST FAIL : %d\n" "$e_code"
    else
            printf "TEST OK!\n"
    fi
done

# Clean exit with status 0
exit 0

#!/usr/bin/env bats

# File: student_tests.sh
#
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF
ls
EOF

    [ "$status" -eq 0 ]
}

@test "Simple pipe: ls | grep .c" {
    run ./dsh <<EOF
ls | grep ".c"
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "dshlib.c" ]]
}

@test "Multiple pipes: ls | grep .c | wc -l" {
    run ./dsh <<EOF
ls | grep ".c" | wc -l
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ [0-9]+ ]]
}

@test "Pipe with built-in command should fail gracefully" {
    run ./dsh <<EOF
cd | ls
EOF

    [ "$status" -eq 0 ]
}

@test "Pipe with non-existent command should fail gracefully" {
    run ./dsh <<EOF
ls | nonexistentcommand
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "execvp" ]] || [[ "$output" =~ "command" ]]
}

@test "Pipe with spaces around pipe symbol" {
    run ./dsh <<EOF
ls    |    grep ".c"
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "dshlib.c" ]]
}

@test "Three-command pipe: ls | grep .c | sort" {
    run ./dsh <<EOF
ls | grep ".c" | sort
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "dshlib.c" ]]
}

@test "Four-command pipe: ls | grep .c | sort | head -1" {
    run ./dsh <<EOF
ls | grep ".c" | sort | head -1
EOF

    [ "$status" -eq 0 ]
}

@test "Pipe with quoted arguments: echo 'hello world' | grep 'hello'" {
    run ./dsh <<EOF
echo "hello world" | grep "hello"
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "hello world" ]]
}

@test "Pipe with empty command should fail gracefully" {
    run ./dsh <<EOF
ls |
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "warning" ]] || [[ "$output" =~ "error" ]] || [[ "$output" =~ "trailing pipe" ]] || [[ "$output" =~ "empty command" ]]
}

@test "Maximum number of pipes allowed" {
    # Create a command with CMD_MAX-1 pipes (CMD_MAX commands)
    # Assuming CMD_MAX is 8 from dshlib.h
    run ./dsh <<EOF
echo test | grep test | grep test | grep test | grep test | grep test | grep test | grep test
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "test" ]]
}

@test "Exceeding maximum number of pipes should show error" {
    # Create a command with CMD_MAX pipes (CMD_MAX+1 commands)
    # Assuming CMD_MAX is 8 from dshlib.h
    run ./dsh <<EOF
echo test | grep test | grep test | grep test | grep test | grep test | grep test | grep test | grep test
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "error: piping limited to" ]]
}

@test "Pipe with command that produces a lot of output" {
    run ./dsh <<EOF
cat /etc/passwd | head -3
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "root" ]]
}

@test "Pipe with command that takes input from stdin" {
    run ./dsh <<EOF
echo "hello\nworld" | grep "world"
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "world" ]]
}

@test "Pipe preserves exit status of last command" {
    run ./dsh <<EOF
ls /nonexistent | grep test
exit
EOF

    [ "$status" -eq 0 ]
}

@test "Exit command after pipe works" {
    run ./dsh <<EOF
ls | grep .c
exit
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "exiting" ]]
}

@test "Pipe with different executables in different directories" {
    run ./dsh <<EOF
ls /bin | grep "sh"
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "sh" ]]
}

@test "Pipe with command that has multiple arguments" {
    run ./dsh <<EOF
echo "line1\nline2\nline3" | grep -n "line"
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "line" ]]
}

@test "Pipe with command that has special characters" {
    run ./dsh <<EOF
echo "test@example.com" | grep "@"
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "test@example.com" ]]
}

@test "Pipe with empty command in the middle should fail gracefully" {
    run ./dsh <<EOF
ls | | grep ".c"
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "warning" ]] || [[ "$output" =~ "error" ]] || [[ "$output" =~ "empty command" ]]
}

@test "Output redirection with > operator" {
    TEMP_FILE="$(mktemp)"

    run ./dsh <<EOF
echo "hello, class" > $TEMP_FILE
cat $TEMP_FILE
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "hello, class" ]]

    rm -f "$TEMP_FILE"
}

@test "Output redirection with >> operator (append mode)" {
    TEMP_FILE="$(mktemp)"

    run ./dsh <<EOF
echo "line 1" > $TEMP_FILE
echo "line 2" >> $TEMP_FILE
cat $TEMP_FILE
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "line 1" ]]
    [[ "$output" =~ "line 2" ]]

    rm -f "$TEMP_FILE"
}

@test "Input redirection with < operator" {
    TEMP_FILE="$(mktemp)"
    echo "test input data" > "$TEMP_FILE"

    run ./dsh <<EOF
cat < $TEMP_FILE
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "test input data" ]]

    rm -f "$TEMP_FILE"
}

@test "Combined input and output redirection" {
    INPUT_FILE="$(mktemp)"
    OUTPUT_FILE="$(mktemp)"

    echo "test input data" > "$INPUT_FILE"

    run ./dsh <<EOF
cat < $INPUT_FILE > $OUTPUT_FILE
cat $OUTPUT_FILE
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "test input data" ]]

    rm -f "$INPUT_FILE" "$OUTPUT_FILE"
}

@test "Redirection with pipe" {
    TEMP_FILE="$(mktemp)"

    run ./dsh <<EOF
echo "line with hello and world" | grep "hello" > $TEMP_FILE
cat $TEMP_FILE
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "hello" ]]

    rm -f "$TEMP_FILE"
}

@test "Multiple redirections in a pipeline" {
    INPUT_FILE="$(mktemp)"
    OUTPUT_FILE="$(mktemp)"

    echo "apple\nbanana\ncherry\ndates" > "$INPUT_FILE"

    run ./dsh <<EOF
cat < $INPUT_FILE | grep "a" > $OUTPUT_FILE
cat $OUTPUT_FILE
EOF

    [ "$status" -eq 0 ]
    [[ "$output" =~ "apple" ]]
    [[ "$output" =~ "banana" ]]
    [[ "$output" =~ "dates" ]]

    rm -f "$INPUT_FILE" "$OUTPUT_FILE"
}

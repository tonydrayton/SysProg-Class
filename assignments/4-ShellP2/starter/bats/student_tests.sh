#!/usr/bin/env bats

# File: student_tests.sh
#
# Create your unit tests suit in this file

DIR="$( cd "$( dirname "$BATS_TEST_FILENAME" )" >/dev/null 2>&1 && pwd )"

@test "Example: check ls runs without errors" {
    run "${DIR}/../dsh" <<EOF
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "cd with no arguments stays in same directory" {
    current=$(pwd)
    run "${DIR}/../dsh" <<EOF
cd
pwd
EOF
    [ "$status" -eq 0 ]
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="${current}dsh2>dsh2>dsh2>cmdloopreturned0"
    [ "$stripped_output" = "$expected_output" ]
}

@test "cd to invalid directory shows error" {
    run "${DIR}/../dsh" <<EOF
cd /nonexistent_directory
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "cd: No such file or directory" ]]
}

# Test external commands
@test "simple external command works" {
    run "${DIR}/../dsh" <<EOF
echo hello
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "hello" ]]
}

@test "external command with multiple arguments" {
    run "${DIR}/../dsh" <<EOF
echo arg1 arg2 arg3
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "arg1 arg2 arg3" ]]
}

@test "quoted strings preserve spaces" {
    run "${DIR}/../dsh" <<EOF
echo "   preserved   spaces   "
EOF
    [ "$status" -eq 0 ]
    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    expected_output="   preserved   spaces   dsh2> dsh2> cmd loop returned 0"
    [ "$stripped_output" = "$expected_output" ]
}

@test "invalid command shows error" {
    run "${DIR}/../dsh" <<EOF
nonexistentcommand
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "execvp" ]]
}

@test "empty input shows warning" {
    run "${DIR}/../dsh" <<EOF

EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "warning: no commands provided" ]]
}

@test "exit command works" {
    run "${DIR}/../dsh" <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "multiple spaces between arguments are collapsed" {
    run "${DIR}/../dsh" <<EOF
echo hello      world
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "hello world" ]]
}

@test "leading/trailing spaces are trimmed" {
    run "${DIR}/../dsh" <<EOF
    echo hello
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "hello" ]]
}

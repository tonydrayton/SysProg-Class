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

@test "cd .. works correctly" {
    current=$(pwd)
    parent=$(dirname "$current")

    run "${DIR}/../dsh" <<EOF
cd ..
pwd
EOF
    [ "$status" -eq 0 ]
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="${parent}dsh2>dsh2>dsh2>cmdloopreturned0"
    [ "$stripped_output" = "$expected_output" ]
}

@test "cd with multiple dots" {
    run "${DIR}/../dsh" <<EOF
cd ....
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "cd: No such file or directory" ]]
}

@test "cd with special characters" {
    run "${DIR}/../dsh" <<EOF
cd /tmp/!@#$%^
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "cd: No such file or directory" ]]
}

@test "cd with very long path" {
    long_path=$(printf 'a%.0s' {1..1000})
    run "${DIR}/../dsh" <<EOF
cd $long_path
EOF
    [ "$status" -eq 0 ]
    [[ "$output" =~ "cd: No such file or directory" ]]
}

@test "cd with spaces in path" {
    current=$(pwd)
    mkdir -p "${current}/test dir with spaces"

    run "${DIR}/../dsh" <<EOF
cd "test dir with spaces"
pwd
cd ..
EOF

    rmdir "${current}/test dir with spaces"
    [ "$status" -eq 0 ]
    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    [[ "$stripped_output" =~ "${current}/test dir with spaces" ]]
}

@test "cd with multiple slashes" {
    run "${DIR}/../dsh" <<EOF
cd ///tmp////
pwd
EOF
    [ "$status" -eq 0 ]
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmpdsh2>dsh2>dsh2>cmdloopreturned0"
    [ "$stripped_output" = "$expected_output" ]
}

@test "cd with relative paths" {
    current=$(pwd)
    mkdir -p "${current}/test_dir/subdir"

    run "${DIR}/../dsh" <<EOF
cd test_dir/subdir
pwd
cd ../..
EOF

    rmdir "${current}/test_dir/subdir"
    rmdir "${current}/test_dir"
    [ "$status" -eq 0 ]
    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')
    [[ "$stripped_output" =~ "${current}/test_dir/subdir" ]]
}

@test "cd with dot-dot path traversal attempt" {
    run "${DIR}/../dsh" <<EOF
cd ../../../../../../../tmp
pwd
EOF
    [ "$status" -eq 0 ]
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/tmpdsh2>dsh2>dsh2>cmdloopreturned0"
    [ "$stripped_output" = "$expected_output" ]
}

@test "extra credit: return codes" {
    run "${DIR}/../dsh" <<EOF
blahblahblahcommand
rc
exit
EOF
    [[ "$output" =~ "Command not found in PATH" ]]
    [[ "$output" =~ "2" ]]

    # Test cd error
    run "${DIR}/../dsh" <<EOF
cd /nonexistent
rc
exit
EOF
    [[ "$output" =~ "No such file or directory" ]]
    [[ "$output" =~ "2" ]]
}

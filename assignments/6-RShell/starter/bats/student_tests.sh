#!/usr/bin/env bats

# File: student_tests.sh
#
# Create your unit tests suit in this file

# Set the path to the shell executable
SHELL="./dsh"

@test "Local shell executes basic commands" {
    run $SHELL <<EOF
ls
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Local shell handles exit command" {
    run $SHELL <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Local shell handles piped commands" {
    run $SHELL <<EOF
ls | grep .c
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Local shell handles built-in cd command" {
    run $SHELL <<EOF
cd ..
pwd
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Client can execute commands on server" {
    $SHELL -s -p 5679 &
    SERVER_PID=$!
    sleep 1

    OUTPUT=$(echo "ls" | $SHELL -c -p 5679)

    echo "$OUTPUT" | grep "dshlib.c" > /dev/null
    [ "$?" -eq 0 ]

    echo "stop-server" | $SHELL -c -p 5679
    sleep 1
}

@test "Client can execute piped commands on server" {
    $SHELL -s -p 5680 &
    SERVER_PID=$!
    sleep 1

    OUTPUT=$(echo "ls | grep .c" | $SHELL -c -p 5680)

    echo "$OUTPUT" | grep "dshlib.c" > /dev/null
    [ "$?" -eq 0 ]

    echo "stop-server" | $SHELL -c -p 5680
    sleep 1
}

@test "Client can use built-in commands on server" {
    $SHELL -s -p 5681 &
    SERVER_PID=$!
    sleep 1

    echo "cd .. && pwd" | $SHELL -c -p 5681
    [ "$?" -eq 0 ]

    echo "stop-server" | $SHELL -c -p 5681
    sleep 1
}

@test "Multiple clients can connect to server sequentially" {
    $SHELL -s -p 5682 &
    SERVER_PID=$!
    sleep 1

    echo "ls" | $SHELL -c -p 5682
    [ "$?" -eq 0 ]

    echo "pwd" | $SHELL -c -p 5682
    [ "$?" -eq 0 ]

    echo "stop-server" | $SHELL -c -p 5682
    sleep 1
}

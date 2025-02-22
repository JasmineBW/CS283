#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Cd with no arguments leaves pwd unchanged" {
    current=$(pwd)
    run ./dsh <<EOF
cd
pwd
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="${current}dsh2>dsh2>dsh2>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "cd to valid directory" {
    run ./dsh <<EOF
cd bats
pwd
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="/starterdsh2>dsh2>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "$output" | grep -q "starter/bats"

    [ "$?" -eq 0 ]
    [ "$status" -eq 0 ]
}

@test "empty command produces warning" {
    run ./dsh <<EOF
       
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>warning:nocommandsprovideddsh2>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "invalid command returns error" {
    run ./dsh <<EOF
invalidcommand
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>CommandnotfoundinPATHdsh2>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "echo unquoted arguments" {
    run ./dsh <<EOF
echo Hello World
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="HelloWorlddsh2>dsh2>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
}

@test "exit command terminates shell immediately" {
    run ./dsh <<EOF
exit
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 249 ]
}

@test "cd with invalid directory returns error" {
    run ./dsh <<EOF
cd /nonexistentdir
exit
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>error:commandexecutionfaileddsh2>"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
}

@test "exit command terminates the shell immediately" {
    run ./dsh <<EOF
exit
EOF
    # Expect that only one prompt is printed before exit and then the final message.
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 249 ]
}

@test "file with no execute permission prints appropriate error" {
    echo "#!/bin/bash" > /tmp/test_noexec.sh
    echo "echo Hello" >> /tmp/test_noexec.sh
    chmod 644 /tmp/test_noexec.sh

    run ./dsh <<EOF
/tmp/test_noexec.sh
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>Permissiondenieddsh2>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    #expect an error message containing "Permission denied"
    echo "${stripped_output} -> ${expected_output}"
    [ "$stripped_output" = "$expected_output" ]
    [ "$status" -eq 0 ]
    rm /tmp/test_noexec.sh
}

@test "rc command prints last operation's return code after error" {
    run ./dsh <<EOF
invalidcommand
rc
EOF
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "$output" | grep -qi "dsh2> 2"
    [ "$status" -eq 0 ]
}

@test "rc command prints last operation's return code after success" {
    run ./dsh <<EOF
echo "test"
rc
EOF
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "$output" | grep -qi "dsh2> 0"
    [ "$status" -eq 0 ]
}
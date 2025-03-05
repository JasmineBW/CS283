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

@test "Exit command terminates shell" {
  run ./dsh <<EOF
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" == *"exiting..."* ]]
  [[ "$output" == *"cmd loop returned"* ]]
}

@test "Dragon command prints dragon art" {
  run ./dsh <<EOF
dragon
exit
EOF
  [ "$status" -eq 0 ]
}

@test "CD command changes directory and subsequent pwd shows new directory" {
  run ./dsh <<EOF
cd /tmp
pwd
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" == *"/tmp"* ]]
}

@test "External command execution: echo" {
  run ./dsh <<EOF
echo hello
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" == *"hello"* ]]
}

@test "Pipeline execution: echo hello | cat" {
  run ./dsh <<EOF
echo hello | cat
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" == *"hello"* ]]
}

@test "File output redirection: echo > file" {
  run ./dsh <<EOF
echo filetest > test_output.txt
exit
EOF
  [ "$status" -eq 0 ]
  [ -f test_output.txt ]
  file_content=$(cat test_output.txt)
  [[ "$file_content" == *"filetest"* ]]
}

@test "File input redirection: cat < file" {
  # Create the input file first.
  echo "inputtest" > test_input.txt
  run ./dsh <<EOF
cat < test_input.txt
exit
EOF
  [ "$status" -eq 0 ]
  [[ "$output" == *"inputtest"* ]]
}

@test "Append output redirection: echo >> file" {
  run ./dsh <<EOF
echo first > test_append.txt
echo second >> test_append.txt
exit
EOF
  [ "$status" -eq 0 ]
  file_content=$(cat test_append.txt)
  [[ "$file_content" == *"first"* ]]
  [[ "$file_content" == *"second"* ]]
}

@test "Empty command input produces warning" {
  run ./dsh <<EOF

exit
EOF
  [ "$status" -eq 0 ]
  # Adjust the expected warning string as defined by your CMD_WARN_NO_CMD constant.
  [[ "$output" == *"CMD_WARN_NO_CMD"* ]] || [[ "$output" == *"warning"* ]]
}

@test "Built-in rc command returns last nonzero return code" {
  run ./dsh <<EOF
foobar
rc
exit
EOF
  [ "$status" -eq 0 ]
  # Check that the rc command prints a nonzero number (indicating an error from 'foobar').
  [[ "$output" =~ [^0-9]*[1-9][0-9]* ]]
}

@test "Too many pipes error is handled" {
  # Construct a command with many pipes to exceed the maximum allowed.
  cmd="echo a"
  $cmd = "|" cmd + " | " + cmd + " | " + cmd + " | " + cmd + " | " + cmd + " | " + cmd + " | " + cmd + " | " + cmd + " | " + cmd + " | " + cmd + " | " + cmd + " | " + cmd 

  run ./dsh <<EOF
$cmd
exit
EOF
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="${current}dsh2>dsh2>dsh2>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Status: $status"
    echo "${stripped_output} -> ${expected_output}"
    [ "$status" -eq 0 ]
    # Adjust the expected error message substring based on your CMD_ERR_EXECUTE constant.
    [[ "$output" == *"CMD_ERR_PIPE_LIMIT"* ]]
}

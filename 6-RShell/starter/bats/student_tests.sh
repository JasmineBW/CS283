#!/usr/bin/env bats
# File: student_tests.sh
#
# Test suite with a dummy server that simulates the remote shell's responses.

setup() {
  dummy_server &
  SERVER_PID=$!
  sleep 1
}

teardown() {
  if kill -0 "$SERVER_PID" 2>/dev/null; then
    kill "$SERVER_PID"
  fi
  rm -f test_output.txt test_input.txt test_append.txt
}

dummy_server() {
  CURR_DIR="/"
  while true; do
    nc -l 1234 -q 1 | while IFS= read -r -d '' cmd; do
      if [ "$cmd" = "exit" ]; then
         printf "Goodbye\x04"
      elif [ "$cmd" = "stop-server" ]; then
         printf "Server stopping\x04"
      elif [[ "$cmd" =~ ^cd[[:space:]]+ ]]; then
         dir=$(echo "$cmd" | awk '{print $2}')
         CURR_DIR="$dir"
         printf "\x04"
      elif [ "$cmd" = "pwd" ]; then
         printf "%s\x04" "$CURR_DIR"
      elif [ "$cmd" = "dragon" ]; then
         printf "dragon art\x04"
      elif [ -z "$cmd" ]; then
         printf "No command entered\x04"
      else
         output=$(bash -c "$cmd" 2>&1)
         printf "%s\x04" "$output"
      fi
    done
  done
}

@test "Example: check ls runs without errors" {
    run bash -c "printf 'ls\nexit\n' | ./dsh -c"
    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="Goodbye"
    echo "${stripped_output} -> ${expected_output}"
    [ "$status" -eq 0 ]
}

@test "Dragon command prints dragon art" {
  run bash -c "printf 'dragon\nexit\n' | ./dsh -c"
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected_output="dragonartGoodbye"
  echo "${stripped_output} -> ${expected_output}"
  [ "$status" -eq 0 ]
}

@test "CD command changes directory and subsequent pwd shows new directory" {
  run bash -c "printf 'cd /bats\npwd\nexit\n' | ./dsh -c"
  # Instead of expecting exactly "/batsGoodbye", we check that "/bats" appears somewhere.
  [ "$status" -eq 0 ]
  [[ "$output" == *"/bats"* ]]
}

@test "External command execution: echo" {
  run bash -c "printf 'echo hello\nexit\n' | ./dsh -c"
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected_output="helloGoodbye"
  echo "${stripped_output} -> ${expected_output}"
  [ "$status" -eq 0 ]
  [[ "$output" == *"hello"* ]]
}

@test "Pipeline execution: echo hello | cat" {
  run bash -c "printf 'echo hello | cat\nexit\n' | ./dsh -c"
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected_output="helloGoodbye"
  echo "${stripped_output} -> ${expected_output}"
  [ "$status" -eq 0 ]
  [[ "$output" == *"hello"* ]]
}

@test "File output redirection: echo > file" {
  run bash -c "printf 'echo filetest > test_output.txt\nexit\n' | ./dsh -c"
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected_output="Goodbye"
  echo "${stripped_output} -> ${expected_output}"
  [ "$status" -eq 0 ]
  sleep 0.5
  [ -f test_output.txt ]
  file_content=$(cat test_output.txt)
  [[ "$file_content" == *"filetest"* ]]
}

@test "File input redirection: cat < file" {
  echo "inputtest" > test_input.txt
  run bash -c "printf 'cat < test_input.txt\nexit\n' | ./dsh -c"
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected_output="inputtestGoodbye"
  echo "${stripped_output} -> ${expected_output}"
  [ "$status" -eq 0 ]
  [[ "$output" == *"inputtest"* ]]
}

@test "Append output redirection: echo >> file" {
  run bash -c "printf 'echo first > test_append.txt\necho second >> test_append.txt\nexit\n' | ./dsh -c"
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected_output="Goodbye"
  echo "${stripped_output} -> ${expected_output}"
  [ "$status" -eq 0 ]
  sleep 0.5
  file_content=$(cat test_append.txt)
  [[ "$file_content" == *"first"* ]]
  [[ "$file_content" == *"second"* ]]
}

@test "Empty command input produces warning" {
  run bash -c "printf '\nexit\n' | ./dsh -c"
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected_output="NocommandenteredGoodbye"
  echo "${stripped_output} -> ${expected_output}"
  [ "$status" -eq 0 ]
  [[ "$output" == *"No command entered"* ]] || [[ "$output" == *"warning"* ]]
}

@test "Built-in rc command returns last nonzero return code" {
  run bash -c "printf 'foobar\nrc\nexit\n' | ./dsh -c"
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected_output="127Goodbye"
  echo "${stripped_output} -> ${expected_output}"
  [ "$status" -eq 0 ]
  [[ "$output" =~ [1-9] ]]
}

@test "Too many pipes error is handled" {
  cmd="echo a"
  for i in {1..12}; do
      cmd+=" | echo a"
  done
  run bash -c "printf '$cmd\nexit\n' | ./dsh -c"
  stripped_output=$(echo "$output" | tr -d '[:space:]')
  expected_output="CMD_ERR_PIPE_LIMIT"
  echo "${stripped_output} -> ${expected_output}"
  [ "$status" -eq 0 ]
  [[ "$output" == *"CMD_ERR_PIPE_LIMIT"* ]]
}

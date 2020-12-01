#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]; then
  clang_format_command="clang-format"
  clang_tidy_command="run-clang-tidy"
fi
if [[ "$OSTYPE" == "linux"* ]]; then
  clang_format_command="clang-format-11"
  clang_tidy_command="run-clang-tidy-11"
fi

eval "$clang_format_command -i *.h *.cpp -style=file"
eval "$clang_tidy_command -fix"

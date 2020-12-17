#!/bin/bash

if [[ "$OSTYPE" == "darwin"* ]]; then
  clang_format_command="clang-format"
fi
if [[ "$OSTYPE" == "linux"* ]]; then
  clang_format_command="clang-format-11"
fi

find . \( -name "*.cpp" -or -name "*.h" \) -exec $clang_format_command -i *.h *.cpp -style=file {} \;

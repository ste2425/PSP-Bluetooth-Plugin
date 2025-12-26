#!/bin/bash

# Script to scan all files for TODO comments and generate a markdown file
# Usage: ./generate_todos.sh [output_file]
# Default output: TODO.md

OUTPUT_FILE="${1:-TODO.md}"
TEMP_FILE=$(mktemp)

{
    echo "# TODO List"
    echo ""
    echo "Generated on $(date)"
    echo ""

    # Find all files (excluding .git, build artifacts, and binaries)
    find . \
        -not -path './.git/*' \
        -not -path './release/*' \
        -not -path './.vscode/*' \
        -type f \
        \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' -o -name '*.s' -o -name '*.S' -o -name '*.py' \) | \
    while read file; do
        # Use grep to find TODO comments (case-insensitive)
        # Matches: // TODO, /* TODO, * TODO
        grep -n -i "TODO" "$file" | while read -r line; do
            # Extract line number and content
            line_num=$(echo "$line" | cut -d: -f1)
            content=$(echo "$line" | cut -d: -f2-)
            
            # Clean up the content (remove leading/trailing whitespace, comment markers)
            content=$(echo "$content" | sed 's/^[[:space:]]*//' | sed 's/\/\///g' | sed 's/\/\*//' | sed 's/\*\///' | sed 's/^\*//' | sed 's/^[[:space:]]*//')
            
            echo "- [$file:$line_num] $content"
        done
    done | sort

} > "$TEMP_FILE"

# Move temp file to output
mv "$TEMP_FILE" "$OUTPUT_FILE"

echo "✓ TODO list generated: $OUTPUT_FILE"
echo "✓ Found $(grep -c '^-' "$OUTPUT_FILE") TODO items"

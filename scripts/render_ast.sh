#!/usr/bin/env bash
# Helper script to render Graphviz AST dot file to PNG
set -e

DOT_FILE="${1:-ast.dot}"
PNG_FILE="${2:-ast.png}"

if ! command -v dot &> /dev/null; then
    echo "Error: 'dot' (Graphviz) command not found in PATH." >&2
    exit 1
fi

if [ ! -f "$DOT_FILE" ]; then
    echo "Error: Input file '$DOT_FILE' not found." >&2
    exit 1
fi

dot -Tpng "$DOT_FILE" -o "$PNG_FILE"
echo "Rendered $DOT_FILE -> $PNG_FILE"

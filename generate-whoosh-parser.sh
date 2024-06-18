#!/usr/bin/env bash

set -e
set -x

antlr4 -Dlanguage=Cpp -package whoosh -visitor src/uspto/whoosh/Whoosh.g4

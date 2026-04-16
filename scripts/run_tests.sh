#!/bin/bash

set -e

BUILD_DIR="build"
TEST_DIR="$BUILD_DIR/tests"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=== Running libclap Tests ===${NC}\n"

PASSED=0
FAILED=0

echo -e "${YELLOW}[Unit Tests]${NC}"
for test in test_clap test_allocator test_arena test_buffer test_trie \
            test_error test_parser test_argument test_namespace \
            test_validator test_convert test_actions test_mutex \
            test_dependency test_subparser test_find test_formatter \
            test_tokenizer; do
    if [ -f "$TEST_DIR/$test" ]; then
        echo -n "  $test ... "
        if $TEST_DIR/$test > /dev/null 2>&1; then
            echo -e "${GREEN}PASSED${NC}"
            ((PASSED++))
        else
            echo -e "${RED}FAILED${NC}"
            ((FAILED++))
        fi
    else
        echo -e "  $test ... ${RED}NOT FOUND${NC}"
    fi
done

echo -e "\n${YELLOW}[Integration Tests]${NC}"
for test in test_usage test_scenarios test_parse_flow; do
    if [ -f "$TEST_DIR/$test" ]; then
        echo -n "  $test ... "
        if $TEST_DIR/$test > /dev/null 2>&1; then
            echo -e "${GREEN}PASSED${NC}"
            ((PASSED++))
        else
            echo -e "${RED}FAILED${NC}"
            ((FAILED++))
        fi
    fi
done

echo -e "\n${YELLOW}=== Summary ===${NC}"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}Some tests failed!${NC}"
    exit 1
fi
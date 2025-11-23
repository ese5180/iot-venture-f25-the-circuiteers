#!/bin/bash
#
# Comprehensive test script for pre-commit and pre-push hooks
# Tests various scenarios to ensure hooks work correctly
#

set +e  # Don't exit on error - we're testing

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Get repository root
REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null)"
if [ -z "$REPO_ROOT" ]; then
    echo -e "${RED}Error: Not in a git repository${NC}"
    exit 1
fi

cd "$REPO_ROOT"

# Hook paths
PRE_COMMIT_HOOK=".git/hooks/pre-commit"
PRE_PUSH_HOOK=".git/hooks/pre-push"

# Check if hooks are installed
if [ ! -f "$PRE_COMMIT_HOOK" ] || [ ! -f "$PRE_PUSH_HOOK" ]; then
    echo -e "${RED}Error: Hooks not installed. Run ./hooks/install.sh first${NC}"
    exit 1
fi

# Ensure test files are clean from previous runs
TEST_FILE="sensors_test/tests_unit_mock/unit/test_bme280_mock.c"
if [ -f "${TEST_FILE}.backup" ]; then
    echo "Restoring test file from previous run..."
    mv "${TEST_FILE}.backup" "$TEST_FILE"
fi

# Clean any leftover build directory
if [ -d "sensors_test/tests_unit_mock/build" ]; then
    echo "Cleaning leftover build directory from previous run..."
    rm -rf "sensors_test/tests_unit_mock/build"
fi

# Save original environment
ORIGINAL_PATH="$PATH"
ORIGINAL_ZEPHYR_BASE="$ZEPHYR_BASE"

# Helper functions
print_section() {
    echo ""
    echo -e "${CYAN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║ $1${NC}"
    echo -e "${CYAN}╚════════════════════════════════════════════════════════════╝${NC}"
}

print_test() {
    echo -e "\n${BLUE}[TEST $TESTS_RUN] $1${NC}"
}

print_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓ PASS${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗ FAIL: $2${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
}

# Cleanup function
cleanup() {
    echo ""
    echo "Cleaning up test artifacts..."
    
    # Remove test files
    rm -f test_file.c test_large.bin test_conflict.txt test_whitespace.txt
    
    # Reset git state
    git reset HEAD . > /dev/null 2>&1
    git checkout -- . > /dev/null 2>&1
    
    # Restore environment
    export PATH="$ORIGINAL_PATH"
    if [ -n "$ORIGINAL_ZEPHYR_BASE" ]; then
        export ZEPHYR_BASE="$ORIGINAL_ZEPHYR_BASE"
    else
        unset ZEPHYR_BASE
    fi
    
    # Clean build directory if it was created for tests
    if [ -d "sensors_test/tests_unit_mock/build" ]; then
        echo "Removing test build directory..."
        rm -rf "sensors_test/tests_unit_mock/build"
    fi
}

# Set up cleanup trap
trap cleanup EXIT

# ============================================================
# PRE-COMMIT HOOK TESTS
# ============================================================

print_section "PRE-COMMIT HOOK TESTS"

# Test 1: Pre-commit with no staged files
print_test "Pre-commit with no staged files"
TESTS_RUN=$((TESTS_RUN + 1))

git reset HEAD . > /dev/null 2>&1
OUTPUT=$($PRE_COMMIT_HOOK 2>&1)
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ] && echo "$OUTPUT" | grep -q "No staged files"; then
    print_result 0
else
    print_result 1 "Should exit successfully with 'No staged files' message"
fi

# Test 2: Pre-commit with merge conflict markers
print_test "Pre-commit with merge conflict markers (should FAIL)"
TESTS_RUN=$((TESTS_RUN + 1))

cat > test_conflict.txt << 'EOF'
This is a test file
<<<<<<< HEAD
Some content
=======
Other content
>>>>>>> branch
EOF

git add test_conflict.txt
OUTPUT=$($PRE_COMMIT_HOOK 2>&1)
EXIT_CODE=$?
git reset HEAD test_conflict.txt > /dev/null 2>&1
rm -f test_conflict.txt

if [ $EXIT_CODE -ne 0 ] && echo "$OUTPUT" | grep -q "Merge conflict markers"; then
    print_result 0
else
    print_result 1 "Should detect merge conflict markers and fail"
fi

# Test 3: Pre-commit with trailing whitespace (warning only)
print_test "Pre-commit with trailing whitespace (warning)"
TESTS_RUN=$((TESTS_RUN + 1))

cat > test_whitespace.txt << 'EOF'
This line has trailing whitespace   
This one too  
EOF

git add test_whitespace.txt
OUTPUT=$($PRE_COMMIT_HOOK 2>&1)
EXIT_CODE=$?
git reset HEAD test_whitespace.txt > /dev/null 2>&1
rm -f test_whitespace.txt

if [ $EXIT_CODE -eq 0 ] && echo "$OUTPUT" | grep -q "Trailing whitespace"; then
    print_result 0
else
    print_result 1 "Should warn about trailing whitespace but pass"
fi

# Test 4: Pre-commit with C file - brace mismatch check
print_test "Pre-commit with C file (brace mismatch warning)"
TESTS_RUN=$((TESTS_RUN + 1))

cat > test_file.c << 'EOF'
#include <stdio.h>

int main() {
    printf("Missing closing brace\n");
    if (1) {
        return 0;
    // Only 2 opening braces, 1 closing - actual mismatch
EOF

git add test_file.c
OUTPUT=$($PRE_COMMIT_HOOK 2>&1)
EXIT_CODE=$?
git reset HEAD test_file.c > /dev/null 2>&1
rm -f test_file.c

if echo "$OUTPUT" | grep -q "brace mismatch"; then
    print_result 0
else
    print_result 1 "Should warn about brace mismatch"
fi

# Test 5: Pre-commit with TODO comments (warning only)
print_test "Pre-commit with TODO/FIXME comments (warning)"
TESTS_RUN=$((TESTS_RUN + 1))

cat > test_file.c << 'EOF'
#include <stdio.h>

int main() {
    // TODO: Implement this feature
    // FIXME: This is broken
    return 0;
}
EOF

git add test_file.c
OUTPUT=$($PRE_COMMIT_HOOK 2>&1)
EXIT_CODE=$?
git reset HEAD test_file.c > /dev/null 2>&1
rm -f test_file.c

if [ $EXIT_CODE -eq 0 ] && echo "$OUTPUT" | grep -q "TODO/FIXME"; then
    print_result 0
else
    print_result 1 "Should warn about TODO/FIXME but pass"
fi

# Test 6: Pre-commit with unit tests (if cmake available)
print_test "Pre-commit with unit tests (normal execution)"
TESTS_RUN=$((TESTS_RUN + 1))

if command -v cmake &> /dev/null && command -v ctest &> /dev/null; then
    # Create a dummy file to stage
    echo "// Test file" > test_file.c
    git add test_file.c
    
    OUTPUT=$($PRE_COMMIT_HOOK 2>&1)
    EXIT_CODE=$?
    
    git reset HEAD test_file.c > /dev/null 2>&1
    rm -f test_file.c
    
    if echo "$OUTPUT" | grep -q "Running Mock Unit Tests"; then
        if [ $EXIT_CODE -eq 0 ] && echo "$OUTPUT" | grep -q "mock unit tests PASSED"; then
            print_result 0
        else
            print_result 1 "Tests should pass"
            echo "Output: $OUTPUT"
        fi
    else
        print_result 1 "Should run unit tests when cmake is available"
    fi
else
    echo -e "${YELLOW}⊘ SKIP: cmake/ctest not available${NC}"
    TESTS_RUN=$((TESTS_RUN - 1))
fi

# Test 7: Pre-commit without cmake (should skip tests gracefully)
print_test "Pre-commit without cmake (should skip tests gracefully)"
TESTS_RUN=$((TESTS_RUN + 1))

# Temporarily hide cmake from PATH
export PATH="/usr/bin:/bin"

echo "// Test file" > test_file.c
git add test_file.c

OUTPUT=$($PRE_COMMIT_HOOK 2>&1)
EXIT_CODE=$?

git reset HEAD test_file.c > /dev/null 2>&1
rm -f test_file.c

# Restore PATH
export PATH="$ORIGINAL_PATH"

if [ $EXIT_CODE -eq 0 ] && echo "$OUTPUT" | grep -q "cmake not found.*Skipping unit tests"; then
    print_result 0
else
    print_result 1 "Should skip tests gracefully when cmake is not available"
fi

# ============================================================
# PRE-PUSH HOOK TESTS
# ============================================================

print_section "PRE-PUSH HOOK TESTS"

# Test 8: Pre-push without ZEPHYR_BASE (should skip gracefully)
print_test "Pre-push without ZEPHYR_BASE (should skip build)"
TESTS_RUN=$((TESTS_RUN + 1))

unset ZEPHYR_BASE

OUTPUT=$($PRE_PUSH_HOOK 2>&1)
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ] && echo "$OUTPUT" | grep -q "ZEPHYR_BASE not set"; then
    print_result 0
else
    print_result 1 "Should skip build when ZEPHYR_BASE is not set"
fi

# Test 9: Pre-push without west (should skip gracefully)
print_test "Pre-push without west (should skip build)"
TESTS_RUN=$((TESTS_RUN + 1))

export ZEPHYR_BASE="/tmp/fake_zephyr"
export PATH="/usr/bin:/bin"  # Remove west from PATH

OUTPUT=$($PRE_PUSH_HOOK 2>&1)
EXIT_CODE=$?

export PATH="$ORIGINAL_PATH"
unset ZEPHYR_BASE

if [ $EXIT_CODE -eq 0 ] && echo "$OUTPUT" | grep -q "west not found"; then
    print_result 0
else
    print_result 1 "Should skip build when west is not available"
fi

# Test 10: Pre-push with unit tests (if cmake available)
print_test "Pre-push with unit tests (normal execution)"
TESTS_RUN=$((TESTS_RUN + 1))

if command -v cmake &> /dev/null && command -v ctest &> /dev/null; then
    # Skip build by not setting ZEPHYR_BASE, but test unit tests
    unset ZEPHYR_BASE
    
    OUTPUT=$($PRE_PUSH_HOOK 2>&1)
    EXIT_CODE=$?
    
    # Pre-push should exit 0 when ZEPHYR_BASE not set
    if [ $EXIT_CODE -eq 0 ]; then
        print_result 0
    else
        print_result 1 "Pre-push should pass when ZEPHYR_BASE not set"
    fi
else
    echo -e "${YELLOW}⊘ SKIP: cmake/ctest not available${NC}"
    TESTS_RUN=$((TESTS_RUN - 1))
fi

# Test 11: Pre-push is actually executable
print_test "Pre-push hook is executable"
TESTS_RUN=$((TESTS_RUN + 1))

if [ -x $PRE_PUSH_HOOK ]; then
    print_result 0
else
    print_result 1 "Pre-push hook should be executable"
fi

# ============================================================
# MOCK UNIT TESTS - DIRECT TEST
# ============================================================

print_section "DIRECT MOCK UNIT TESTS"

# Test 12: Build and run mock unit tests directly
print_test "Build and run mock unit tests directly"
TESTS_RUN=$((TESTS_RUN + 1))

if command -v cmake &> /dev/null && command -v ctest &> /dev/null; then
    TEST_DIR="sensors_test/tests_unit_mock"
    
    if [ -d "$TEST_DIR" ]; then
        # Clean build directory
        rm -rf "$TEST_DIR/build"
        mkdir -p "$TEST_DIR/build"
        cd "$TEST_DIR/build"
        
        # Configure
        if cmake -DCMAKE_BUILD_TYPE=Release .. > /tmp/cmake_test.log 2>&1; then
            # Build
            if cmake --build . > /tmp/cmake_build_test.log 2>&1; then
                # Run tests
                if ctest --output-on-failure > /tmp/ctest_output.log 2>&1; then
                    print_result 0
                else
                    print_result 1 "Tests failed"
                    echo "Test output:"
                    cat /tmp/ctest_output.log
                fi
            else
                print_result 1 "Build failed"
                cat /tmp/cmake_build_test.log
            fi
        else
            print_result 1 "CMake configuration failed"
            cat /tmp/cmake_test.log
        fi
        
        cd "$REPO_ROOT"
    else
        print_result 1 "Test directory not found"
    fi
else
    echo -e "${YELLOW}⊘ SKIP: cmake/ctest not available${NC}"
    TESTS_RUN=$((TESTS_RUN - 1))
fi

# Test 13: Verify test executables exist
print_test "Verify test executables were created"
TESTS_RUN=$((TESTS_RUN + 1))

if [ -f "sensors_test/tests_unit_mock/build/test_bme280_mock" ] && \
   [ -f "sensors_test/tests_unit_mock/build/test_adxl345_mock" ]; then
    print_result 0
else
    if command -v cmake &> /dev/null; then
        print_result 1 "Test executables not found"
    else
        echo -e "${YELLOW}⊘ SKIP: cmake not available${NC}"
        TESTS_RUN=$((TESTS_RUN - 1))
    fi
fi

# Test 14: Run individual test executable
print_test "Run individual test executable (test_bme280_mock)"
TESTS_RUN=$((TESTS_RUN + 1))

if [ -f "sensors_test/tests_unit_mock/build/test_bme280_mock" ]; then
    OUTPUT=$(./sensors_test/tests_unit_mock/build/test_bme280_mock 2>&1)
    EXIT_CODE=$?
    
    if [ $EXIT_CODE -eq 0 ] && echo "$OUTPUT" | grep -q "ALL TESTS PASSED"; then
        print_result 0
    else
        print_result 1 "BME280 test should pass"
        echo "Output: $OUTPUT"
    fi
else
    if command -v cmake &> /dev/null; then
        print_result 1 "Test executable not found"
    else
        echo -e "${YELLOW}⊘ SKIP: cmake not available${NC}"
        TESTS_RUN=$((TESTS_RUN - 1))
    fi
fi

# Test 15: Run individual test executable (ADXL345)
print_test "Run individual test executable (test_adxl345_mock)"
TESTS_RUN=$((TESTS_RUN + 1))

if [ -f "sensors_test/tests_unit_mock/build/test_adxl345_mock" ]; then
    OUTPUT=$(./sensors_test/tests_unit_mock/build/test_adxl345_mock 2>&1)
    EXIT_CODE=$?
    
    if [ $EXIT_CODE -eq 0 ] && echo "$OUTPUT" | grep -q "ALL TESTS PASSED"; then
        print_result 0
    else
        print_result 1 "ADXL345 test should pass"
        echo "Output: $OUTPUT"
    fi
else
    if command -v cmake &> /dev/null; then
        print_result 1 "Test executable not found"
    else
        echo -e "${YELLOW}⊘ SKIP: cmake not available${NC}"
        TESTS_RUN=$((TESTS_RUN - 1))
    fi
fi

# ============================================================
# SIMULATED FAILURE TESTS
# ============================================================

print_section "SIMULATED FAILURE TESTS"

# Test 16: Simulate test failure by modifying test
print_test "Simulate test failure (modify test to fail)"
TESTS_RUN=$((TESTS_RUN + 1))

if command -v cmake &> /dev/null && command -v ctest &> /dev/null; then
    TEST_FILE="sensors_test/tests_unit_mock/unit/test_bme280_mock.c"
    
    if [ -f "$TEST_FILE" ]; then
        # Backup original file
        cp "$TEST_FILE" "${TEST_FILE}.backup"
        
        # Modify test to fail - make assertion that uses dev but always fails
        perl -i -pe 's/ASSERT_NOT_NULL\(dev, "BME280 device should be detected"\);/ASSERT_TRUE(dev == NULL, "Forced failure for testing");/' "$TEST_FILE"
        
        # Remove build directory to force complete rebuild
        rm -rf sensors_test/tests_unit_mock/build
        mkdir -p sensors_test/tests_unit_mock/build
        cd sensors_test/tests_unit_mock/build
        
        # Configure and build
        cmake .. > /dev/null 2>&1
        cmake --build . > /dev/null 2>&1
        
        # Run tests (should fail)
        OUTPUT=$(ctest --output-on-failure 2>&1)
        EXIT_CODE=$?
        
        cd "$REPO_ROOT"
        
        # Restore original file
        mv "${TEST_FILE}.backup" "$TEST_FILE"
        
        # Rebuild with original file for subsequent tests
        rm -rf sensors_test/tests_unit_mock/build
        mkdir -p sensors_test/tests_unit_mock/build
        cd sensors_test/tests_unit_mock/build
        cmake .. > /dev/null 2>&1
        cmake --build . > /dev/null 2>&1
        cd "$REPO_ROOT"
        
        if [ $EXIT_CODE -ne 0 ]; then
            print_result 0
        else
            print_result 1 "Modified test should fail"
        fi
    else
        print_result 1 "Test file not found"
    fi
else
    echo -e "${YELLOW}⊘ SKIP: cmake/ctest not available${NC}"
    TESTS_RUN=$((TESTS_RUN - 1))
fi

# Test 17: Pre-commit should catch failing tests
print_test "Pre-commit should catch failing tests"
TESTS_RUN=$((TESTS_RUN + 1))

if command -v cmake &> /dev/null && command -v ctest &> /dev/null; then
    TEST_FILE="sensors_test/tests_unit_mock/unit/test_bme280_mock.c"
    
    if [ -f "$TEST_FILE" ]; then
        # Backup and modify using perl for cross-platform compatibility
        cp "$TEST_FILE" "${TEST_FILE}.backup"
        # Make assertion that uses dev but always fails
        perl -i -pe 's/ASSERT_NOT_NULL\(dev, "BME280 device should be detected"\);/ASSERT_TRUE(dev == NULL, "Forced failure");/' "$TEST_FILE"
        
        # Remove build dir to force rebuild
        rm -rf sensors_test/tests_unit_mock/build
        
        # Stage a file to trigger pre-commit
        echo "// trigger commit" > test_file.c
        git add test_file.c
        
        # Run pre-commit hook
        OUTPUT=$($PRE_COMMIT_HOOK 2>&1)
        EXIT_CODE=$?
        
        # Cleanup
        git reset HEAD test_file.c > /dev/null 2>&1
        rm -f test_file.c
        mv "${TEST_FILE}.backup" "$TEST_FILE"
        
        if [ $EXIT_CODE -ne 0 ] && echo "$OUTPUT" | grep -qi "mock unit tests FAILED"; then
            print_result 0
        else
            print_result 1 "Pre-commit should fail when tests fail"
            echo "Exit code: $EXIT_CODE"
        fi
    else
        print_result 1 "Test file not found"
    fi
else
    echo -e "${YELLOW}⊘ SKIP: cmake/ctest not available${NC}"
    TESTS_RUN=$((TESTS_RUN - 1))
fi

# ============================================================
# FINAL SUMMARY
# ============================================================

print_section "TEST SUMMARY"

echo ""
echo "Total Tests Run:    $TESTS_RUN"
echo -e "Tests Passed:       ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed:       ${RED}$TESTS_FAILED${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                  ✓ ALL TESTS PASSED                        ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"
    exit 0
else
    echo -e "${RED}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║                  ✗ SOME TESTS FAILED                       ║${NC}"
    echo -e "${RED}╚════════════════════════════════════════════════════════════╝${NC}"
    exit 1
fi


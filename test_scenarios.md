# Testing Pre-Commit Hook Scenarios

## Test 1: Normal Commit (Should Pass)
```bash
echo "Normal file" > normal.txt
git add normal.txt
git commit -m "Normal commit"
```
Expected: ✅ Should pass with warnings if any

## Test 2: Merge Conflict Markers (Should Fail)

Expected: ❌ Should block commit

## Test 3: TODO Comment (Should Warn)
```bash
echo "// TODO: Fix this later" > todo.c
git add todo.c
git commit -m "Test TODO"
```
Expected: ⚠️ Should warn but allow commit

## Test 4: Large File (Should Warn)
```bash
# Create a large file (>10MB)
fsutil file createnew large.bin 10485761
git add large.bin
git commit -m "Test large file"
```
Expected: ⚠️ Should warn but allow commit


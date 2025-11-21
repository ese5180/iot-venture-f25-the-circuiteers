# Git Hooks for VendGuard Project

This directory contains git hooks to help maintain code quality and catch issues early in the development process.

## Available Hooks

### Pre-commit Hook
Runs before each commit to perform quick checks:
- Checks for merge conflict markers
- Warns about TODO/FIXME comments
- Warns about trailing whitespace
- Warns about large files (>10MB)
- Basic syntax checks for C files

### Pre-push Hook
Runs before pushing to remote repository:
- Verifies that the main application builds successfully
- Prevents pushing code that doesn't compile
- Uses a 5-minute timeout to prevent hanging

## Installation

### Option 1: Automatic Installation (Recommended)

Run the install script:
```bash
chmod +x hooks/install.sh
./hooks/install.sh
```

### Option 2: Manual Installation

1. Copy the hooks to your `.git/hooks/` directory:
   ```bash
   cp hooks/pre-commit .git/hooks/pre-commit
   cp hooks/pre-push .git/hooks/pre-push
   ```

2. Make them executable:
   ```bash
   chmod +x .git/hooks/pre-commit
   chmod +x .git/hooks/pre-push
   ```

### Option 3: Using Git Config (for team sharing)

If you want to share hooks with your team, you can set the git hooks path:
```bash
git config core.hooksPath hooks
```

**Note:** This requires the hooks directory to be committed to the repository.

## Usage

### Pre-commit Hook
The pre-commit hook runs automatically when you run `git commit`. If it finds issues, it will:
- Show warnings for minor issues (you can still commit)
- Block the commit for critical issues (merge conflicts, etc.)

To skip the hook (not recommended):
```bash
git commit --no-verify
```

### Pre-push Hook
The pre-push hook runs automatically when you run `git push`. It will:
- Build your main application to verify it compiles
- Block the push if the build fails

To skip the hook (not recommended):
```bash
git push --no-verify
```

## Requirements

### Pre-commit Hook
- No special requirements (runs basic checks)

### Pre-push Hook
- Zephyr environment must be initialized (`source zephyr/zephyr-env.sh`)
- `west` tool must be installed and in PATH
- Zephyr SDK must be installed

## Troubleshooting

### Hook not running
- Make sure the hook file is executable: `chmod +x .git/hooks/pre-commit`
- Check that the hook is in the correct location: `.git/hooks/`

### Pre-push hook fails but code compiles locally
- Make sure ZEPHYR_BASE is set: `echo $ZEPHYR_BASE`
- Make sure west is in your PATH: `which west`
- Try building manually: `west build -b nucleo_wl55jc apps`

### Pre-push hook is too slow
- The hook uses a 5-minute timeout
- If your build takes longer, consider skipping the hook for large refactors: `git push --no-verify`
- Or modify the timeout in `hooks/pre-push`

## Customization

You can modify the hooks to:
- Add additional checks
- Change which files are checked
- Adjust timeout values
- Add team-specific rules

## Disabling Hooks

To temporarily disable hooks:
```bash
# Skip pre-commit
git commit --no-verify

# Skip pre-push
git push --no-verify
```

To permanently remove hooks:
```bash
rm .git/hooks/pre-commit
rm .git/hooks/pre-push
```


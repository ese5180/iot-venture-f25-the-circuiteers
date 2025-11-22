# TDD & CI/CD 完整设置指南

## 系统要求

### 最小要求
- CMake 3.20+
- C11 编译器（GCC, Clang, MSVC）
- Git 2.9+（支持 hooks 配置）

### 安装步骤

#### macOS
```bash
brew install cmake
```

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install cmake build-essential
```

#### Windows
- 安装 [CMake](https://cmake.org/download/)
- 安装 [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/) 或使用 MinGW

## 快速配置 (5 分钟)

### 1. 克隆项目
```bash
cd /your/project/path
git clone <repo-url>
cd sensors_test
```

### 2. 配置 Git Hooks
```bash
# 方式 A：自动配置 (推荐)
git config core.hooksPath .githooks

# 验证配置
git config core.hooksPath
# 输出: .githooks
```

### 3. 构建 Mock 测试
```bash
cd tests_unit_mock
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### 4. 验证测试
```bash
# 方式 A：使用 CTest
ctest --output-on-failure

# 方式 B：直接运行
./test_bme280_mock
./test_adxl345_mock
```

### 5. 完成！
现在每次 `git commit` 时，Mock 测试将自动运行。

## 详细配置指南

### Pre-commit 钩子配置

#### 选项 1：自动配置（推荐）
```bash
# 在项目根目录运行一次
git config core.hooksPath .githooks

# 验证
git config --list | grep hooksPath
```

**优势**：
- 适用于所有团队成员
- 无需手动复制文件
- Git 2.9+ 自动支持

#### 选项 2：手动配置
```bash
# 复制钩子脚本
cp .githooks/pre-commit .git/hooks/pre-commit

# 设置可执行权限
chmod +x .git/hooks/pre-commit

# 验证
ls -la .git/hooks/pre-commit
```

#### 选项 3：全局配置（所有 Git 项目）
```bash
# 创建全局 hooks 目录
mkdir -p ~/.githooks
cp .githooks/pre-commit ~/.githooks/

# 配置 Git
git config --global core.hooksPath ~/.githooks
```

### Mock 测试构建配置

#### 发布版本 (推荐用于 CI/CD)
```bash
cd tests_unit_mock/build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
ctest
```

#### 调试版本 (用于开发)
```bash
cd tests_unit_mock/build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
ctest --verbose --output-on-failure
```

#### 启用代码覆盖 (用于分析)
```bash
cd tests_unit_mock/build_coverage
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="--coverage" \
      -DCMAKE_CXX_FLAGS="--coverage" ..
cmake --build .
ctest
```

## GitHub Actions 工作流配置

### 自动启用条件

工作流文件已存在于 `.github/workflows/mock-tests.yml`，满足以下条件时自动运行：

1. **Push 到主分支**
   ```bash
   git push origin main
   ```

2. **PR 到主分支**
   - GitHub 自动创建 PR 检查

3. **每日计划运行**
   - 每天 UTC 2:00 AM 自动运行

### 工作流配置

编辑 `.github/workflows/mock-tests.yml` 自定义行为：

```yaml
on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]
  schedule:
    - cron: '0 2 * * *'  # 修改运行时间
```

### 查看 CI/CD 结果

1. GitHub 仓库 → "Actions" 标签
2. 选择最新的 workflow run
3. 查看各个 job 的详细日志

## 常见工作流

### 开发新功能

```bash
# 1. 创建特性分支
git checkout -b feature/my-feature

# 2. 修改代码并添加测试
vim tests_unit_mock/unit/test_bme280_mock.c
# 添加新的测试函数...

# 3. 在本地运行测试
cd tests_unit_mock/build
ctest --output-on-failure

# 4. 提交 (pre-commit 钩子自动运行)
git add .
git commit -m "Add new feature with tests"

# ✅ Pre-commit 自动运行测试并通过

# 5. 推送到远程
git push origin feature/my-feature

# 6. GitHub Actions 在 PR 中自动验证
```

### 修复 Bug 并验证

```bash
# 1. 识别问题并修复
git checkout main
git pull origin main
git checkout -b fix/critical-bug

# 2. 修改代码
vim src/main.c

# 3. 添加测试用例验证修复
vim tests_unit_mock/unit/test_adxl345_mock.c

# 4. 本地验证
cd tests_unit_mock/build
cmake --build .
ctest --verbose

# 5. 提交
git commit -m "Fix critical bug - verified with new tests"

# 6. 推送 + CI/CD 验证
git push origin fix/critical-bug
```

### 发布版本

```bash
# 1. 准备发布
git checkout main
git pull origin main

# 2. 确保所有测试通过
cd tests_unit_mock/build
ctest --output-on-failure

# 3. 更新版本号
vim VERSION  # 或其他版本管理方式

# 4. 提交和标记
git commit -m "Release v1.0.0"
git tag -a v1.0.0 -m "Version 1.0.0"
git push origin main --tags

# ✅ GitHub Actions 自动运行完整测试套件
```

## 故障排查

### 问题 1: Pre-commit 钩子不运行

**症状**：`git commit` 时没有看到测试输出

**解决方案**：
```bash
# 1. 验证配置
git config core.hooksPath
# 应输出: .githooks

# 2. 检查钩子文件
ls -la .githooks/pre-commit
# 应显示: -rwxr-xr-x (可执行)

# 3. 手动测试钩子
bash .githooks/pre-commit
# 应输出测试结果

# 4. 重新配置
git config core.hooksPath .githooks
```

### 问题 2: CMake 配置失败

**症状**：`cmake ..` 报错 "CMake not found"

**解决方案**：
```bash
# 检查 CMake 是否安装
cmake --version

# 如果未安装，选择对应平台安装
# macOS:
brew install cmake

# Ubuntu:
sudo apt-get install cmake

# Windows:
# 从 https://cmake.org/download/ 下载并安装
```

### 问题 3: 编译错误

**症状**：`cmake --build .` 报错

**解决方案**：
```bash
# 清理并重新构建
cd tests_unit_mock
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# 如果仍然失败，检查编译器
gcc --version  # 或 clang --version
```

### 问题 4: 测试失败

**症状**：`ctest --output-on-failure` 显示失败

**解决方案**：
```bash
# 1. 查看详细输出
cd tests_unit_mock/build
./test_bme280_mock
./test_adxl345_mock

# 2. 检查 Mock 值是否需要更新
vim unit/mock_sensor.c

# 3. 如果是值不匹配的问题，更新默认值：
# mock_state.adxl345 = {.x = NEW_X, .y = NEW_Y, .z = NEW_Z}

# 4. 重新构建并测试
cmake --build .
./test_adxl345_mock
```

### 问题 5: GitHub Actions 中的 Windows 测试失败

**症状**：Ubuntu/macOS 通过但 Windows 失败

**解决方案**：
```bash
# 使用 MSVC 编译器显式指定
cd tests_unit_mock/build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
ctest
```

## 性能优化

### 加快测试速度

1. **增量编译**（避免全部重新编译）
   ```bash
   # 修改单个文件后
   cmake --build build --parallel
   ```

2. **并行测试**
   ```bash
   # 在支持的系统上并行运行
   ctest --parallel 4
   ```

3. **调试版本跳过优化**
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   # 编译更快，但可能无法捕获某些优化相关 bug
   ```

### 减少 CI/CD 成本

1. **仅在必要时运行**
   ```yaml
   # .github/workflows/mock-tests.yml
   on:
     push:
       branches: [main]
       paths:
         - 'sensors_test/**'  # 仅当此目录变更时运行
   ```

2. **缓存依赖**
   ```yaml
   - uses: actions/cache@v3
     with:
       path: build
       key: ${{ runner.os }}-build-${{ hashFiles('CMakeLists.txt') }}
   ```

3. **分离快速和慢速测试**
   ```bash
   # 快速 Mock 测试 (< 1s)
   ctest -L fast
   
   # 慢速集成测试 (定期运行)
   ctest -L slow
   ```

## 扩展功能

### 添加代码质量检查

```bash
# 安装 clang-format
# macOS:
brew install clang-format

# Ubuntu:
sudo apt-get install clang-format

# 创建 .clang-format 文件
# 在 CMakeLists.txt 中添加:
add_custom_target(format
    COMMAND clang-format -i ${SOURCES}
)
```

### 集成 SonarQube

```yaml
# .github/workflows/sonarcloud.yml
- name: SonarCloud Scan
  uses: SonarSource/sonarcloud-github-action@master
  env:
    GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
    SONARCLOUD_TOKEN: ${{ secrets.SONARCLOUD_TOKEN }}
```

### 自动发布到 Releases

```yaml
# .github/workflows/release.yml
- name: Create Release
  uses: softprops/action-gh-release@v1
  if: startsWith(github.ref, 'refs/tags/')
```

## 最佳实践

### ✅ 推荐做法

1. **每次提交前运行 Mock 测试**
   ```bash
   # 好的做法：在本地验证
   ctest
   git commit
   ```

2. **添加测试用例验证修复**
   ```bash
   # 修复 bug 时同时添加测试
   vim tests_unit_mock/unit/test_*.c
   ```

3. **使用有意义的提交信息**
   ```bash
   # 好的
   git commit -m "Fix ADXL345 Z-axis bounds - verified with mock tests"
   
   # 不好
   git commit -m "fix"
   ```

4. **定期更新依赖**
   ```bash
   # 检查并更新 CMake, 编译器等
   cmake --version
   gcc --version
   ```

### ❌ 避免的做法

1. **绕过 pre-commit 钩子**
   ```bash
   # 避免
   git commit --no-verify

   # 除非特别必要，否则不要使用
   ```

2. **提交未通过测试的代码**
   ```bash
   # 确保本地测试通过后再提交
   ctest
   ```

3. **修改 Mock 值而不更新文档**
   ```bash
   # 更新值时也要更新说明
   # 在 mock_sensor.c 中添加注释
   ```

## 团队协作流程

### 1. 第一次克隆项目
```bash
git clone <repo>
cd sensors_test

# 配置 Git hooks
git config core.hooksPath .githooks

# 构建 Mock 测试
cd tests_unit_mock/build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### 2. 日常开发
```bash
# 推拉最新代码
git pull origin main

# 开发新功能
git checkout -b feature/my-feature

# 修改、测试、提交 (pre-commit 自动验证)
git add .
git commit -m "Feature: ..."

# 推送到远程
git push origin feature/my-feature
```

### 3. Code Review
- PR 创建时 GitHub Actions 自动运行
- 所有测试必须通过才能合并
- 点击 "Details" 查看 CI/CD 日志

### 4. 合并和发布
```bash
# Merge PR
# 所有 CI/CD 检查已通过

# 为发布版本标记
git tag -a v1.0.0
git push origin v1.0.0
```

## 检查清单

### 配置检查清单
- [ ] Git 2.9+
- [ ] CMake 3.20+
- [ ] C11 编译器
- [ ] 执行 `git config core.hooksPath .githooks`
- [ ] Mock 测试构建成功
- [ ] 本地测试全部通过

### 开发检查清单
- [ ] 代码符合编码规范
- [ ] 添加了相应的测试用例
- [ ] 本地 Mock 测试通过
- [ ] Pre-commit 钩子通过
- [ ] 提交信息清晰准确
- [ ] PR 中 GitHub Actions 通过

### 发布检查清表
- [ ] 所有单元测试通过 (Mock)
- [ ] 硬件集成测试通过
- [ ] 代码审查完成
- [ ] 版本号已更新
- [ ] 更新日志已编写
- [ ] 创建 Release 标签

---

**联系方式**：如有问题，请提交 Issue 或 PR  
**最后更新**：2025-01-21

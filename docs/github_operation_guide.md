# GitHub 操作指南与项目上下文恢复

本文档旨在帮助你（或未来的 AI 助手）快速恢复项目上下文，并掌握代码同步到 GitHub 的方法。

## 1. 项目基础信息

*   **本地路径**: `/home/le0n/workspace/le0n`
*   **GitHub 仓库**: `https://github.com/le0n-daily/le0n.git`
*   **Git 用户名**: `leon`
*   **Git 邮箱**: `150014734+le0n-daily@users.noreply.github.com`
*   **当前分支**: `main`

---

## 2. 如何同步代码到 GitHub

### 方式 A：让 AI 助手帮忙（推荐）

如果你在新会话中开启了 **MCP (GitHub)** 功能，只需发送以下指令：

> "帮我把最新的代码同步到 GitHub，仓库是 le0n-daily/le0n。"

AI 会自动执行 `add` -> `commit` -> `push` 的全套流程。

### 方式 B：手动命令行操作

如果你想自己动手，请在终端（Terminal）执行以下命令：

```bash
# 1. 进入项目目录
cd /home/le0n/workspace/le0n

# 2. 查看修改状态 (可选，但推荐)
git status

# 3. 添加所有修改
git add .

# 4. 提交修改 (记得写清楚改了什么)
git commit -m "你的提交信息，例如：完成日志格式化器开发"

# 5. 推送到远程仓库
git push origin main
```

### 方式 C：一行命令极速同步（常用）

如果你赶时间，可以直接使用这一行命令完成所有操作：

```bash
git add . && git commit -m "update" && git push origin main
```

---

## 3. 开发与调试指南

### 万能编译命令 (无需 Make)

无需编写 CMakeLists.txt，直接使用 g++ 编译当前项目：

```bash
g++ -std=c++11 -g -Wall -rdynamic le0n/log.cc le0n/util.cc tests/test.cc -o le0n_test -lpthread
```

### 运行测试

```bash
./le0n_test
```

---

## 4. 给新 AI 的“上下文恢复提示词”

如果你开启了新的聊天窗口，可以将以下内容直接复制给 AI，让他迅速进入状态：

```markdown
我是 le0n，这是我正在开发的 C++ 服务器日志模块项目。
项目路径：/home/le0n/workspace/le0n
GitHub：https://github.com/le0n-daily/le0n

当前进度：
1. **日志模块**：已实现 LogLevel, LogEvent, LogFormatter, Logger, LogAppender 等核心类；支持 stdout/file 输出；支持流式/格式化日志宏；RAII 自动提交。
2. **配置模块**：已实现 ConfigVarBase, ConfigVar, Config 核心类；引入 yaml-cpp 库；实现配置加载与解析（开发中）。
3. **环境**：已配置好 CMake, Git, .gitignore 和 VSCode 调试环境。

请基于以上上下文辅助我进行后续开发。
如果我让你同步代码，请使用 MCP 工具推送到上述 GitHub 仓库。
```

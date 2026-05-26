# C++ Coding Standards

---

## 1. 类型（Types）

### 适用范围

`class` / `struct` / `enum` / `enum class` / `using` / `typedef` / `concept` / 类型模板参数。

### 规则

- 统一使用 **PascalCase** / **UpperCamelCase**
- 每个单词首字母大写
- **不使用下划线**

### 例

```cpp
class PlayerController;
struct UrlTableProperties;
using ByteBuffer = std::vector<std::uint8_t>;
enum class ValueKind;
```

---

## 2. 变量名（Variables）

### 适用范围

局部变量 / 函数形参 / 全局变量 / 静态变量（除常量外）。

### 规则

- 统一使用 **lowerCamelCase**
- `bool` 类型使用小写 b 作为前缀
- 名词或名词短语（表达状态）
- 避免无意义短名：`tmp` / `data` / `value`（除非语境非常明确）

### 例

```cpp
float playerSpeed = 6.0f;
int enemyCount = 0;
float deltaTime = 0.0f;
bool bUse = false;
```

---

## 3. 常量（Constants）

### 适用范围

constexpr / const（包括 class 内静态常量）。

### 规则

- 统一使用 **kPascalCase**
- `k` + `PascalCase`

### 例

```cpp
constexpr int kMaxPlayers = 4;
static constexpr float kDefaultGravity = 9.8f;
```

---

## 4. 函数（Functions）

### 适用范围

自由函数 / 成员函数 / 静态成员函数 / 命名空间内函数。

### 规则

- 统一使用 **lowerCamelCase**
- 函数名应为 **动词短语**
- 函数名 **严禁** 使用 **and** 和 **or** 等连词来表达 2 个以上的动作
- 原则上禁止 **一个函数仅有一行代码** 且没有复用，除非有明确理由
- 布尔返回值使用：`is` / `has` / `can` / `should` 开头

### 例

```cpp
void openFile();
bool isVisible() const;
bool hasTarget() const;
void resetTimer();
```

---

## 5. 命名空间（Namespaces）

### 适用范围

namespace。

### 规则

- 命名空间使用 **全小写**，多词就拆分，禁止使用下划线，即禁止 **snake_case**
- 顶层命名空间应 **全局唯一且可识别**

### 例

```cpp
namespace myproject {
    namespace net {
        class PacketReader;
    }  // namespace net
}  // namespace myproject
```

---

## 6. 枚举（Enums）

### 适用范围

enum / enum class。

### 规则

- 枚举类型名按 Type 规则：PascalCase
- 枚举项（enumerators）：PascalCase
- 禁止 ALL_CAPS 的枚举项（避免与宏冲突）

### 例

```cpp
enum class ValueKind {
  Argument,
  BasicBlock,
  Constant,
};
```

---

## 7. 成员变量（Members）

### 适用范围

class / struct 的数据成员。

### 规则

- 使用 `lowerCamelCase` + `后缀下划线`

### 例

```cpp
class Player {
public:
  void setHp(int hp);

private:
  int hp_ = 100;
  float moveSpeed_ = 6.0f;
};
```

---

## 8. 宏（Macros）

### 适用范围

`#define` 宏（含常量宏、函数宏、条件编译宏等）。

### 规则

- **全大写 + 下划线分词**
- 必须带 **项目专属前缀**，避免与外部宏/系统宏冲突

### 例

```cpp
#define MYPROJECT_ENABLE_LOGGING 1
```

---

## 9. 注释（Comment）

### 适用范围

任何有必要撰写注释的地方。

### 规则

- 超过 2 行的注释，必须使用 **Doxygen** 风格
- **函数** 的注释必须是 **Doxygen** 风格
- 禁止使用冒号、括号
- 禁止注释末尾使用句号、分号
- 禁止分割线注释
- 禁止行尾注释，但对于以下情况做豁免
  - 对 include 的行尾注释例外
  - 对 namespace 的行尾注释例外


### 例

```cpp
// 这是单行注释

/**
* @note 这是多行注释
* 这是多行注释
*/

#include <limits> // std::numeric_limits 这是一条合法注释

namespace game{
    
} // namespace name 这是一条合法注释
```

---

## 10. 格式 & 编码

### 适用范围

所有 `.cpp` 和 `.h` 文件。

### 规则

- 严禁制表符，必须使用 4 个空格来表示缩进
- 换行符严格采用 CRLF，任何不一致都将被视为异常
- 严格 UTF-8 编码格式，任何不一致都将被视为异常

### 例

```cpp
class ConsoleSession final
{
    bool bIsStarted_ = false;
}
```

---

# Files & Folders Standards

## 1. 文件命名

### 适用范围

`.h` / `.hpp` / `.cpp` / `.text` / `.md` / `.json`

### 规则

- 统一使用 **PascalCase**
- 若该文件主要实现一个类：文件名与类名一致
- 不要在文件名前加类型前缀（如 U/A/F/E 等）
- cmake 的 `CMakeLists.txt` 为特例

### 例

```text
Source/
  Gameplay/
    PlayerController.h
    PlayerController.cpp
    EnemySpawner.h
    EnemySpawner.cpp
```

---

## 2. 目录命名

### 适用范围

文件夹

### 规则

- 统一使用 **PascalCase**
- 目录名应明确表达模块职责
- 不包括 `src` / `include` /  `tests` / `third_party` / `tools` 这几个特例，它们已经高度约定俗成，不得更改
- 不包括 `.agents` 及其目录下的子目录、文件，这些属于 codex CLI 相关内容与项目无关
- 不包括 `FusouMapForge` 目录下的子目录

### 例

```text
Gameplay/
Rendering/
UI/
Core/
```

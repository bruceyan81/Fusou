# C++ Coding Standards

---

## 1. 类型（Types）

### 适用范围

`class` / `struct` / `enum` / `enum class` / `using` / `typedef` / `concept` / 类型模板参数。

### 规则

- 统一使用 **PascalCase**
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

局部变量 / 函数形参 / 全局变量 / 静态变量（除 constexpr 常量外）。

### 规则

- 统一使用 **lowerCamelCase**
- 全局变量以小写 g 开头
  - 凡是 namespace 作用域的非 constexpr 变量，一律视为全局变量并使用 g 前缀
  - 是否为 static、const、bool 均不改变这一点
  - 因而全局 bool 使用 g 前缀，**不使用 b 前缀**
- `bool` 类型前缀使用小写 b
- 名词或名词短语（表达状态）但 **布尔类型** 不受这条规则约束
- 避免无意义短名：`tmp` / `data` / `value`（除非语境非常明确）

### 例

```cpp
float playerSpeed = 6.0f;
int enemyCount = 0;
float deltaTime = 0.0f;
bool bIsRunning = false;
char gView = 'p';
```

---

## 3. 常量（Constants）

### 适用范围

constexpr（包括 class 内静态成员），const 不适用于本规则。

### 规则

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

- 除 C++ 规定名称形式的特殊函数外，函数名统一使用 lowerCamelCase
- 函数名应为 **动词短语**
- 严格禁止仅有一行实现且调用点少于 2 处的函数
  - 第三方回调、接口契约、虚函数重写等“必须以函数形式存在”的情况可以豁免
- 从命名层面严格禁止 and 和 or 作为独立单词出现
- 布尔返回值区分以下 3 种情况
  - 查询型函数：如果函数只查询状态，不改变对象状态，原则上应使用 `is` / `has` / `can` / `should` 开头
  - 命令型函数：如果函数会执行动作，且 `bool` 仅表示动作是否成功，可以使用动作动词开头
  - 对失败属于正常业务分支、且调用者必须关注结果的命令型函数，建议使用 `try` 开头

### 例

```cpp
bool tryOpenFile();
bool isVisible() const;
bool hasTarget() const;
void resetTimer();
```

---

## 5. 命名空间（Namespaces）

### 适用范围

namespace。

### 规则

- 命名空间使用 **全小写**，禁止使用下划线，即禁止 **snake_case**
- 顶层命名空间应 **全局唯一且可识别**
- 命名空间层级按模块职责划分，不以拆分单词为目的增加层级

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

class / struct 的数据成员，包含静态成员变量。

### 规则

- 使用 `lowerCamelCase` + `后缀下划线`

### 例

```cpp
class Player {
public:
    void setHp(int hp);

private:
    static constexpr char kView_ = 'P';
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

## 11. include 规则

### 适用范围

所有 `.cpp` 和 `.h` 文件。

### 规则

- 原则上从上到下先 include 项目头，然后标准库头

### 例

```cpp
#include "Character.h"
#include <string>
#include <cstddef>
```

---

# Files & Folders Standards

## 1. 文件命名

### 适用范围

`.h` / `.hpp` / `.cpp` / `.txt` / `.md` / `.json`

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
- 不包括 `src` / `include` / `tests` / `third_party` / `tools` 这几个特例，它们已经高度约定俗成，不得更改
- 不包括 `.agents` 及其目录下的子目录、文件，这些属于 codex CLI 相关内容与项目无关
- 不包括 `FusouMapForge` 目录下的子目录

### 例

```text
Gameplay/
Rendering/
UI/
Core/
```

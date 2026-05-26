# JavaScript 编码规范 

## 1. 基础全局约束 (Global Constraints)
- **缩进 (Indent)**: 严格执行 **2 空格** 缩进。禁止 Tab。
- **语句结束符 (Semicolon)**: **必须显式添加分号**。禁止依赖 ASI。
- **代码行宽 (Line Length)**: 单词行长度限制在 **100 字符** 以内。
- **区块括号 (Braces)**: 所有控制流 (`if`/`else`/`for`/`while`/`try`) **必须使用花括号**，严禁单行省略。

## 2. 变量声明与生命周期 (Variable Management)
- **禁止使用 `var`**：仅允许 `const` 和 `let`。
- **不可变优先原则**：
    - 默认使用 `const`。
    - 仅在变量引用必须被重新赋值时使用 `let`。
- **原子化声明**：一行仅声明一个变量。
    - **Bad**: `const a = 1, b = 2;`
    - **Good**: 
        ```javascript
        const a = 1;
        const b = 2;
        ```

## 3. 命名实体规范 (Naming Conventions)

| 实体类型 | 命名格式 | 示例 |
| :--- | :--- | :--- |
| 变量、函数、属性、方法 | **camelCase** (小驼峰) | `isDataLoading`, `fetchUserRecord()` |
| 类 (Class)、构造函数 | **PascalCase** (大驼峰) | `OrderManager`, `HttpClient` |
| 全局常量、枚举值 | **UPPER_SNAKE_CASE** (全大写蛇形) | `MAX_RETRY_LIMIT`, `STATUS_CODE_SUCCESS` |
| 文件名、目录名 | **kebab-case** (连字符) | `user-auth-service.js`, `api-v1/` |

## 4. 现代语法特性 (Modern Syntax - ES6+)
- **字符串 (Strings)**：
    - 静态文本：使用 **单引号 `'`**。
    - 动态模版：必须使用 **反引号 `` ` ``**。禁止使用 `+` 拼接字符串。
- **对象 (Objects)**：
    - 必须使用 **对象字面量简写** (Property Shorthand)。
    - **解构赋值**：从对象或数组获取超过 2 个属性时，必须使用解构。
- **函数 (Functions)**：
    * 非顶级函数（如回调、内联匿名函数）：必须使用 **箭头函数**。
    * 参数传递：参数超过 3 个时，必须以对象解构形式传递。
- **集合操作**：优先使用 **展开运算符 `...`** 进行对象/数组的浅拷贝和合并。

## 5. 逻辑逻辑判断 (Logic & Comparisons)
- **严格相等**：禁止使用 `==` 和 `!=`。必须使用 `===` 和 `!==`。
- **隐式类型转换 (Truthiness)**：
    - 禁止显式与 `true` / `false` 比较。
    - **Yes**: `if (isValid) {}`
    - **No**: `if (isValid === true) {}`

## 6. 文档化与类型提示 (Documentation)
- **JSDoc 强制性**：所有**导出的函数**和**类方法**必须包含 JSDoc 注释块。
- **核心标签**：必须包含 `@param` (带类型) 和 `@returns`。
```javascript
/**
 * @param {string} endpoint - API 请求路径
 * @param {Object} options - 请求配置
 * @returns {Promise<Response>} 
 */
async function request(endpoint, options) { ... }
```

## 7. 错误处理 (Error Handling)
 **异步处理**：所有 `await` 必须包裹在 `try...catch` 块中。
 **错误对象**：`throw` 必须抛出 `Error` 实例，严禁抛出字符串或字面量。
    - **Yes**: `throw new Error('Failed to save');`
    - **No**: `throw 'Failed to save';`

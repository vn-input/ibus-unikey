---
name: development-guidelines
description: Provide a set of industry-standard coding and project organisation best practices used throughout software engineering tasks. Includes naming conventions, boolean clarity, modularization, DRY, and forbidden antipatterns.
---

This skill contains a concise but thorough set of development guidelines for contributors to the project. Use it whenever you are asked to review code, refactor logic, establish structure, or explain why a particular pattern should be followed or avoided.

### Key principles

1. **Modular design & functional decomposition**
   * Break large features into small, focused functions or classes. Prefer a single responsibility per module.
   * Avoid monolithic functions; if a routine grows beyond ~50 lines or does two things, split it.
   * Organise source files in subdirectories by component or feature. For example, `src/engine/`, `src/setup/`, `tests/`.
   * Apply the DRY (Don't Repeat Yourself) principle: factor repeated logic into helpers or utilities.

2. **Readable boolean expressions**
   * Precompute complex conditions into well‑named boolean variables before using them in `if`, `while`, etc.  
     ```c
     bool is_at_begin_line = (a <= 0);
     bool is_beyond_third_column = (b >= 3);
     bool can_accumulate = is_at_begin_line && is_beyond_third_column;
     if (can_accumulate) {
         …
     }
     ```
   * Prefix boolean variables with `is_`, `has_`, `should_` to make their meaning explicit.
   * Avoid writing `if (a <= 0 && b >= 3)` directly; break the expression apart as above.
   * Use formal logic and De Morgan’s laws when it makes the intent clearer: for example,
     ```c
     // clearer than if (!(a || b))
     bool none_passed = !a && !b;
     ```
   * Minimise the use of negations (`!`, `not`) and the `||` operator in deep chains; name the intermediate states instead.

3. **Naming conventions**
   * Choose descriptive, consistent names for functions, variables, and constants.
   * Boolean variables start with `is_`, `has_`, `can_`, `should_`.
   * Constants use `ALL_CAPS` or `kCamelCase` depending on project style.
   * Place enums and common constants in a central header or source file (`constants.h`, `enums.h`, etc.) so they can be looked up easily and documented in one place. This makes it simpler for translators, reviewers, and automated tools to reference the values.
   * Avoid single‑letter names except for well‑known iterators (`i`, `j`).

4. **Avoid forbidden practices**
   * No magic numbers: use named constants instead.
   * No deep nesting or long chains of `if/else` that obscure logic; refactor into early returns or helpers.
   * No excessive use of global state; pass context explicitly when possible.
   * Do not repeat code; don't copy/paste logic across modules.
   * Avoid premature optimisation; make it readable first, optimise later with profiling.
   * Do not ignore error returns; handle failures or propagate meaningful errors.

5. **Documentation & comments**
   * Prefer self‑documenting code. Add comments to explain *why* something is done, not *what* the code does.
   * Keep comments up to date; stale comments are worse than none.
   * Use README or `docs/` for higher‑level architectural documentation.

6. **Project structure & modularisation**
   * Group related source files into sub‑directories by functionality (e.g., `engine/`, `setup/`, `ukengine/`).
   * Separate tests into a parallel tree (e.g., `tests/engine/`, `tests/setup/`).
   * Keep build scripts, configuration, and documentation in top‑level directories (`CMakeLists.txt`, `.github/`, `docs/`).
   * Encourage small, focused modules that can be understood and compiled independently.

7. **Formal methods & logical clarity**
   * When expressing combined conditions, consider rewriting using logical equivalences to make intent explicit:
     - `not a && not b` is clearer than `!(a || b)` when checking that *neither* condition is true.
     - `a || b` may be replaced with a descriptive variable: `bool has_access = user_is_admin || user_has_permission;`.
   * Think in terms of the problem domain and name the composed booleans accordingly.

### Usage

Consult this document during code review, refactoring, or when establishing new components. Its presence in the `.github/skills` directory means the AI assistant will reference these guidelines automatically when appropriate.

For planning new work, see the companion
[`project_planning.md`](project_planning.md) guide in this folder; it explains
how to create and name planning documents by category.
---

### Hướng dẫn (Tiếng Việt)

1. **Thiết kế mô‑đun & phân rã chức năng**
   * Chia các tính năng lớn thành những hàm hoặc lớp nhỏ, tập trung. Mỗi mô‑đun nên chỉ có một trách nhiệm.
   * Tránh các hàm đơn khối; nếu một hàm vượt quá ~50 dòng hoặc thực hiện hai việc thì hãy tách ra.
   * Tổ chức tệp nguồn theo thư mục con theo thành phần hoặc tính năng, ví dụ `src/engine/`, `src/setup/`, `tests/`.
   * Áp dụng nguyên tắc DRY (Don't Repeat Yourself): đưa logic lặp vào hàm phụ hoặc thư viện.

2. **Biểu thức Boolean dễ đọc**
   * Tính trước các điều kiện phức tạp vào các biến boolean có tên rõ ràng trước khi dùng trong `if`, `while`, v.v.:  
     ```c
     bool is_at_begin_line = (a <= 0);
     bool is_beyond_third_column = (b >= 3);
     bool can_accumulate = is_at_begin_line && is_beyond_third_column;
     if (can_accumulate) { … }
     ```
   * Tiền tố `is_`, `has_`, `should_` cho các biến boolean.
   * Tránh viết `if (a <= 0 && b >= 3)` trực tiếp; hãy tách ra như trên.
   * Dùng logic hình thức và định luật De Morgan khi làm rõ ý định: ví dụ `!a && !b` rõ hơn `!(a || b)`.
   * Giảm thiểu việc dùng phủ định (`!`, `not`) và toán tử `||` trong chuỗi dài; đặt tên các trạng thái trung gian.

3. **Quy ước đặt tên**
   * Chọn tên mô tả, nhất quán cho hàm, biến, và hằng.
   * Biến boolean bắt đầu bằng `is_`, `has_`, `can_`, `should_`.
   * Hằng số dùng `ALL_CAPS` hoặc `kCamelCase` tùy phong cách dự án.
   * Đặt các enum và hằng chung trong một tệp tiêu đề hoặc nguồn tập trung (ví dụ `constants.h`, `enums.h`). Việc này giúp mọi người dễ tra cứu và dễ tài liệu hoá một chỗ.
   * Tránh tên một chữ cái trừ vòng lặp (`i`, `j`).

4. **Tránh thực hành bị cấm**
   * Không dùng số thần kỳ (magic numbers); hãy dùng hằng có tên.
   * Tránh lồng ghép sâu hoặc chuỗi `if/else` dài làm mờ logic; tái cấu trúc với trả sớm hoặc hàm phụ.
   * Hạn chế trạng thái toàn cục; truyền ngữ cảnh rõ ràng thay vì dùng biến toàn cục.
   * Không sao chép/​dán mã; tránh lặp lại.
   * Không tối ưu hoá quá sớm; làm cho nó dễ đọc trước, tối ưu khi cần.
   * Không bỏ qua các giá trị trả về lỗi; xử lý hoặc truyền lại lỗi có ý nghĩa.

5. **Tài liệu & chú thích**
   * Ưu tiên mã tự giải thích. Thêm chú thích để giải thích *tại sao* làm điều đó, không phải *cái gì*.
   * Giữ chú thích cập nhật; chú thích lỗi thời còn tệ hơn không có.
   * Dùng README hoặc `docs/` cho tài liệu kiến trúc tổng quát.

6. **Cấu trúc dự án & mô‑đun hóa**
   * Nhóm tệp nguồn liên quan vào thư mục con theo chức năng (ví dụ `engine/`, `setup/`, `ukengine/`).
   * Tách các bài kiểm tra vào cây song song (ví dụ `tests/engine/`, `tests/setup/`).
   * Giữ script build, cấu hình và tài liệu ở thư mục cấp trên (`CMakeLists.txt`, `.github/`, `docs/`).
   * Khuyến khích các mô‑đun nhỏ, tập trung, dễ hiểu và biên dịch độc lập.
   * Thêm chú thích tiêu đề chuẩn cho mỗi tệp nguồn/tiêu đề tóm tắt mục đích, trách nhiệm của nội dung, thông báo bản quyền/giấy phép và thông tin liên hệ người bảo trì. Điều này giúp rõ ràng mức sở hữu và hỗ trợ kiểm tra giấy phép.

7. **Phương pháp hình thức & rõ ràng logic**
   * Khi biểu diễn các điều kiện kết hợp, xem xét viết lại dùng các đẳng thức logic để làm rõ ý định:
     - `not a && not b` rõ hơn `!(a || b)` khi kiểm tra *không điều kiện nào* đúng.
     - `a || b` có thể thay bằng biến mô tả: `bool has_access = user_is_admin || user_has_permission;`.
   * Nghĩ theo miền vấn đề và đặt tên boolean kết hợp tương ứng.

Văn bản tiếng Việt ở trên là bản dịch và tóm tắt của các hướng dẫn chính; sử dụng khi cần giải thích bằng tiếng Việt.
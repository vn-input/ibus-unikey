### Hiện tại bộ gõ này không còn được hỗ trợ chính thức.

<!--
// Copyright (C) 2026 Hoang Duy Tran & AI Agent
// Contact:
//   hoangduytran1960@googlemail.com
-->

Nếu bạn gặp vấn đề khi sử dụng, bạn có thể cân nhắc:
- [ibus-bamboo](https://github.com/BambooEngine/ibus-bamboo)
- [ibus-bogo](https://github.com/BoGoEngine/ibus-bogo)
- Các bản fork của ibus-unikey: https://github.com/vn-input/ibus-unikey/network/members

IBus-Unikey IME
===============

[![Build Status](https://travis-ci.org/vn-input/ibus-unikey.svg?branch=master)](https://travis-ci.org/vn-input/ibus-unikey)

`ibus-unikey` là bộ gõ tiếng Việt cho [IBus](https://github.com/ibus/ibus), sử dụng lõi `ukengine` (biến thể từ UniKey/x-unikey).

---

## Recent changes (macro feature set)

Các thay đổi macro/UI hiện có trong nhánh làm việc này:

1. **Macro storage không còn giới hạn 1024 entries**
	- `CMacroTable` chuyển từ mảng tĩnh sang cấp phát động.
	- Tự động tăng dung lượng bằng chiến lược nhân đôi.

2. **Dictionary semantics khi load macro file**
	- Sau khi sort theo key, các key trùng được gộp.
	- Entry xuất hiện **sau** trong file sẽ ghi đè entry trước.

3. **Macro dialog CRUD/UI nâng cấp**
	- Thêm `Add` / `Remove`, và dùng lại cùng hộp thoại cho việc chỉnh sửa.
	- Dialog thêm/chỉnh sửa mở qua double-click hoặc Enter trên dòng macro; không còn chỉnh sửa inline.
	- Replace field là `GtkTextView` nhiều dòng, scrollable, phù hợp với các macro lớn.
	- Label `With:` trong dialog add/edit được canh lên trên để khớp với vùng nhập nhiều dòng.
	- Sort theo cột bằng click header.
	- Sau khi thêm hoặc lưu thay đổi, danh sách được sort, tự cuộn tới item mới và chọn lại dòng vừa lưu.
	- Khi đổi sort, giao diện tự cuộn về item đang chọn (hoặc dòng đầu nếu chưa chọn).

4. **Search UX cho macro list**
	- Có `GtkSearchEntry` ngay trên bảng macro cùng các nút `First` / `Prev` / `Next` / `Last`.
	- Các nút điều hướng search khởi đầu ở trạng thái disabled-by-style: vẫn có hover muted effect nhưng không xử lý click cho tới khi search có kết quả.
	- Nhấn Enter để commit search text và cập nhật match navigation.
	- Khi bấm nút `x` để clear ô search, danh sách đầy đủ được khôi phục ngay lập tức.
	- Sau khi clear search, giao diện tự cuộn về dòng đã được chọn trước khi bắt đầu search; nếu không có dòng nào, nó quay về dòng đầu tiên.

5. **File import/export hỗ trợ**
	- Người dùng có thể nhập khẩu macro từ các định dạng bên ngoài (Espanso YAML, macOS text substitutions plist) và xuất khẩu sang các định dạng này.
	- Chức năng có sẵn trong hộp thoại macro; lựa chọn định dạng dựa trên phần mở rộng tên tệp (`.yml`/`.yaml`/`.plist`).
	- Dialog import/export ghi nhớ thư mục dùng gần nhất qua `GSettings`, nên lần mở kế tiếp sẽ quay lại đúng vị trí trước đó.
	- Tích hợp với `macro_file_io` để tự động chuyển đổi và xử lý trùng lặp.

6. **Tối ưu hiệu năng macro list lớn**
	- Detach model khi nạp hàng loạt.
	- Tránh cập nhật UI theo từng dòng.

7. **Tổ chức lại setup UI theo hướng production-style**
	- Tách UI thành các file độc lập trong `setup/gui/ui/`:
	  - `main_window.ui`
	  - `macro_dialog.ui`
	  - `add_macro_dialog.ui`
	- GTK builder bootstrap tập trung ở `src/setup/gui/builder/gui_builder.cpp`.
	- Handlers được tách theo owner dialog/window trong `src/setup/gui/handlers/`.
	- Cấu hình engine options, macro editor, và dialog add/edit vẫn dùng cùng builder nên signal binding của GTK giữ nguyên.

### Runtime flow (high level)

1. `src/main.cpp` khởi tạo IBus component và engine.
2. `src/engine.cpp` nhận key event từ IBus.
3. Engine chuyển key vào `ukengine` (`UnikeyFilter` / `UkEngine::process`).
4. Kết quả preedit/commit trả lại cho IBus app đang focus.
5. Macro table được nạp từ file người dùng khi engine load config.

---

## Macro subsystem guide

### Macro file format

- File mặc định: `~/.ibus/unikey/macro`
- Header UTF-8: `DO NOT DELETE THIS LINE*** version=1 ***`
- Mỗi dòng macro: `key:replacement`

**Import/export formats**

Ứng dụng cấu hình giờ hỗ trợ đọc và ghi một số định dạng bên ngoài:

- **YAML**: cấu trúc giống [Espanso](https://espanso.org) (`.yml`/`.yaml`).
- **Apple plist**: bảng thay thế văn bản của macOS (`.plist`).

Chọn định dạng dựa trên phần mở rộng tệp khi lưu hoặc nhập. Các macro được
chuyển đổi sang bảng nội bộ với cùng quy tắc dedupe như khi load file gốc.

**Remembered import/export locations**

- Thư mục làm việc gần nhất của import/export được lưu trong `GSettings` key
	`macro-last-working-dir`.
- Khi mở file chooser, ibus-unikey đặt thư mục hiện hành theo giá trị đã lưu,
	hoặc fallback về thư mục mặc định nếu chưa có giá trị hợp lệ.
- Không có file `config.yaml` riêng cho tính năng này.

### Core files

- `ukengine/mactab.h`, `ukengine/mactab.cpp`
	- Quản lý bảng macro trong bộ nhớ.
	- Parse / load / write file.
	- Lookup theo key trong lúc gõ.

- `src/setup/macro_utils.cpp`
	- Chuyển đổi qua lại giữa `CMacroTable` và `GtkListStore`.

- `src/setup/macro_file_io.cpp`
	- Nhập/xuất macro theo định dạng native, YAML, và plist.

- `src/setup/macro_dialog_state.cpp`
	- Lưu search text, selected key, import/export folder, dialog geometry, và các trạng thái UX liên quan.

- `src/setup/signal_handlers_macros.cpp`
	- Toàn bộ hành vi macro dialog hiện tại: CRUD, sort, search, match navigation, import/export, và reset selection sau khi clear search.

- `src/setup/gui/builder/gui_builder.cpp`
	- Nạp toàn bộ setup UI từ nhiều file `.ui` vào cùng một `GtkBuilder`.

- `src/setup/gui/handlers/main_window_handlers.cpp`
- `src/setup/gui/handlers/add_macro_dialog_handlers.cpp`
	- Tách handlers theo owner dialog/window để cấu trúc setup dễ bảo trì hơn.

### Important behavior notes

- `CMacroTable::removeDuplicates()` chạy sau sort và giữ giá trị mới nhất cho key trùng.
- `lookup` dùng `bsearch` trên bảng đã sort.
- UI luôn giữ một row sentinel `(...)` cho thao tác thêm/sửa tiện lợi.

---

## Build and run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"
```

Chạy setup utility từ build:

```bash
./build/setup/ibus-setup-unikey
```

Install system-wide:

```bash
sudo cmake --install build
```

Nếu hệ thống yêu cầu `libexec` path riêng:

```bash
cmake -S . -B build -DLIBEXECDIR=/usr/libexec
cmake --build build -j"$(nproc)"
sudo cmake --install build
```

---

## Test helpers in this repo

- `tools/stress_macro_test.sh`
	- Sinh file macro lớn và kiểm tra load/write/reload consistency.

- `tools/macro_probe.cpp`
	- Utility nhỏ để kiểm tra round-trip macro file bằng `CMacroTable`.

Ví dụ:

```bash
bash tools/stress_macro_test.sh build 50000
```

---

## Contributing and creating a PR

### 1) Fork và clone

```bash
git clone https://github.com/<your-user>/ibus-unikey.git
cd ibus-unikey
git remote add upstream https://github.com/vn-input/ibus-unikey.git
```

### 2) Tạo branch tính năng

```bash
git checkout -b feature/macro-unlimited-crud-sort
```

### 3) Build + verify trước khi commit

```bash
cmake -S . -B build
cmake --build build -j"$(nproc)"
bash tools/stress_macro_test.sh build 50000
```

### 4) Chỉ commit source cần thiết

Không commit output build (`build/`, object files, binaries).

```bash
git status
git add README.md setup/ ukengine/ tools/
git commit -m "Improve macro system: dynamic storage, CRUD/sort UI, docs"
```

### 5) Push và mở Pull Request

```bash
git push origin feature/macro-unlimited-crud-sort
```

Sau đó mở PR trên GitHub, target vào repo bạn muốn đóng góp (upstream hoặc fork đang active).

### PR checklist đề xuất

- [ ] Giải thích rõ hành vi mới (unlimited macro, dedupe semantics, CRUD/sort UI)
- [ ] Mô tả cách test với file macro lớn
- [ ] Đính kèm screenshot Macro dialog nếu thay UI
- [ ] Nêu rõ các file chính đã thay đổi

---

## Notes about project status

Do dự án gốc có thể không còn maintain tích cực, PR có thể phù hợp hơn với các fork đang hoạt động trong network của `vn-input/ibus-unikey`.

### Legacy wiki

Tài liệu cũ: [wiki](https://github.com/vn-input/ibus-unikey/wiki)


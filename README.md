### Hiện tại bộ gõ này không còn được hỗ trợ chính thức.

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

Các thay đổi chính đã được thực hiện trong nhánh làm việc này:

1. **Macro storage không còn giới hạn 1024 entries**
	 - `CMacroTable` chuyển từ mảng tĩnh sang cấp phát động.
	 - Tự động tăng dung lượng bằng chiến lược nhân đôi.

2. **Dictionary semantics khi load macro file**
	 - Sau khi sort theo key, các key trùng được gộp.
	 - Entry xuất hiện **sau** trong file sẽ ghi đè entry trước.

3. **Macro dialog CRUD/UI nâng cấp**
	 - Thêm `Add` / `Remove`.
	 - Dialog thêm macro riêng (Replace / With).
	 - Sort theo cột bằng click header.
	 - Sau khi thêm, danh sách được sort, tự cuộn tới item mới và highlight.
	 - Khi đổi sort, giao diện tự cuộn về item đang chọn (hoặc dòng đầu nếu chưa chọn).

4. **Tối ưu hiệu năng macro list lớn**
	 - Detach model khi nạp hàng loạt.
	 - Tránh cập nhật UI theo từng dòng.

5. **Sửa signal binding cho GTK builder**
	 - Build setup utility có `-rdynamic` để GTK tìm được handlers qua `gtk_builder_connect_signals`.

---

## Source map (for new developers)

### Top-level modules

- `src/`:
	- IBus engine integration (`engine.cpp`, `engine.h`, `engine_private.h`)
	- Xử lý event bàn phím từ IBus, nạp cấu hình, gọi API của `ukengine`.
- `ukengine/`:
	- Lõi xử lý tiếng Việt (tone, roof, hook, macro expansion, charset conversion).
	- Đây là nơi thay đổi thuật toán và data structure cốt lõi.
- `setup/`:
	- Ứng dụng cấu hình GUI (`ibus-setup-unikey`) bằng GTK3 + GtkBuilder.
	- Macro editor UI, import/export, cấu hình engine options.
- `src/config/`:
	- GSettings schema + wrapper đọc/ghi config.
- `data/`, `po/`:
	- Tài nguyên giao diện và bản dịch.

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

### Core files

- `ukengine/mactab.h`, `ukengine/mactab.cpp`
	- Quản lý bảng macro trong bộ nhớ.
	- Parse / load / write file.
	- Lookup theo key trong lúc gõ.

- `setup/macro_utils.h`, `setup/macro_utils.cpp`
	- Chuyển đổi qua lại giữa `CMacroTable` và `GtkListStore`.

- `setup/signal_handlers.cpp`
	- Toàn bộ hành vi UI của Macro dialog (CRUD/sort/import/export).

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

